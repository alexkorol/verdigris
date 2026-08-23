# TASK-0097 — Native persistence durability and fault-model audit (FINDINGS)

- Lane `ox-pc-bc`, model `openrouter/stealth/ox-alpha`.
- SPEC base commit: `d2423873c577d299b3b39c56024d1d840993c72b`
  (verified ancestor of this branch; branch fast-forwarded to program tip
  `0bee7f1e` before work).
- Claim head at audit start: `c289156ae46979efe32ce53e4d5c928e69f4ce43`
  (`STATUS.md` claim). Final evidence head is frozen in `REPORT.md`.
- Capsule honored: read-only; no real owner save was opened or modified; no
  ports bound; port 6500 untouched; only task-folder paths changed.
- Machine-readable twin of this report:
  `captures/persistence-contract.json`.

## 0. Executive summary

The native persistence **library** is small, canonical, fail-closed, and
well-tested as a unit: a versioned line-oriented snapshot format
(`native/src/core.cpp:964`, `1226-1295`) with byte-stable output, unknown-key
tolerance, and an atomic temp+rename file adapter
(`native/persistence/adapter.hpp:23-57`). D-106/D-109 semantics at the
simulation boundary are locked by five focused tests
(`native/tests/core_tests.cpp:755-891`).

The durability **system**, however, does not exist yet in production:

1. **No production caller.** Nothing under `native/src` or `native/include`
   calls `verdigris::snapshot`, `restore`, or any adapter function —
   `server_main.cpp` never includes persistence
   (`native/src/server_main.cpp:1-41`). There are no save triggers, no file
   locations, no load-on-startup. The only callers are tests.
2. **The real player profile has no serialization seam at all.** The live
   guest/House/Scion state players accumulate is `ProtocolSession`'s ~40 fields
   (`native/include/verdigris/networking.hpp:161-247`: inventory, wear set,
   personal bank, house store, quest chain, passive tree, chronicle JSON,
   lifecycle/mortal oath, combat XP…). It is held in
   `WebSocketServer::sessions_` keyed by identity for the lifetime of one
   server process (`native/src/networking.cpp:2988`) and dies with the process.

**Consequence:** within a running process, D-109 holds (disconnect keeps the
in-memory session). Across a crash or restart it does not: the same identity
silently receives a fresh deterministic profile. The client reconnect flow is
proven across a server restart (`native/tests/session_tests.cpp:401-450`) but
asserts only identity and a non-empty scene — not progress preservation,
because there is none to assert.

## 1. Scope, method, and evidence discipline

Every claim below carries a `file:line` citation verified in this worktree at
the audit head. Test-behavior claims cite committed test code (not run logs);
no gates were re-run because this lane's capsule forbids binding ports and the
SPEC acceptance requires source mapping rather than gate execution. The four
literal acceptance commands were executed with transcripts captured in
`REPORT.md`.

## 2. Layer map

### 2.1 Core snapshot format (`verdigris::snapshot` / `restore`)

| Property | Value | Citation |
|---|---|---|
| Version seam | `kSnapshotSchemaVersion = 1`; restore hard-throws on missing/mismatched version, no migration path | `native/src/core.cpp:964`, `1297-1301` |
| Grammar | `key=value` lines; strings hex-encoded lower-case; numbers decimal; booleans `0/1`; collections emit `<key>.count` then numeric indexes; fixed field order ⇒ byte-stable output | `core.cpp:968-1053`, `persistence/README.md:3-19` |
| Parse policy | first occurrence wins for duplicate keys; CRLF tolerated; separator-less/blank lines skipped; unknown keys ignored | `core.cpp:1079-1093`, `1088-1090` comment |
| Fail-closed behavior | missing required field throws naming the key (`1101-1105`); malformed numbers (`1107-1119`), booleans (`1127-1137`), hex strings (`979-996`) throw; collection count capped at 1,000,000 as an allocation guard (`1143-1147`) |
| Restore reset | constructs a seedless `Simulation(0)`, overwrites every durable field from bytes, clears events/actors/instance; RNG `state`+`serial` restored so restarts cannot reroll drops | `core.cpp:1303-1310`, `1352-1374`; ADR-002 consequence |
| Player actor rebuild | restore rebuilds exactly one player Actor; the first equipped carried item becomes `equipped_item_id` (`1367-1372`) |

