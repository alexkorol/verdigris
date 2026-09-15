#include "verdigris/service_store.hpp"

#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <winsqlite/winsqlite3.h>
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "winsqlite3.lib")

#include <array>
#include <filesystem>
#include <fstream>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <vector>

namespace verdigris::service {
namespace {
constexpr std::size_t max_snapshot_size = 16 * 1024 * 1024;
constexpr std::size_t max_commit_size = 64 * 1024 * 1024;
constexpr char backup_marker[] = "Verdigris service backup v1\n";
std::filesystem::path utf8_path(const std::string& text) {
    return std::filesystem::path(std::u8string(text.begin(), text.end()));
}
void clear_error(std::string* error) { if (error) error->clear(); }
void set_error(std::string* error, const std::exception& e) { if (error) *error = e.what(); }
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
void sql_check(int result, sqlite3* db) {
    if (result != SQLITE_OK && result != SQLITE_DONE && result != SQLITE_ROW)
        throw std::runtime_error(std::string("Service storage: ") + (db ? sqlite3_errmsg(db) : "SQLite operation failed"));
}
void exec(sqlite3* db, const char* sql) { sql_check(sqlite3_exec(db, sql, nullptr, nullptr, nullptr), db); }
struct Statement {
    sqlite3* db;
    sqlite3_stmt* value{};
    Statement(sqlite3* database, const char* sql) : db(database) {
        sql_check(sqlite3_prepare_v2(db, sql, -1, &value, nullptr), db);
    }
    ~Statement() { sqlite3_finalize(value); }
    void bind(int index, const std::string& text) {
        require(text.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()), "Storage value too large");
        sql_check(sqlite3_bind_text(value, index, text.data(), static_cast<int>(text.size()), SQLITE_TRANSIENT), db);
    }
    void bind(int index, std::int64_t number) { sql_check(sqlite3_bind_int64(value, index, number), db); }
    int step() { const int result = sqlite3_step(value); sql_check(result, db); return result; }
    std::string text(int column) {
        const auto* ptr = sqlite3_column_text(value, column);
        return ptr ? std::string(reinterpret_cast<const char*>(ptr), sqlite3_column_bytes(value, column)) : std::string{};
    }
    std::int64_t number(int column) { return sqlite3_column_int64(value, column); }
};
struct Transaction {
    sqlite3* db;
    bool committed{};
    explicit Transaction(sqlite3* database) : db(database) { exec(db, "BEGIN IMMEDIATE"); }
    ~Transaction() { if (!committed) sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr); }
    void commit() { exec(db, "COMMIT"); committed = true; }
};
std::string hex(const unsigned char* bytes, std::size_t count) {
    constexpr char digits[] = "0123456789abcdef";
    std::string result(count * 2, '0');
    for (std::size_t i = 0; i < count; ++i) {
        result[i * 2] = digits[bytes[i] >> 4]; result[i * 2 + 1] = digits[bytes[i] & 15];
    }
    return result;
}
std::string random_hex() {
    std::array<unsigned char, 32> bytes{};
    require(BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(bytes.size()), BCRYPT_USE_SYSTEM_PREFERRED_RNG) >= 0,
            "System random generation failed");
    return hex(bytes.data(), bytes.size());
}
std::string digest(const std::string& text) {
    BCRYPT_ALG_HANDLE algorithm{};
    require(BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) >= 0, "SHA256 provider unavailable");
    BCRYPT_HASH_HANDLE hash{};
    std::array<unsigned char, 32> bytes{};
    const auto created = BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0);
    const auto hashed = created >= 0 ? BCryptHashData(hash, reinterpret_cast<PUCHAR>(const_cast<char*>(text.data())),
                                                    static_cast<ULONG>(text.size()), 0) : created;
    const auto finished = hashed >= 0 ? BCryptFinishHash(hash, bytes.data(), static_cast<ULONG>(bytes.size()), 0) : hashed;
    if (hash) BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    require(finished >= 0, "SHA256 generation failed");
    return hex(bytes.data(), bytes.size());
}
bool valid_secret(const std::string& text, const char* prefix) {
    const std::string start(prefix);
    if (text.size() != start.size() + 64 || text.compare(0, start.size(), start) != 0) return false;
    return text.find_first_not_of("0123456789abcdef", start.size()) == std::string::npos;
}
std::int64_t expires(std::int64_t now, std::int64_t ttl) {
    require(now >= 0 && ttl > 0 && now <= std::numeric_limits<std::int64_t>::max() - ttl, "Invalid credential lifetime");
    return now + ttl;
}
} // namespace

