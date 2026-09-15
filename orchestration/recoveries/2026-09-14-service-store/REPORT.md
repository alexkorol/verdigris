# Native service authentication and transactional store

Status: Implemented and locally tested. Integration/packaging/player acceptance
belong to the parent service/co-op task; this lane makes no gameplay claim.

Implementation commit: `2dca7f4ba33734f56e7b853e800b2691ac0b78c5`, pushed and
remote-verified (`git ls-remote`) on `origin/codex/native-service-store-20260914`.

## Implementation

New `service_store.hpp`, `service_store.cpp`, `service_store_tests.cpp` only.
Namespace `verdigris::service`; public `Admission`, `StoreConfig`, and noncopyable
PIMPL `Store`. All calls serialize under one mutex; a Windows exclusive file
handle owns `service.lock` until destruction, including during schema checks.
UNC/network/removable data roots are rejected; use a fixed local service disk.

- `open(error)` validates SQLite integrity, refuses future schemas, and requires
  WAL + synchronous FULL. Schema version 1 contains accounts, hashed enrollment
  codes, hashed credentials, and globally unique outcome IDs.
- `issue_enrollment(now, ttl_ms, error)` returns a one-use 256-bit random code.
  `enroll(code, now, error)` atomically consumes the code and creates independent
  random `acct_` identity plus `ses_` reconnect token. Only SHA256 hashes of
  codes/tokens reach storage. Expiry is exclusive (`now < expires`).
- `authenticate(token, now, error)` returns the same account/token/expiry;
  `revoke(token,error)` durably revokes the credential, idempotently.
- `load_account(id,error)` returns an opaque server-owned snapshot. Missing
  account is null with no error; malformed identity or storage error sets error.
- `commit_accounts(map, outcome_id,error)` updates existing accounts and records
  the outcome in one immediate transaction. Duplicate nonempty outcome IDs are
  no-op successes, including after restart. Empty ID performs an ordinary atomic
  checkpoint. Unknown accounts, oversized snapshots, or any write/commit failure
  roll back the entire transaction. Limits: 256 accounts, 16 MiB per account,
  64 MiB total, 256-byte outcome ID.
- `has_outcome(id)` throws on storage errors so failures cannot be mistaken for
  absence. Callers must catch/fail closed. Other operations return error values.
- `backup_to(fresh_directory,error)` uses SQLite online backup while callers are
  serialized, closes/checkpoints the backup, then flushes `backup.complete`.
  `restore_backup(backup_directory,fresh_directory,error)` requires that marker,
  locks/validates the source, and copies it consistently into a fresh directory.
  Existing destinations are never overwritten. Failed destinations remain for
  diagnosis and lack an accepted completion marker; discard them administratively.

## Windows dependencies and primary references

No vendored binary or new redistributable. Source uses installed Windows SDK
`winsqlite/winsqlite3.h`, `winsqlite3.lib` and `bcrypt.lib` with MSVC library
pragmas. Runtime: Windows 10+ system `winsqlite3.dll` and BCrypt. This host's
`C:/Windows/System32/winsqlite3.dll` reports 3.43.2. Windows services must keep OS
security updates current; SQLite is serviced by Windows. No Linux build claim.
SQLite is public-domain software; Windows system components are OS dependencies
and are not copied into the service package.

- [Microsoft BCryptGenRandom](https://learn.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgenrandom):
  system-preferred cryptographic RNG, 32 random bytes for every code/token/ID.
- [Microsoft CNG hash example](https://learn.microsoft.com/en-us/windows/win32/seccng/creating-a-hash-with-cng):
  SHA256 through BCrypt algorithm/hash APIs.
- [SQLite WAL](https://www.sqlite.org/wal.html) and
  [synchronous FULL](https://www.sqlite.org/pragma.html#pragma_synchronous):
  FULL WAL flushes each transaction commit.
- [SQLite online backup API](https://www.sqlite.org/backup.html): consistent
  backup rather than copying a live database file without its WAL.
- The installed Windows SDK header is the published C API declaration:
  `C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/um/winsqlite/winsqlite3.h`.

## Verification

In this worktree, initialize MSVC via
`C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Auxiliary/Build/vcvars64.bat`, then:

```text
cl /nologo /std:c++20 /EHsc /W4 /WX /DVERDIGRIS_SERVICE_STORE_TESTING /Inative/include native/src/service_store.cpp native/tests/service_store_tests.cpp /Fo:native/build/ /Fe:native/build/service_store_tests.exe
native/build/service_store_tests.exe
```

Both exit 0; **95 assertions passed**. Tests cover wrong credential kinds,
arbitrary guest identities, time overflow, exact enrollment/session expiry,
revocation and restart, one-use enrollment rollback/replay, private distinct
snapshots, unknown account failure, duplicate outcomes, limits, 16 concurrent
enrollment attempts (one winner), 16 concurrent outcome attempts (one shared
atomic result), failed writes after first account and after outcome insert,
and scans proving plaintext codes/tokens absent from DB/WAL files.

Two real child processes terminate with code 77 inside an uncommitted transaction
after the first write and after outcome insertion. Fresh processes successfully
open/recover the WAL; both original account snapshots and absence of the failed
outcome are checked. Live backup then further live changes then restore proves
backup-point snapshots, credentials, and outcome deduplication survive. Missing,
unmarked, and existing-destination restore cases fail safely. Future schema fails.

Production build (without test macro), `/W4 /WX`: exit 0. `dumpbin /symbols`
contains neither `test_interrupt_after_writes` nor `TerminateProcess`.
`python native/tools/check_legacy_denylist.py`: PASS. `git diff --check`: clean.
QA databases remain contained under ignored `native/build/service-store-test-*`;
they are disposable and must not be packaged. No owner game/profile touched.

## Integration and operational boundaries

Parent adds the source to server/test build targets and invokes this focused
suite, followed by the full native gate and actual service client acceptance.
Do not include `VERDIGRIS_SERVICE_STORE_TESTING` in production targets.
On non-MSVC Windows link `winsqlite3` and `bcrypt` explicitly.

Service owns wall-clock `now`; never accept client timestamps for admission.
Authenticate before deriving account ownership. `commit_accounts` intentionally
does not parse JSON or adjudicate item/House ownership: snapshots must come from
the authoritative server, and response/side-effects follow successful commit.
Generate outcome IDs on the server with restart-safe uniqueness. If an operation
is retried, reuse its outcome ID and check deduplication before mutating live state.

Session TTL is configurable (`StoreConfig::session_ttl_ms`), default fixed 30
days from enrollment; there is no token refresh or operator account recovery
API in the assigned interface. Expired/revoked credentials fail closed; deleting
the client token is not a way to reclaim an account. Parent must expose the
lifetime clearly or request an account-preserving recovery extension before
claiming indefinite account access. Client secret storage, TLS, rate limiting,
operator code delivery without logging, and active socket fencing are parent scope.

Protect live and backup directories with service-identity/admin ACLs: snapshot
data is private and not encrypted by this adapter. Do not copy an active SQLite
file manually; call backup API, then retain the whole completed backup directory.
Restore into a new directory, verify using the service, and change the service's
data-dir setting during a stopped-service maintenance interval. Restoring an old
backup restores credential revocation/expiry to that point too; apply operational
credential revocations again if required. Test evidence covers process crashes;
physical power-loss/media-failure testing is not claimed. FULL durability depends
on the filesystem/storage device honoring flush requests.

No deployment, package promotion, separate-computer play, or owner acceptance is
claimed by this storage lane.