### 2.2 File adapter (`native/persistence/adapter.hpp`, header-only)

| Property | Value | Citation |
|---|---|---|
| Write path | `<target>.tmp` → open/trunc → write → flush → close → atomic replace: `MoveFileExW(MOVEFILE_REPLACE_EXISTING \| MOVEFILE_WRITE_THROUGH)` on Windows (because `std::filesystem::rename` cannot replace there), POSIX `rename(2)` elsewhere; failure removes the temp and throws | `adapter.hpp:23-57`, Windows rationale `37-40` |
| Read path | plain sized byte read; ignores `.tmp` leftovers | `adapter.hpp:59-73` |
| Durability gap | no `FlushFileBuffers`/`fsync` of the temp data before rename; `MOVEFILE_WRITE_THROUGH` forces the *rename metadata* through disk, not necessarily the stream contents written just before | `adapter.hpp:27-47` |
| Concurrency gap | fixed `.tmp` suffix means two writers to one target share/clobber one temp path | `adapter.hpp:25` |
| Production linkage | exposed via `native/include/verdigris/persistence.hpp:6`; included ONLY by `native/tests/core_tests.cpp:9` | grep evidence §6 |

### 2.3 Session layer (the actual player-facing profile)

- Identity resolution on `player:login`: `guestId` → `playtestGuestId` →
  `"default-guest"` (`networking.cpp:2988`).
- Session adoption: existing identity reuses the stored `ProtocolSession`,
  `replace_socket` (`networking.cpp:590`); if adopted from a different socket,
  `reset_world_for_new_socket()` runs (`604-651`): transient fields reset
  (live tree points, respawn wards, panes, node position), life restored to
  max, world returned to town, while inventory/wear/bank/house store/passive
  tree/quest chain/chronicle/mortal oath persist in memory. A `permadead`
  lifecycle is never resurrected by re-login (`620-634`); a sworn mortal oath
  is re-derived from the chronicle roster (`626-633`).
- Disconnect removes only the `Connection`; the session object survives
  forever in `sessions_` (`2990`, `2988`) — also an unbounded-retention risk.
- Old socket gets `player:session-replaced` and is closed (`2988` tail).
- Same-socket hot re-login keeps the active instance including ground items
  (`604-610` comment; test `native/tests/networking_tests.cpp:268-283`).
- Restart/crash: `sessions_` vanishes; fresh FNV-of-identity seeded profile
  (`2988` seed loop). Client `Retrying→Ready` across restart proven in
  `session_tests.cpp:401-450`; progress equality NOT asserted (none exists).

## 3. Persisted-field inventory

Full machine-readable inventory lives in
`captures/persistence-contract.json` (`persistedFields`). Summary:

- Scalars: `schemaVersion`, `rng.state`, `rng.serial`, `tick`,
  `nextLegendOrdinal`, `house.id`, `house.name`, `house.campaignComplete`
  (`core.cpp:1228-1257`).
- House collections: routes (id/parentId/optional/children), unlockedRoutes,
  clearedRoutes, specializations, storedTrophies, storedItems, relicCandidates
  (item shape: id/name/attackBonus/ownerId/useCount/equipped/relicCandidate/
  history[]), lostTrophies, seasonalRewards, bounded legends (10 fields),
  campaignComplete (`core.cpp:1236-1257`, item serializer `1023-1040`).
- Scions & pools: current `scion`, all `fallenScions`, `pendingRelicItems`,
  `pendingRelicTrophies` — the last two absorb surfaced ground candidates at
  the snapshot boundary so nothing recoverable strands on a dead floor
  (`core.cpp:1265-1291`), mirroring `retire_instance()`
  (`core.cpp:563-605`).
- Deliberately absent: actors/monsters/events/windups/instance state/
  pending wave/seasonal mechanic pointer (`core.hpp:401-427`;
  `persistence/README.md:22-25`); **and the entire ProtocolSession profile**
  (§2.3) which no format currently represents.
- Wire-only "snapshots" that are NOT durable storage:
  `ProtocolSession::snapshot()` is JSON-over-websocket state projection
  (`networking.cpp:879`, `985`); the passive-tree JSON and chronicle JSON are
  in-memory documents (`networking.hpp:179,213`).

## 4. Save triggers, serialization seams, stale-data compatibility