struct Store::Impl {
    std::string directory;
    StoreConfig config;
    std::mutex mutex;
    sqlite3* db{};
    HANDLE lock{INVALID_HANDLE_VALUE};
#ifdef VERDIGRIS_SERVICE_STORE_TESTING
    int interrupt_after{-1};
    bool terminate{};
#endif
    Impl(std::string path, StoreConfig settings) : directory(std::move(path)), config(settings) {}
    ~Impl() { close(); }
    void close() {
        if (db) { sqlite3_close(db); db = nullptr; }
        if (lock != INVALID_HANDLE_VALUE) { CloseHandle(lock); lock = INVALID_HANDLE_VALUE; }
    }
    void ready() { require(db != nullptr, "Service store is not open"); }
    bool has(const std::string& id) {
        Statement query(db, "SELECT 1 FROM outcomes WHERE id=?"); query.bind(1, id);
        return query.step() == SQLITE_ROW;
    }
    void interrupt() {
#ifdef VERDIGRIS_SERVICE_STORE_TESTING
        if (interrupt_after >= 0 && --interrupt_after <= 0) {
            interrupt_after = -1;
            if (terminate) TerminateProcess(GetCurrentProcess(), 77);
            throw std::runtime_error("Injected transaction failure");
        }
#endif
    }
};

Store::Store(std::string directory, StoreConfig config) : impl_(std::make_unique<Impl>(std::move(directory), config)) {}
Store::~Store() = default;

