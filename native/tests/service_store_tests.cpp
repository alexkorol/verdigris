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
void crash_child(const fs::path& directory, const std::string& a, const std::string& b, int writes) {
    wchar_t executable[32768]{}; GetModuleFileNameW(nullptr, executable, 32768);
    std::wstring command = L"\"" + std::wstring(executable) + L"\" --crash \"" + directory.wstring() + L"\" " +
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
        if (argc == 6 && std::string(argv[1]) == "--crash") {
            Store store(argv[2]); if (!store.open()) return 78;
            store.test_interrupt_after_writes(std::stoi(argv[5]), true);
            store.commit_accounts({{argv[3], "crash-a"}, {argv[4], "crash-b"}}, "outcome:crashed");
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
        // Refuse future schema without lowering its version or accepting writes.
        const auto future = root / "future"; fs::create_directory(future); sqlite3* db{};
        check(sqlite3_open16((future / "service.sqlite").c_str(), &db) == SQLITE_OK, "future fixture open");
        check(sqlite3_exec(db, "PRAGMA user_version=999", nullptr, nullptr, nullptr) == SQLITE_OK, "future schema fixture"); sqlite3_close(db);
        Store future_store(future.string()); check(!future_store.open(&error), "future schema rejected");
        std::cout << "service_store_tests: " << assertions << " assertions passed; Windows SQLite, 16-thread races, 2 real crash recoveries, online backup/restore\n";
        // Preserve contained QA databases for evidence; all identities are disposable.
        return 0;
    } catch (const std::exception& e) { std::cerr << "service_store_tests failed: " << e.what() << '\n'; return 1; }
}