Save triggers: **production none** (grep §6); tests call `snapshot()` directly
(`core_tests.cpp:755,778,821,847`) and exercise the adapter against a disposable
temp file twice (`883-891`). ADR-002 reserves "platform layer owns files; one
flat file per House" as future work (`docs/rebuild/ADR-002-persistence-seam.md:36-56`).

Three version seams exist with three different policies:

| Seam | Written | Validated on read | Tested |
|---|---|---|---|
| Core snapshot `schemaVersion=1` | `core.cpp:1228` | hard throw ≠1 (`1299-1301`) | **NO** |
| Chronicle document `version=3` | `networking.cpp:2318` | **never checked anywhere** | NO |
| Passive-tree `schemaVersion=2` | `networking.cpp:1219` | **never checked**; loader degrades missing keys to defaults (`1204-1214`) | NO |

Stale-data compatibility of the core format: forward-compatible additions
(unknown keys ignored — locked by `core_tests.cpp:769-775`); removals, renames,
type changes, truncation, and version drift all fail closed with key-named
errors (`core.cpp:1101-1147`) — implemented but **untested** outside the happy
path.

## 5. Reconnect vs. durability truth table

| Scenario | What survives | Where | Evidence |
|---|---|---|---|
| Socket drop, same process | whole session incl. active-instance choice rules | `sessions_` | `networking.cpp:2990` |
| Re-login, new socket, same process | account-ish memory profile; transient reset to town; full life; permadead stays dead | `reset_world_for_new_socket` | `networking.cpp:604-651`; `session_tests.cpp:1988-2013` (chronicle byte-identical after reconnect) |
| Same-socket hot re-login | even the active instance + ground items | session kept | `networking.cpp:604-610`; `networking_tests.cpp:268-283` |
| Server restart (clean or crash) | **nothing**; deterministic fresh profile per identity | — | `networking.cpp:2988`; `session_tests.cpp:437-443` asserts identity+scene only |
| Simulation-level crash mid-instance | carried value kept; surfaced recovery candidates requeue; floor value gone (D-106/D-109 boundary) | core snapshot semantics | `core.cpp:1265-1291`; `core_tests.cpp:778-878` |

## 6. Grep scope proof

`rg -n 'write_snapshot|write_atomic|read_snapshot|persistence::' native/src`
→ no matches; adapter include graph = `core_tests.cpp:9` +
`include/verdigris/persistence.hpp:6` only;
`rg -n "snapshot\(|restore\(" native/src native/include` → definitions plus
wire-JSON `ProtocolSession::snapshot` (`networking.cpp:879`) only. Full
literal transcripts in `REPORT.md`.

## 7. Red risks (ranked)

- **R1 (P0) No production save trigger.** Crash/restart silently resets every
  profile; D-109's "crash never loses progress" is unimplementable until a
  platform layer actually wires `write_snapshot`. Today the guarantee exists
  only inside a living process (`server_main.cpp:1-41`;
  `networking.cpp:2988,2990`).
- **R2 (P0) Session profile has no serialization seam.** Even perfect wiring
  of the core format would persist only the small Simulation slice; inventory,
  bank, quests, passive tree, chronicle, XP, lifecycle need their own versioned
  contract (`networking.hpp:161-247` vs `core.hpp:430-434`).
- **R3 (P1) Power-loss window in the adapter**: rename can outlive temp-data
  durability without fsync/FlushFileBuffers (`adapter.hpp:27-47`).
- **R4 (P1) Fixed `.tmp` name**: concurrent writers corrupt each other's temp
  (`adapter.hpp:25`) — becomes reachable the moment R1 lands.
- **R5 (P1) Inconsistent version policies** across the three seams (§4):
  one hard-fails untested, two silently accept anything.
- **R6 (P2)** `sessions_` never evicts identities → unbounded memory on
  long-lived servers (`networking.cpp:2988,2990`).
- **R7 (P2)** Restore mirrors only the first equipped item onto the live Actor
  (`core.cpp:1367-1372`); underlying Scion data intact, mirror lossy for
  multi-equipped saves.

## 8. Named negative control (required by SPEC)

