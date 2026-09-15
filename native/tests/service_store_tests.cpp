#include "verdigris/service_store.hpp"
#define NOMINMAX
#include <windows.h>
#include <winsqlite/winsqlite3.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>
#include <vector>

using verdigris::service::Admission;
using verdigris::service::Store;
namespace fs = std::filesystem;
namespace {
int assertions = 0;
void check(bool condition, const char* message) {
    ++assertions;
    if (!condition) throw std::runtime_error(message);
}
Admission enroll(Store& store, std::int64_t now) {
    std::string error;
    const auto code = store.issue_enrollment(now, 1000, &error);
    check(!code.empty(), "issue enrollment");
    const auto admission = store.enroll(code, now, &error);
    check(admission.has_value(), "enroll account");
    return *admission;
}
std::string contents(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}
void assert_secrets_absent(const fs::path& directory, const std::vector<std::string>& secrets) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.path().extension() == ".lock") continue;
        const auto bytes = contents(entry.path());
        for (const auto& secret : secrets) check(bytes.find(secret) == std::string::npos, "credential plaintext persisted");
    }
}
void crash_child(const fs::path& directory, const std::string& a, const std::string& b, int writes, bool global_state = false) {
    wchar_t executable[32768]{}; GetModuleFileNameW(nullptr, executable, 32768);
    std::wstring command = L"\"" + std::wstring(executable) + (global_state ? L"\" --crash-world \"" : L"\" --crash \"") + directory.wstring() + L"\" " +
        std::wstring(a.begin(), a.end()) + L" " + std::wstring(b.begin(), b.end()) + L" " + std::to_wstring(writes);
    STARTUPINFOW startup{}; startup.cb = sizeof(startup); PROCESS_INFORMATION process{};
    check(CreateProcessW(executable, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process), "start crash child");
    const auto wait = WaitForSingleObject(process.hProcess, 15000);
    DWORD code{}; GetExitCodeProcess(process.hProcess, &code);
    if (wait != WAIT_OBJECT_0) TerminateProcess(process.hProcess, 88);
    CloseHandle(process.hProcess); CloseHandle(process.hThread);
    check(wait == WAIT_OBJECT_0 && code == 77, "crash child reached precommit process termination");
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 6 && (std::string(argv[1]) == "--crash" || std::string(argv[1]) == "--crash-world")) {
            Store store(argv[2]); if (!store.open()) return 78;
            store.test_interrupt_after_writes(std::stoi(argv[5]), true);
            if (std::string(argv[1]) == "--crash-world")
                store.commit_state({{argv[3], "crash-a"}, {argv[4], "crash-b"}}, "world:uncommitted", "outcome:crashed-world");
            else store.commit_accounts({{argv[3], "crash-a"}, {argv[4], "crash-b"}}, "outcome:crashed");
            return 79;
        }
        const auto root = fs::absolute(fs::path("native/build") / ("service-store-test-" + std::to_string(GetCurrentProcessId())));
        check(!fs::exists(root), "fresh QA directory"); fs::create_directories(root);
        const auto live = root / "live";
        std::string error;
        Admission a, b, revoked;
        std::string code;
        {
            Store unopened((root / "unopened").string());
            check(!unopened.authenticate("invalid", 0, &error) && !error.empty(), "unopened fails closed");
            check(!unopened.load_world_state(&error) && !error.empty(), "global state read failure is not silent absence");
            Store store(live.string(), {10000});
            check(store.open(&error), "open local store"); check(error.empty(), "open clears error");
            check(store.open(&error), "repeat open is idempotent");
            Store second(live.string()); check(!second.open(&error), "exclusive directory lock");
            check(!store.enroll("guest-alice", 100, &error), "arbitrary guest identity rejected");
            check(!store.authenticate("ses_" + std::string(64, '0'), 100, &error), "random token rejected");
            check(store.issue_enrollment(-1, 10, &error).empty(), "negative issue clock rejected");
            check(store.issue_enrollment(0, 0, &error).empty(), "zero TTL rejected");
            check(store.issue_enrollment(std::numeric_limits<std::int64_t>::max(), 1, &error).empty(), "expiry overflow rejected");
            const auto expired = store.issue_enrollment(100, 10);
            check(!store.enroll(expired, 110, &error), "enrollment expires at boundary");
            code = store.issue_enrollment(100, 1000);
            store.test_interrupt_after_writes(1, false);
            check(!store.enroll(code, 100, &error), "failed enrollment rolls back");
            const auto accepted = store.enroll(code, 100, &error);
            check(accepted.has_value(), "failed enrollment did not consume code"); a = *accepted;
            check(!store.enroll(code, 100, &error), "enrollment replay rejected");
            check(a.account_id.rfind("acct_", 0) == 0 && a.token.rfind("ses_", 0) == 0, "namespaced identities");
            check(a.expires_at_ms == 10100, "configured fixed session expiry");
            check(!store.authenticate(a.token, -1), "negative authentication clock rejected");
            check(store.authenticate(a.token, 10099).has_value(), "credential before expiry");
            check(!store.authenticate(a.token, 10100), "credential expires at boundary");
            check(!store.authenticate(code, 100), "enrollment code cannot authenticate");
            b = enroll(store, 100); revoked = enroll(store, 100);
            check(a.account_id != b.account_id && a.token != b.token, "independent accounts and credentials");
            check(store.revoke(revoked.token), "revoke credential");
            check(!store.authenticate(revoked.token, 100), "revoked credential rejected");
            check(store.revoke(revoked.token), "revoke idempotent");
            check(!store.load_account("../../other-account", &error), "path-like identity rejected");
            check(store.commit_accounts({{a.account_id, "{\"house\":\"A\",\"item\":\"item_1\"}"}, {b.account_id, "{\"house\":\"B\"}"}}, "outcome:first", &error), "atomic two-account outcome");
            check(store.has_outcome("outcome:first"), "outcome recorded");
            const auto a_snapshot = store.load_account(a.account_id), b_snapshot = store.load_account(b.account_id);
            check(store.commit_accounts({{a.account_id, "duplicate must not replace"}}, "outcome:first"), "duplicate accepted as no-op");
            check(store.load_account(a.account_id) == a_snapshot && store.load_account(b.account_id) == b_snapshot, "duplicate preserves both snapshots");
            store.test_interrupt_after_writes(1, false);
            check(!store.commit_accounts({{a.account_id, "bad-a"}, {b.account_id, "bad-b"}}, "outcome:failed", &error), "failure injected between writes");
            check(!store.has_outcome("outcome:failed"), "failed outcome absent");
            check(store.load_account(a.account_id) == a_snapshot && store.load_account(b.account_id) == b_snapshot, "partial write rolled back");
            store.test_interrupt_after_writes(3, false);
            check(!store.commit_accounts({{a.account_id, "bad-a"}, {b.account_id, "bad-b"}}, "outcome:failed-late", &error), "failure after outcome insert before commit");
            check(!store.has_outcome("outcome:failed-late"), "late failed outcome rolled back");
            check(!store.commit_accounts({{a.account_id, "bad-a"}, {"acct_" + std::string(64, '0'), "unknown"}}, "outcome:unknown", &error), "unknown account transaction rejected");
            check(store.load_account(a.account_id) == a_snapshot, "unknown account transaction preserves earlier writes");
            check(!store.commit_accounts({}, "outcome:empty", &error), "empty new outcome rejected");
            check(!store.commit_accounts({{a.account_id, std::string(16 * 1024 * 1024 + 1, 'x')}}, "", &error), "oversized snapshot rejected");

            const auto race_code = store.issue_enrollment(100, 1000);
            std::atomic<int> winners{0}; std::vector<std::thread> threads;
            for (int i = 0; i < 16; ++i) threads.emplace_back([&] { if (store.enroll(race_code, 100)) ++winners; });
            for (auto& thread : threads) thread.join();
            check(winners == 1, "concurrent enrollment consumes code exactly once");
            threads.clear(); winners = 0;
            for (int i = 0; i < 16; ++i) threads.emplace_back([&, i] {
                if (store.commit_accounts({{a.account_id, "winner-" + std::to_string(i)}, {b.account_id, "winner-" + std::to_string(i)}}, "outcome:race")) ++winners;
            });
            for (auto& thread : threads) thread.join();
            check(winners == 16 && store.load_account(a.account_id) == store.load_account(b.account_id), "concurrent outcome has one atomic winner");
            check(store.commit_accounts({{a.account_id, "stable-a"}, {b.account_id, "stable-b"}}, ""), "ordinary checkpoint without outcome");
            check(!store.has_outcome(""), "empty outcome never stored");
            check(store.backup_to((root / "backup").string(), &error), "online consistent backup");
            check(!store.backup_to((root / "backup").string(), &error), "backup does not overwrite existing destination");
            check(store.commit_accounts({{a.account_id, "after-backup"}}, "outcome:after-backup"), "live writes continue after backup");
            assert_secrets_absent(live, {code, expired, race_code, a.token, b.token, revoked.token});
        }
        {
            Store restarted(live.string()); check(restarted.open(&error), "restart unlocks database");
            check(restarted.authenticate(a.token, 500)->account_id == a.account_id, "credential survives restart");
            check(!restarted.authenticate(revoked.token, 500), "revocation survives restart");
            check(!restarted.enroll(code, 500), "consumed enrollment remains consumed after restart");
            check(restarted.load_account(a.account_id) == "after-backup" && restarted.load_account(b.account_id) == "stable-b", "snapshots survive restart");
            check(restarted.has_outcome("outcome:first"), "outcome survives restart");
        }
        for (int writes : {1, 3}) {
            crash_child(live, a.account_id, b.account_id, writes);
            Store recovered(live.string()); check(recovered.open(&error), "crash lock release and WAL recovery");
            check(recovered.load_account(a.account_id) == "after-backup" && recovered.load_account(b.account_id) == "stable-b", "process crash leaves no partial multi-account state");
            check(!recovered.has_outcome("outcome:crashed"), "process crash rolls back outcome");
        }
        const auto global_directory = root / "global";
        Admission global_a, global_b;
        const std::string queued = "{\"relic_1\":\"queued\"}", claimed = "{\"relic_1\":\"claimed\"}";
        {
            Store store(global_directory.string()); check(store.open(&error), "global state store open");
            check(!store.load_world_state(&error) && error.empty(), "absent global state is distinct from failure");
            global_a = enroll(store, 100); global_b = enroll(store, 100);
            check(store.commit_state({}, queued, "outcome:global-only", &error), "world-only checkpoint allowed");
            check(store.load_world_state() == queued, "world-only snapshot readable");
            check(store.commit_accounts({{global_a.account_id, "source-with-relic"}, {global_b.account_id, "recipient-empty"}}, "outcome:global-baseline"), "seed accounts without changing global state");
            check(store.load_world_state() == queued, "ordinary account commit preserves global state");
            for (int writes : {1, 3, 4}) {
                store.test_interrupt_after_writes(writes, false);
                check(!store.commit_state({{global_a.account_id, "source-transferred"}, {global_b.account_id, "recipient-with-relic"}}, claimed, "outcome:global-failed", &error), "injected account/world/outcome transaction fails");
                check(store.load_account(global_a.account_id) == "source-with-relic" && store.load_account(global_b.account_id) == "recipient-empty" && store.load_world_state() == queued,
                      "account/world/outcome rollback preserves source, recipient and global ledger together");
                check(!store.has_outcome("outcome:global-failed"), "failed global outcome absent");
            }
            check(!store.commit_state({{global_a.account_id, "invalid"}}, std::string(16 * 1024 * 1024 + 1, 'x'), "outcome:global-oversized", &error), "oversized global state rejected");
            check(store.load_account(global_a.account_id) == "source-with-relic" && store.load_world_state() == queued, "oversized global rejection has no side effect");
            check(store.commit_state({{global_a.account_id, "source-transferred"}, {global_b.account_id, "recipient-with-relic"}}, claimed, "outcome:relic-transfer", &error), "source recipient and claimed ledger commit atomically");
            check(store.commit_state({{global_b.account_id, "duplicate-recipient"}}, queued, "outcome:relic-transfer", &error), "duplicate combined outcome returns success");
            check(store.load_account(global_a.account_id) == "source-transferred" && store.load_account(global_b.account_id) == "recipient-with-relic" && store.load_world_state() == claimed,
                  "duplicate combined outcome changes neither accounts nor ledger");
            check(store.backup_to((root / "global-backup").string(), &error), "combined account/world state backup");
            check(store.commit_state({}, "world:after-backup", "outcome:global-after-backup"), "global ledger advances after backup");
            check(store.commit_state({}, claimed, ""), "restore crash fixture ledger without rewriting accounts");
        }
        for (int writes : {1, 3, 4}) {
            crash_child(global_directory, global_a.account_id, global_b.account_id, writes, true);
            Store recovered(global_directory.string()); check(recovered.open(&error), "global state WAL crash recovery open");
            check(recovered.load_account(global_a.account_id) == "source-transferred" && recovered.load_account(global_b.account_id) == "recipient-with-relic" && recovered.load_world_state() == claimed,
                  "real process crash preserves last committed source recipient and ledger");
            check(!recovered.has_outcome("outcome:crashed-world") && recovered.has_outcome("outcome:relic-transfer"), "crash preserves committed deduplication and rejects partial outcome");
        }
        check(Store::restore_backup((root / "global-backup").string(), (root / "global-restored").string(), &error), "restore combined account/world backup");
        {
            Store restored((root / "global-restored").string()); check(restored.open(&error), "combined restore opens");
            check(restored.load_account(global_a.account_id) == "source-transferred" && restored.load_account(global_b.account_id) == "recipient-with-relic" && restored.load_world_state() == claimed,
                  "backup restores matching account inventories and global ownership ledger");
            check(restored.has_outcome("outcome:relic-transfer") && !restored.has_outcome("outcome:global-after-backup"), "combined backup restores its exact outcome point");
        }
        check(Store::restore_backup((root / "backup").string(), (root / "restored").string(), &error), "restore backup into fresh store");
        check(!Store::restore_backup((root / "backup").string(), live.string(), &error), "restore never overwrites live directory");
        check(!Store::restore_backup((root / "missing").string(), (root / "missing-restore").string(), &error), "missing backup rejected");
        check(!Store::restore_backup(live.string(), (root / "unmarked-restore").string(), &error), "incomplete or unmarked backup rejected");
        {
            Store restored((root / "restored").string()); check(restored.open(&error), "restored store admits reads");
            check(restored.authenticate(a.token, 500)->account_id == a.account_id, "restored credentials authenticate");
            check(restored.load_account(a.account_id) == "stable-a" && restored.load_account(b.account_id) == "stable-b", "restore retains consistent backup point");
            check(!restored.has_outcome("outcome:after-backup") && restored.has_outcome("outcome:first"), "restore correct deduplication point");
        }
        Admission original, other, recovered_admission;
        const auto recovery_directory = root / "recovery";
        std::string recovery_code;
        {
            Store recovery_store(recovery_directory.string(), {100}); check(recovery_store.open(&error), "recovery store open");
            original = enroll(recovery_store, 100); other = enroll(recovery_store, 100);
            check(recovery_store.commit_accounts({{original.account_id, "saved-house-a"}, {other.account_id, "private-house-b"}}, "outcome:before-recovery"), "save before credential expiration");
            check(recovery_store.issue_recovery("../../other", 200, 100).empty(), "recovery malformed identity rejected");
            check(recovery_store.issue_recovery("acct_" + std::string(64, '0'), 200, 100).empty(), "recovery unknown foreign account rejected");
            check(recovery_store.issue_recovery(original.account_id, 200, 0).empty(), "recovery invalid lifetime rejected");
            check(!recovery_store.authenticate(original.token, 200), "original credential expired");
            const auto expired_recovery = recovery_store.issue_recovery(original.account_id, 100, 100);
            check(!recovery_store.enroll(expired_recovery, 200), "recovery expires at exact boundary");
            recovery_code = recovery_store.issue_recovery(original.account_id, 200, 100);
            check(!recovery_code.empty() && !recovery_store.authenticate(recovery_code, 200), "recovery is not direct authentication");
            // Leave code outstanding for restart redemption.
        }
        {
            Store recovery_store(recovery_directory.string(), {100}); check(recovery_store.open(&error), "recovery restart");
            const auto result = recovery_store.enroll(recovery_code, 201);
            check(result.has_value() && result->account_id == original.account_id, "expired account recovery preserves identity");
            recovered_admission = *result;
            check(result->token != original.token && result->expires_at_ms == 301, "recovery rotates credential lifetime");
            check(recovery_store.load_account(result->account_id) == "saved-house-a", "recovery preserves saved House snapshot");
            check(recovery_store.load_account(other.account_id) == "private-house-b", "recovery preserves unrelated account");
            check(recovery_store.has_outcome("outcome:before-recovery"), "recovery preserves outcomes");
            check(!recovery_store.enroll(recovery_code, 201), "recovery replay rejected");
            check(!recovery_store.authenticate(original.token, 150), "old credential revoked even before former expiry");
            check(recovery_store.authenticate(result->token, 201).has_value(), "recovered token authenticates");
            const auto rotate = recovery_store.issue_recovery(original.account_id, 201, 100);
            const auto stale = recovery_store.issue_recovery(original.account_id, 201, 100);
            recovery_store.test_interrupt_after_writes(1, false);
            check(!recovery_store.enroll(rotate, 202), "recovery partial failure rejected");
            check(recovery_store.authenticate(result->token, 202).has_value(), "failed recovery restores prior credential");
            const auto rotated = recovery_store.enroll(rotate, 202);
            check(rotated.has_value() && rotated->account_id == original.account_id, "failed recovery code remains usable");
            recovered_admission = *rotated;
            check(!recovery_store.authenticate(result->token, 202), "successful recovery revokes previous valid credential");
            check(!recovery_store.enroll(stale, 202), "successful recovery invalidates other outstanding recovery codes");
            assert_secrets_absent(recovery_directory, {recovery_code, rotate, stale, original.token, recovered_admission.token});
        }
        // An existing version-one store gains the recovery table without resetting accounts.
        {
            sqlite3* migration{}; check(sqlite3_open16((recovery_directory / "service.sqlite").c_str(), &migration) == SQLITE_OK, "migration fixture open");
            check(sqlite3_exec(migration, "DROP TABLE recovery; PRAGMA user_version=1", nullptr, nullptr, nullptr) == SQLITE_OK, "version-one fixture");
            sqlite3_stmt* count{}; sqlite3_prepare_v2(migration, "SELECT count(*) FROM accounts", -1, &count, nullptr);
            check(sqlite3_step(count) == SQLITE_ROW && sqlite3_column_int(count, 0) == 2, "recovery never creates additional accounts");
            sqlite3_finalize(count); sqlite3_close(migration);
            Store migrated(recovery_directory.string()); check(migrated.open(&error), "version-one migration succeeds");
            check(migrated.load_account(original.account_id) == "saved-house-a", "migration preserves House snapshot");
            check(migrated.authenticate(recovered_admission.token, 203).has_value(), "migration preserves recovered credential");
            const auto concurrent_recovery = migrated.issue_recovery(original.account_id, 203, 100);
            check(!concurrent_recovery.empty(), "migrated account supports recovery");
            std::atomic<int> winners{0}; std::vector<std::thread> threads;
            for (int i = 0; i < 16; ++i) threads.emplace_back([&] { if (migrated.enroll(concurrent_recovery, 204)) ++winners; });
            for (auto& thread : threads) thread.join();
            check(winners == 1, "concurrent recovery accepts exactly one redemption");
            check(migrated.load_account(original.account_id) == "saved-house-a", "concurrent recovery preserves House");
        }
        // Refuse future schema without lowering its version or accepting writes.
        const auto version_two = root / "version-two";
        Admission legacy;
        { Store store(version_two.string()); check(store.open(), "version-two fixture setup"); legacy=enroll(store,100);
          check(store.commit_accounts({{legacy.account_id,"legacy-house"}},"legacy-outcome"),"legacy account checkpoint"); }
        {
            sqlite3* migration{};check(sqlite3_open16((version_two / "service.sqlite").c_str(), &migration) == SQLITE_OK, "version-two database open");
            check(sqlite3_exec(migration,"DROP TABLE world_state; PRAGMA user_version=2",nullptr,nullptr,nullptr)==SQLITE_OK,"version-two database fixture");sqlite3_close(migration);
            Store migrated(version_two.string());check(migrated.open(&error),"version-two world-state migration");
            check(migrated.load_account(legacy.account_id)=="legacy-house"&&migrated.has_outcome("legacy-outcome"),"world-state migration preserves accounts and outcomes");
            check(!migrated.load_world_state(&error)&&error.empty(),"world-state migration starts absent without fabricating ledger");
            check(migrated.commit_state({},queued,"migration-world")&&migrated.load_world_state()==queued,"migrated database supports global transactions");
        }
        const auto future = root / "future"; fs::create_directory(future); sqlite3* db{};
        check(sqlite3_open16((future / "service.sqlite").c_str(), &db) == SQLITE_OK, "future fixture open");
        check(sqlite3_exec(db, "PRAGMA user_version=999", nullptr, nullptr, nullptr) == SQLITE_OK, "future schema fixture"); sqlite3_close(db);
        Store future_store(future.string()); check(!future_store.open(&error), "future schema rejected");
        std::cout << "service_store_tests: " << assertions << " assertions passed; Windows SQLite, 16-thread races, 5 real crash recoveries, atomic account/world state, online backup/restore\n";
        // Preserve contained QA databases for evidence; all identities are disposable.
        return 0;
    } catch (const std::exception& e) { std::cerr << "service_store_tests failed: " << e.what() << '\n'; return 1; }
}
