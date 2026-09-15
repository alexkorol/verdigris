#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace verdigris::service {

struct Admission {
    std::string account_id;
    std::string token;
    std::int64_t expires_at_ms{};
};

struct StoreConfig {
    // Reconnect credential lifetime, fixed from enrollment (no sliding expiry).
    std::int64_t session_ttl_ms{30LL * 24 * 60 * 60 * 1000};
};

// Windows 10+ service adapter; keep outside the deterministic simulation.
// One Store owns a fixed local-disk data directory exclusively until destruction.
// Public methods serialize calls. Snapshots are opaque server-owned UTF-8 JSON;
// authorization and gameplay validation must happen before commit_accounts.
class Store {
public:
    explicit Store(std::string directory, StoreConfig config = {});
    ~Store();
    Store(const Store&) = delete;
    Store& operator=(const Store&) = delete;

    bool open(std::string* error = nullptr);
    std::string issue_enrollment(std::int64_t now, std::int64_t ttl_ms, std::string* error = nullptr);
    // Operator-only, after out-of-band ownership verification. Never expose as
    // an unauthenticated client endpoint. enroll also redeems these rec_ codes.
    std::string issue_recovery(const std::string& account_id, std::int64_t now,
                               std::int64_t ttl_ms, std::string* error = nullptr);
    std::optional<Admission> enroll(const std::string& code, std::int64_t now, std::string* error = nullptr);
    std::optional<Admission> authenticate(const std::string& token, std::int64_t now, std::string* error = nullptr);
    bool revoke(const std::string& token, std::string* error = nullptr);
    std::optional<std::string> load_account(const std::string& account_id, std::string* error = nullptr);
    // Nonempty outcome IDs are globally unique, restart-safe server IDs.
    // A duplicate outcome is a no-op success. Empty ID = ordinary atomic save.
    bool commit_accounts(const std::map<std::string, std::string>& snapshots,
                         const std::string& outcome_id, std::string* error = nullptr);
    // Persist opaque service-wide ownership state (e.g. a relic ledger) in the
    // SAME transaction as every affected account and outcome deduplication.
    // Empty account batches are allowed. commit_accounts preserves this state.
    bool commit_state(const std::map<std::string, std::string>& snapshots,
                      const std::string& global_state, const std::string& outcome_id,
                      std::string* error = nullptr);
    // Missing state returns nullopt with an empty error; callers may initialize
    // a fresh ledger only in that case, never after a storage read error.
    std::optional<std::string> load_world_state(std::string* error = nullptr);
    bool has_outcome(const std::string& outcome_id);

    // Consistent SQLite online backup. Destination directory MUST NOT exist.
    // Backup contains credential hashes and private saves: protect like live data.
    bool backup_to(const std::string& fresh_directory, std::string* error = nullptr);
    // Restore into a fresh directory; never overwrites a running/existing store.
    static bool restore_backup(const std::string& backup_directory,
                               const std::string& fresh_directory, std::string* error = nullptr);

#ifdef VERDIGRIS_SERVICE_STORE_TESTING
    // Test binary only; never wired to client input or production configuration.
    void test_interrupt_after_writes(int writes, bool terminate_process);
#endif

private:
    bool commit_impl(const std::map<std::string, std::string>& snapshots,
                     const std::string* global_state, const std::string& outcome_id,
                     std::string* error);
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace verdigris::service