**Stale-version case:** a realistic rollback/newer-build snapshot
(`schemaVersion != 1`) is rejected by implemented code
(`core.cpp:1299-1301`), yet **no current test locks that rejection** — a
refactor that quietly accepts foreign versions would pass the entire suite
(F-03 in the matrix). **Partial-write companions:** truncated-file rejection
(F-01) and concurrent-writer integrity through the shared `.tmp` path (F-10)
are likewise implemented-or-designed but entirely untested; F-10 is a genuine
torn-write vector once production saves exist.

## 9. Deterministic disposable-profile fault matrix

Contract: every row injects into synthetic snapshots produced by a seeded
`Simulation` and stored under a fresh temp directory; no real owner profile is
ever opened or mutated. Machine-readable form:
`captures/persistence-contract.json` → `faultMatrix.rows`.

| ID | Injection | Expected outcome | Coverage today | Smallest locking test |
|---|---|---|---|---|
| F-01 | Truncate bytes mid-collection (count=N, member k<N missing) | throw `missing snapshot field: <key>.<i>...` | NONE | cut buffer right after a `.count` line; expect throw |
| F-02 | Empty byte vector | throw missing `schemaVersion` | NONE | `restore({})` expects throw |
| F-03 | `schemaVersion=0` / `=2` rewrite | throw unsupported version; no partial state | **NONE — negative control** | rewrite version line both ways; expect throw |
| F-04 | Corrupt hex digit in a string value | throw `invalid snapshot string` | NONE | flip two hex chars; expect throw |
| F-05 | Negative number in unsigned field (`rng.state=-1`) | throw `invalid snapshot number` | NONE | rewrite line; expect throw |
| F-06 | Collection count > 1e6 | throw too-large before allocation | NONE | rewrite count; expect throw |
| F-07 | Duplicate of an EXISTING key appended | first wins; semantics unchanged | PARTIAL (only brand-new keys tested, `core_tests.cpp:769-775`) | append second `house.name`; expect baseline-equal restore |
| F-08 | CRLF line endings everywhere | parses identically to LF | NONE | swap `\n`→`\r\n`; expect equal restore |
| F-09 | Stale `.tmp` beside target before next write | write truncates and succeeds; read ignores tmp | NONE | pre-create junk tmp; write_atomic succeeds |
| F-10 | Two threads alternate `write_atomic` payloads on one target | target always byte-exact equals ONE payload; failures are exceptions, never torn content | **NONE — partial-write control** | 50 alternating writes; post-read must match writer A or B exactly |
| F-11 | Read nonexistent path | throw; creates nothing | NONE | expect throw on missing temp path |
| F-12 | ENV: power loss between temp data write and rename | DESIGN GAP: renamed target may be empty/partial (no fsync) | STRUCTURAL | successor adds FlushFileBuffers/fsync before rename + review lock |
| F-13 | Unknown extra field from newer build | ignored; canonical re-save identical | COVERED | already locked: `core_tests.cpp:769-775` |
| F-14 | Snapshot taken mid-instance | instance retired; carried kept; surfaced candidates requeued; deterministic RNG continuation | COVERED (D-106/D-109 locking) | already locked: `core_tests.cpp:778-878` |

## 10. Smallest locking tests for a successor

- **L1** locks F-03: expect throw restoring `schemaVersion∈{0,2}` rewrites of
  a valid snapshot (~10 lines in `core_tests.cpp` style).
- **L2** locks F-01/F-02/F-04/F-05/F-06: one table-driven malformed-bytes test
  asserting key-named throws.
- **L3** locks F-07/F-08: duplicate-key and CRLF tolerance pins.
- **L4** locks F-10 (+F-09): concurrency integrity over one disposable temp
  target.
- **L5** locks F-11: read-missing throws, creates nothing.
- **L6** (successor task, after R1/R2): server-restart durability journey —
  login → mutate → restart server seam → re-login same identity → assert
  profile equality. This test **cannot pass today**; writing it is the honest
  acceptance gate for wiring real saves.
- **L7** locks R3: code-review lock that the adapter fsyncs temp data before
  rename (portable unit-testing of power loss is out of scope).

## 11. Stop point

Per SPEC ("stop before modifying or opening non-disposable saves"), this audit
opened no user data and mutated nothing outside the owned task folder. Source
and test mapping is complete for the current tree; the successor seam work
(R1/R2) should begin from L1-L7 above and ADR-002's one-file-per-House model.