bool Store::open(std::string* error) {
    std::lock_guard<std::mutex> guard(impl_->mutex); clear_error(error);
    try {
        if (impl_->db) return true;
        require(!impl_->directory.empty(), "A service data directory is required");
        expires(0, impl_->config.session_ttl_ms);
        const auto directory = std::filesystem::absolute(utf8_path(impl_->directory));
        require(directory.root_name().wstring().rfind(L"\\\\", 0) != 0, "Service data must use a local disk, not a network share");
        require(GetDriveTypeW(directory.root_path().c_str()) == DRIVE_FIXED, "Service data requires a fixed local disk");
        std::filesystem::create_directories(directory);
        impl_->lock = CreateFileW((directory / "service.lock").c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                                  OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        require(impl_->lock != INVALID_HANDLE_VALUE, "Service data directory is already owned or inaccessible");
        sql_check(sqlite3_open16((directory / "service.sqlite").c_str(), &impl_->db), impl_->db);
        sqlite3_busy_timeout(impl_->db, 5000);
        // Refuse future schemas before any schema mutation.
        { Statement version(impl_->db, "PRAGMA user_version");
          require(version.step() == SQLITE_ROW && version.number(0) <= 3, "Unsupported service database schema"); }
        { Statement journal(impl_->db, "PRAGMA journal_mode=WAL");
          require(journal.step() == SQLITE_ROW && journal.text(0) == "wal", "Service storage requires SQLite WAL"); }
        exec(impl_->db, "PRAGMA synchronous=FULL; PRAGMA foreign_keys=ON; PRAGMA secure_delete=ON; PRAGMA wal_autocheckpoint=256;");
        { Statement sync(impl_->db, "PRAGMA synchronous");
          require(sync.step() == SQLITE_ROW && sync.number(0) == 2, "Service storage requires FULL durability"); }
        { Statement integrity(impl_->db, "PRAGMA quick_check");
          require(integrity.step() == SQLITE_ROW && integrity.text(0) == "ok", "Service database integrity check failed"); }
        Transaction transaction(impl_->db);
        exec(impl_->db,
            "CREATE TABLE IF NOT EXISTS accounts(id TEXT PRIMARY KEY, snapshot TEXT NOT NULL);"
            "CREATE TABLE IF NOT EXISTS enrollment(hash TEXT PRIMARY KEY, expires INTEGER NOT NULL);"
            "CREATE TABLE IF NOT EXISTS credentials(hash TEXT PRIMARY KEY, account_id TEXT NOT NULL REFERENCES accounts(id), expires INTEGER NOT NULL, revoked INTEGER NOT NULL DEFAULT 0);"
            "CREATE TABLE IF NOT EXISTS recovery(hash TEXT PRIMARY KEY, account_id TEXT NOT NULL REFERENCES accounts(id), expires INTEGER NOT NULL);"
            "CREATE TABLE IF NOT EXISTS outcomes(id TEXT PRIMARY KEY);"
            "CREATE TABLE IF NOT EXISTS world_state(id INTEGER PRIMARY KEY CHECK(id=1), snapshot TEXT NOT NULL); PRAGMA user_version=3;");
        transaction.commit();
        return true;
    } catch (const std::exception& e) { impl_->close(); set_error(error, e); return false; }
}

std::string Store::issue_enrollment(std::int64_t now, std::int64_t ttl_ms, std::string* error) {
    std::lock_guard<std::mutex> guard(impl_->mutex); clear_error(error);
    try {
        impl_->ready(); const auto expiry = expires(now, ttl_ms);
        const auto code = "enr_" + random_hex();
        Transaction transaction(impl_->db);
        { Statement cleanup(impl_->db, "DELETE FROM enrollment WHERE expires<=?"); cleanup.bind(1, now); cleanup.step(); }
        Statement insert(impl_->db, "INSERT INTO enrollment(hash,expires) VALUES(?,?)");
        insert.bind(1, digest(code)); insert.bind(2, expiry); insert.step();
        transaction.commit(); return code;
    } catch (const std::exception& e) { set_error(error, e); return {}; }
}

std::string Store::issue_recovery(const std::string& account_id, std::int64_t now, std::int64_t ttl_ms, std::string* error) {
    std::lock_guard<std::mutex> guard(impl_->mutex); clear_error(error);
    try {
        impl_->ready(); require(valid_secret(account_id, "acct_"), "Invalid account identity");
        const auto expiry = expires(now, ttl_ms);
        const auto code = "rec_" + random_hex();
        Transaction transaction(impl_->db);
        { Statement account(impl_->db, "SELECT 1 FROM accounts WHERE id=?"); account.bind(1, account_id);
          require(account.step() == SQLITE_ROW, "Recovery requires an existing account"); }
        { Statement cleanup(impl_->db, "DELETE FROM recovery WHERE expires<=?"); cleanup.bind(1, now); cleanup.step(); }
        Statement insert(impl_->db, "INSERT INTO recovery(hash,account_id,expires) VALUES(?,?,?)");
        insert.bind(1, digest(code)); insert.bind(2, account_id); insert.bind(3, expiry); insert.step();
        transaction.commit(); return code;
    } catch (const std::exception& e) { set_error(error, e); return {}; }
}

std::optional<Admission> Store::enroll(const std::string& code, std::int64_t now, std::string* error) {
    std::lock_guard<std::mutex> guard(impl_->mutex); clear_error(error);
    try {
        impl_->ready(); const bool recovering = valid_secret(code, "rec_");
        require(recovering || valid_secret(code, "enr_"), "Enrollment rejected");
        const auto expiry = expires(now, impl_->config.session_ttl_ms);
        const auto hash = digest(code);
        Transaction transaction(impl_->db);
        std::string account_id;
        if (recovering) {
            Statement lookup(impl_->db, "SELECT expires,account_id FROM recovery WHERE hash=?"); lookup.bind(1, hash);
            require(lookup.step() == SQLITE_ROW && lookup.number(0) > now, "Recovery rejected");
            account_id = lookup.text(1);
        } else {
            Statement lookup(impl_->db, "SELECT expires FROM enrollment WHERE hash=?"); lookup.bind(1, hash);
            require(lookup.step() == SQLITE_ROW && lookup.number(0) > now, "Enrollment rejected");
            account_id = "acct_" + random_hex();
        }
        Admission admission{account_id, "ses_" + random_hex(), expiry};
        if (recovering) {
            Statement revoke(impl_->db, "UPDATE credentials SET revoked=1 WHERE account_id=?");
            revoke.bind(1, account_id); revoke.step();
        } else {
            Statement insert(impl_->db, "INSERT INTO accounts(id,snapshot) VALUES(?,'{}')");
            insert.bind(1, account_id); insert.step();
        }
        impl_->interrupt();
        { Statement insert(impl_->db, "INSERT INTO credentials(hash,account_id,expires) VALUES(?,?,?)");
          insert.bind(1, digest(admission.token)); insert.bind(2, admission.account_id); insert.bind(3, expiry); insert.step(); }
        if (recovering) {
            // Successful recovery invalidates all outstanding recovery codes for
            // the account as well as every previous reconnect credential.
            Statement consume(impl_->db, "DELETE FROM recovery WHERE account_id=?"); consume.bind(1, account_id); consume.step();
        } else {
            Statement consume(impl_->db, "DELETE FROM enrollment WHERE hash=?"); consume.bind(1, hash); consume.step();
        }
        transaction.commit(); return admission;
    } catch (const std::exception& e) { set_error(error, e); return std::nullopt; }
}

std::optional<Admission> Store::authenticate(const std::string& token, std::int64_t now, std::string* error) {
    std::lock_guard<std::mutex> guard(impl_->mutex); clear_error(error);
    try {
        impl_->ready(); require(now >= 0 && valid_secret(token, "ses_"), "Authentication rejected");
        Statement lookup(impl_->db, "SELECT account_id,expires FROM credentials WHERE hash=? AND revoked=0 AND expires>?");
        lookup.bind(1, digest(token)); lookup.bind(2, now);
        require(lookup.step() == SQLITE_ROW, "Authentication rejected");
        return Admission{lookup.text(0), token, lookup.number(1)};
    } catch (const std::exception& e) { set_error(error, e); return std::nullopt; }
}

bool Store::revoke(const std::string& token, std::string* error) {
    std::lock_guard<std::mutex> guard(impl_->mutex); clear_error(error);
    try {
        impl_->ready(); require(valid_secret(token, "ses_"), "Invalid credential format");
        Statement update(impl_->db, "UPDATE credentials SET revoked=1 WHERE hash=?"); update.bind(1, digest(token)); update.step();
        return true;
    } catch (const std::exception& e) { set_error(error, e); return false; }
}

std::optional<std::string> Store::load_account(const std::string& id, std::string* error) {
    std::lock_guard<std::mutex> guard(impl_->mutex); clear_error(error);
    try {
        impl_->ready(); require(valid_secret(id, "acct_"), "Invalid account identity");
        Statement lookup(impl_->db, "SELECT snapshot FROM accounts WHERE id=?"); lookup.bind(1, id);
        if (lookup.step() == SQLITE_ROW) return lookup.text(0);
        return std::nullopt;
    } catch (const std::exception& e) { set_error(error, e); return std::nullopt; }
}

bool Store::commit_accounts(const std::map<std::string, std::string>& snapshots, const std::string& outcome, std::string* error) {
    return commit_impl(snapshots, nullptr, outcome, error);
}

bool Store::commit_state(const std::map<std::string, std::string>& snapshots, const std::string& global_state,
                         const std::string& outcome, std::string* error) {
    return commit_impl(snapshots, &global_state, outcome, error);
}

std::optional<std::string> Store::load_world_state(std::string* error) {
    std::lock_guard<std::mutex> guard(impl_->mutex); clear_error(error);
    try {
        impl_->ready(); Statement lookup(impl_->db, "SELECT snapshot FROM world_state WHERE id=1");
        if (lookup.step() == SQLITE_ROW) return lookup.text(0);
        return std::nullopt;
    } catch (const std::exception& e) { set_error(error, e); return std::nullopt; }
}

bool Store::commit_impl(const std::map<std::string, std::string>& snapshots, const std::string* global_state,
                        const std::string& outcome, std::string* error) {
    std::lock_guard<std::mutex> guard(impl_->mutex); clear_error(error);
    try {
        impl_->ready(); require(outcome.size() <= 256, "Outcome identity too long");
        Transaction transaction(impl_->db);
        if (!outcome.empty() && impl_->has(outcome)) { transaction.commit(); return true; }
        require((!snapshots.empty() || global_state) && snapshots.size() <= 256, "Invalid account transaction size");
        std::size_t total = global_state ? global_state->size() : 0;
        require(total <= max_snapshot_size, "Global state snapshot too large");
        for (const auto& [id, snapshot] : snapshots) {
            require(valid_secret(id, "acct_"), "Invalid account identity");
            require(snapshot.size() <= max_snapshot_size, "Account snapshot too large");
            total += snapshot.size(); require(total <= max_commit_size, "Account transaction too large");
            Statement update(impl_->db, "UPDATE accounts SET snapshot=? WHERE id=?");
            update.bind(1, snapshot); update.bind(2, id); update.step();
            require(sqlite3_changes(impl_->db) == 1, "Account transaction references an unknown account");
            impl_->interrupt();
        }
        if (global_state) {
            Statement update(impl_->db, "INSERT OR REPLACE INTO world_state(id,snapshot) VALUES(1,?)");
            update.bind(1, *global_state); update.step(); impl_->interrupt();
        }
        if (!outcome.empty()) {
            Statement insert(impl_->db, "INSERT INTO outcomes(id) VALUES(?)"); insert.bind(1, outcome); insert.step();
        }
        impl_->interrupt(); transaction.commit(); return true;
    } catch (const std::exception& e) { set_error(error, e); return false; }
}

bool Store::has_outcome(const std::string& outcome) {
    std::lock_guard<std::mutex> guard(impl_->mutex);
    // Errors must not be mistaken for absence: this read throws on storage failure.
    impl_->ready(); return !outcome.empty() && impl_->has(outcome);
}

bool Store::backup_to(const std::string& fresh_directory, std::string* error) {
    std::lock_guard<std::mutex> guard(impl_->mutex); clear_error(error);
    try {
        impl_->ready(); const auto path = utf8_path(fresh_directory);
        require(std::filesystem::create_directory(path), "Backup destination must be a fresh directory");
        {
            Store backup(fresh_directory, impl_->config);
            std::string failure; require(backup.open(&failure), "Cannot open backup destination");
            auto* operation = sqlite3_backup_init(backup.impl_->db, "main", impl_->db, "main");
            require(operation != nullptr, "Cannot initialize service backup");
            const int result = sqlite3_backup_step(operation, -1);
            const int finished = sqlite3_backup_finish(operation);
            require(result == SQLITE_DONE && finished == SQLITE_OK, "Service backup failed; discard incomplete destination");
            exec(backup.impl_->db, "PRAGMA wal_checkpoint(TRUNCATE)");
        }
        // A crash/failure during backup must never leave a restorable empty store.
        const auto marker = CreateFileW((path / "backup.complete").c_str(), GENERIC_WRITE, 0, nullptr,
                                        CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        require(marker != INVALID_HANDLE_VALUE, "Cannot mark completed backup");
        DWORD written{};
        const bool marked = WriteFile(marker, backup_marker, sizeof(backup_marker) - 1, &written, nullptr) &&
                            written == sizeof(backup_marker) - 1 && FlushFileBuffers(marker);
        CloseHandle(marker);
        require(marked, "Cannot durably mark completed backup");
        return true;
    } catch (const std::exception& e) { set_error(error, e); return false; }
}

bool Store::restore_backup(const std::string& backup_directory, const std::string& fresh_directory, std::string* error) {
    clear_error(error);
    try {
        const auto path = utf8_path(backup_directory);
        require(std::filesystem::is_regular_file(path / "service.sqlite"), "Backup database is missing");
        std::ifstream marker(path / "backup.complete", std::ios::binary);
        const std::string marker_text{std::istreambuf_iterator<char>(marker), std::istreambuf_iterator<char>()};
        require(marker_text == backup_marker, "Backup is incomplete or lacks its completion marker");
        Store backup(backup_directory);
        if (!backup.open(error)) return false;
        return backup.backup_to(fresh_directory, error);
    } catch (const std::exception& e) { set_error(error, e); return false; }
}

#ifdef VERDIGRIS_SERVICE_STORE_TESTING
void Store::test_interrupt_after_writes(int writes, bool terminate_process) {
    std::lock_guard<std::mutex> guard(impl_->mutex);
    impl_->interrupt_after = writes; impl_->terminate = terminate_process;
}
#endif
} // namespace verdigris::service
