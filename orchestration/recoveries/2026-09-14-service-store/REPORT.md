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
  WAL + synchronous FULL. Schema version 3 contains accounts, hashed enrollment
  and account recovery codes, hashed credentials, and globally unique outcome
  IDs plus opaque singleton global state. Version-one/two stores migrate
  transactionally without resetting accounts.
- `issue_enrollment(now, ttl_ms, error)` returns a one-use 256-bit random code.
  `enroll(code, now, error)` atomically consumes the code and creates independent
  random `acct_` identity plus `ses_` reconnect token. Only SHA256 hashes of
  codes/tokens reach storage. Expiry is exclusive (`now < expires`).
- `authenticate(token, now, error)` returns the same account/token/expiry;
  `revoke(token,error)` durably revokes the credential, idempotently.
- `issue_recovery(account_id,now,ttl,error)` is operator-only after out-of-band
  account ownership verification. It issues a hashed-at-rest one-use `rec_` code
  for an existing account. `enroll` redeems it, preserves the account ID and all
  snapshots/outcomes, atomically revokes all previous credentials, issues a new
  token, and invalidates every outstanding recovery code for that account.
  Failed recovery rolls back credential revocations and leaves its code usable.
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
- `commit_state(accounts,global_state,outcome_id,error)` atomically commits the
  opaque global ledger with every affected account and outcome record. Empty
  account batches are allowed for global-only transitions. A duplicate outcome
  is a no-op for both accounts and global state. Global state is limited to
  16 MiB and counts toward the existing 64 MiB total transaction bound.
  `load_world_state(error)` returns null with no error when absent. Existing
  `commit_accounts` never overwrites global state. Parsing and relic ownership
  rules remain the parent's authority; this adapter guarantees transactionality.
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

Both exit 0; **202 assertions passed**. Tests cover wrong credential kinds,
arbitrary guest identities, time overflow, exact enrollment/session expiry,
revocation and restart, one-use enrollment rollback/replay, private distinct
snapshots, unknown account failure, duplicate outcomes, limits, 16 concurrent
enrollment attempts (one winner), 16 concurrent outcome attempts (one shared
atomic result), failed writes after first account and after outcome insert,
and scans proving plaintext codes/tokens absent from DB/WAL files.

Five real child processes terminate with code 77 inside uncommitted transactions:
two account-only failures after the first write and after outcome insertion,
and three combined account/global failures after account, global, and outcome
writes. Fresh processes successfully
open/recover the WAL; both original account snapshots and absence of the failed
outcome are checked. Live backup then further live changes then restore proves
backup-point snapshots, credentials, and outcome deduplication survive. Missing,
unmarked, and existing-destination restore cases fail safely. Future schema fails.
Recovery checks include expired token -> same saved House/account, unchanged
foreign account, no new accounts, original valid/expired token revocation, replay,
expired recovery, unknown identity, transaction failure, restart redemption,
outstanding-code invalidation, 16-thread one-winner redemption, and version-one
schema migration preserving recovered credentials and House state.
Combined-state tests also cover absent-vs-error reads, global-only saves,
ordinary account saves preserving the global ledger, duplicate combined outcome
no-ops, oversize rejection, and backup/restore of the same source inventory,
recipient inventory, global ownership snapshot and deduplication point.
Version-two migration creates an absent global ledger without fabricating state.

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
days from enrollment/recovery; there is no sliding token refresh. Expired/revoked
credentials fail closed. Operators can verify account ownership out-of-band and
issue a one-use recovery code through `issue_recovery`; redeem through the normal
enrollment UI. Never expose recovery-code issuance to unauthenticated clients.
Client secret storage, TLS, rate limiting,
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

## Focused integration authority regression (subsequent parent assignment)

Added only `native/tests/service_authority_tests.cpp` under explicit parent
assignment; no parent checkout edits. This direct `ProtocolSession` gate requires
the parent's service integration API and is intentionally not wired into this
worker branch's historical build. It is a **failing regression baseline**, not
passing service acceptance. Parent owns production corrections and integration.

Captured parent `native/src/networking.cpp` SHA256:
`D2B99826A96BFE74EDBB90BF1E2CE635A7831F2D808B14326AC174B4B97FDC6E`.
Copied source/object outputs only into this worktree's ignored
`native/build/authority-audit`; parent files remained read-only.
MSVC compilation succeeded. Running `service_authority_tests.exe` returned
**exit 1: 10 checks, 10 failures**, reproducing:

- client `mortal:false` can remove an admitted oath;
- selecting a persisted crypt Scion revives it;
- zero/negative client shop prices purchase below the authoritative price;
- arbitrary shop item `coins` mints currency;
- shop purchases work from an expedition, away from the town service;
- bank deposit quantity -1 mints carried currency; zero creates invalid bank data;
- founding/entering another House leaks the first bank and loses its treasury checkpoint.

Set-up uses ordinary found/create/set-out commands. The crypt test arranges a
server-owned persisted death snapshot to model restart; the private-bank test
can arrange a server-owned bank snapshot once corrected proximity checks reject
its initial ordinary deposit. Neither sends a development or client-save command.
Build the test against the parent's current `core`, `seasonal`, `networking`, and
`service_store` objects and headers, then run it as a new service authority gate.
Fixes must make these assertions pass before the integrated candidate is promoted.

## Shared actor and scheduling regression (subsequent parent assignment)

Added only `native/tests/service_actor_tests.cpp`; parent still owns all module
edits and build wiring. The new gate compiled and returned **exit 0: 27 checks,
0 failures** against captured parent sources:

- networking SHA256 `F793B765086F3ED565FEA6557F70E33C446D4E5CA452F408238B4247C41ADE25`
- core SHA256 `15338A106429ED9AE0DB940E1841DA1BEC904CF081C3CD0DF3DD1AE9904B90CD`

Coverage: distinct pending targets/windups, isolated critical modifiers,
independent attack recovery, two actors damaging one shared enemy with exact
event-based damage conservation, one actor's disengagement/exit preserving the
ally, one shared boss telegraph and one attributed impact for each in-range
actor, repeated/stale sample rejection, extra participant pursuit compared
against the same-seed single-participant clock, and independent dash recovery.

Direct service sessions use normal found/create/set-out admission, then send
1,000 movement packets before a tick, 1,000 duplicate ticks, stale/fresh movement
sequences, stop intent, and a fresh client sequence after socket replacement.
Another 1,000 attack commands are checked to leave enemy/ally damage unchanged
until the authority tick resolves contact. World arrangement uses deterministic
core fixture seams, not service development commands. This is headless actor and
service-command evidence, not graphical, socket soak, or external-network proof.

Outputs and captured source/object copies are only under this worker's ignored
`native/build/actor-audit`. Compile the new file with the integrated core,
seasonal, networking, and service_store objects/headers to wire the gate. The
test source compiled cleanly; captured production sources retain existing
unreferenced-parameter, cast, and inet_addr deprecation warnings in this audit
build, which did not use /WX for those parent modules.

Read-only review also identified the old reconnect movement sequence retained
across socket replacement; parent corrected that seam before this source
capture and the new regression passes. Earlier source findings about reader
thread reaping, lifetime resident session caps, and failed-pickup side effects
were sent to parent for bounded correction; this lane does not claim those
findings were runtime soak results or all were corrected by this actor gate.
