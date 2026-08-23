# FINDINGS — TASK-0097 Native persistence durability and fault-model audit

- Lane: `ox-pc-bg` (model `openrouter/stealth/ox-alpha`)
- Branch: `worker/verdigris/pc/ox-pc-bg`
- Base commit: `d2423873c577d299b3b39c56024d1d840993c72b` (verified: `git rev-parse HEAD` at claim time)
- Frozen head SHA: recorded in `STATUS.md` at review request (this evidence commit)
- Machine-readable contract: [`captures/persistence-contract.json`](captures/persistence-contract.json)
- Constraint honored: **read-only audit** — no real owner profiles opened or modified; no ports outside the capsule; port 6500 never touched. All file-layer evidence comes from source reading plus the disposable temp-dir test (`verdigris-task-0030.snapshot`).

## 1. Executive result

The native core has a **proven, versioned, byte-stable snapshot seam** with a
**correctly atomic file adapter**, but **nothing in production calls either
one**. Durability today ends at process exit:

| Layer | State | Evidence |
|---|---|---|
| Core snapshot v1 (`snapshot`/`restore`) | implemented + locked by tests | native/include/verdigris/core.hpp:404-405; native/src/core.cpp:1158-1307; tests core_tests.cpp:729-852 |
| File adapter (`write_atomic`/`read`) | implemented + tested in temp dir only | native/persistence/adapter.hpp:23-73; sole production-tree includer is native/tests/core_tests.cpp:9 |
| Server wiring (save triggers, load on boot) | **absent** — `sessions_` map is memory-only | native/src/server_main.cpp (no persistence call); native/include/verdigris/networking.hpp:275 |
| Reconnect within one server lifetime | correct and D-109-compliant | native/src/networking.cpp:538-579, 2886-2890; session_tests.cpp:383-471 |

Consequence for the frozen invariants: **D-109 currently holds within a
server lifetime only.** A crash or restart loses every account/House/scion
progress even though the format to prevent it exists and round-trips.
D-106 holds unconditionally (it is enforced inside the simulation, before
any persistence question arises).

## 2. Every persisted field (core durable snapshot v1)

Format: canonical line-oriented text, fixed field order, strings lower-case
hex, numbers decimal, booleans `0`/`1`, collections carry `.count` +
numeric indexes (native/src/core.cpp:893-1009). Equivalent durable states
are byte-identical; unknown keys are ignored on restore and duplicate keys
resolve first-wins (core.cpp:1011-1025, locked by core_tests.cpp:745-749).

Full field-by-field mapping with write/read citations lives in
`captures/persistence-contract.json` → `persisted_fields_core_v1`. Summary:

- Scalars: `schemaVersion=1` (mandatory), `rng.state`, `rng.serial`, `tick`,
  `nextLegendOrdinal`, `house.id`, `house.name`, `house.campaignComplete`.
- Collections: `house.routes[]` (id/parentId/optional/children),
  `unlockedRoutes`, `clearedRoutes`, `specializations`, `storedTrophies`,
  `storedItems`, `relicCandidates` (D-106 pool), `lostTrophies` (D-106 pool),
  `seasonalRewards`, `legends[]` (bounded `kLegendCapacity=64`,
  founding-entry protected — core.cpp:282-288, core.hpp:193),
  `scion` (full record incl. carried items/trophies/deeds), `fallenScions[]`,
  `pendingRelicItems[]`, `pendingRelicTrophies[]`.
- Sub-record schemas: item (id/name/attackBonus/ownerId/useCount/equipped/
  relicCandidate/history), trophy (id/name), scion, legend — all cited per
  field in the JSON contract.
- Deliberately absent (D-109 boundary): live instance state
  (`instance_`, ground items/trophies) is retired at the snapshot boundary;
  surfaced relic/trophy candidates fold back into the pending pools exactly
  once (core.cpp:1197-1223 vs `retire_instance()` at 558-595). Events,
  actors, windups, seasonal mechanic pointers are transient; restore clears
  them and rebuilds the player actor from scion state (core.cpp:1284-1305).

RNG continuity is part of the contract: restored `rng.state`+`serial`
reproduce identical subsequent drops, so save/load cannot reroll loot
(core_tests.cpp:773-792; ADR-002 consequence).

### Persisted nowhere (the second durability class)

Everything on `ProtocolSession` is process-lifetime memory only:
chronicle document, inventory, wear seats, bank, House store, treasury,
quest chain, passive tree, `tree_quest_points_`, best depth, cleared road
nodes, combat XP, lifecycle state, kit flags — plus the global relic
circulation pool (function-local statics, networking.cpp:749-750).
Citation: networking.hpp:161-241. None of it survives restart, and none of
it is covered by the v1 snapshot.

## 3. Save triggers

- Production: **none**. `rg write_atomic` across `native/` hits only
  adapter.hpp itself and core_tests.cpp:859-863.
- Test-only trigger: temp-dir round trip (core_tests.cpp:854-866).
- Recommended successor wiring order (cheapest D-109 win first): after
  extraction commits stores (core.cpp:833-852); after death registration
  (764-831); after successor creation (871-883); after route unlock;
  checkpoint on login/session-replace/disconnect; periodic tick checkpoint.
  Full list in the JSON contract → `save_triggers.recommended_for_successor`.

## 4. Serialization / version seams

Three distinct seams (do not conflate them):

1. **Core durable snapshot v1** — mandatory `schemaVersion`; any value ≠ 1
   throws `unsupported or missing snapshot schemaVersion` (core.cpp:1230-1233).
   Fail-closed everywhere else too: missing required fields, malformed
   numbers/booleans/hex, and collection counts > 1,000,000 all throw
   (core.cpp:1033-1079, 911-928). No migration path defined — acceptable
   while no real files exist, must precede shipping saves.
2. **Passive tree wire payload** — carries its own `schemaVersion: 2`
   (networking.cpp:1147). Display/wire only; `player:skilltree:save` stores
   the client object verbatim while the point budget stays server-derived
   (earned − spent), so budget authority survives client tampering
   (networking.cpp:1120-1166).
3. **Chronicle document** — mints `{version:3,...}` (networking.cpp:2242-2251)
   but `player:chronicles:save` **replaces the whole chronicle with whatever
   the client sends, without validating version or shape**
   (networking.cpp:2574-2584). Red risk R3.

## 5. File location & atomicity

- Location: none chosen in production (deferred storage-service decision,
  OD-005). The adapter is path-agnostic; the only existing target is the
  disposable test file under `temp_directory_path()`
  (core_tests.cpp:857-858).
- Atomicity: temp file `<target>.tmp` written + flushed in the target's own
  directory (same volume ⇒ rename stays atomic), then
  `MoveFileExW(MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)` on Windows
  or `std::filesystem::rename` elsewhere; failure removes the temp file and
  throws (adapter.hpp:23-57).
- Honest gaps (all cited in the JSON contract → `file_layer.known_gaps`):
  - `ofstream::flush()` reaches the OS cache, not media: no fsync /
    FlushFileBuffers on the temp data before the rename;
    MOVEFILE_WRITE_THROUGH covers rename metadata only.
  - No directory fsync after POSIX rename (crash may lose the rename).
  - Fixed `.tmp` suffix: concurrent writers collide; orphaned `.tmp` files
    are never swept.
  - Windows replace fails if a reader holds the target open without
    FILE_SHARE_DELETE — no retry/backoff around that race.

## 6. Stale-data compatibility

- Forward compatible by design: older readers ignore appended unknown keys
  and keep restoring canonical fields (locked, core_tests.cpp:745-749).
- Version bump: hard fail-closed, no silent best-effort migration.
- Malformed data never silently changes state (throw list in §4).
- Exception: the two client-supplied seams. The passive tree tolerates
  garbage node ids leniently (try/catch skip, networking.cpp:1105) with a
  server-derived budget; the chronicle save accepts anything (§4/R3).

## 7. Reconnect semantics

- Identity: `guestId` (shared `default-guest` anonymous account unless
  overridden); identity string pins the FNV-1a seed of the whole simulation
  (networking.cpp:2886-2890, 511-514) — this makes disposable-profile fault
  injection deterministic.
- Same-process re-login / second socket: session adopted, socket replaced,
  world reset to town, transients cleared, **permadead preserved** so a
  mortal final death cannot be undone by reconnecting (networking.cpp:554-562);
  old socket gets `player:session-replaced` then a flushed close
  (networking.cpp:2890, 2697-2704). Locked by session_tests.cpp:434-471.
- Unexpected drop: client enters Retrying, refuses offline simulation
  (position frozen), resumes to Ready on an authoritative login snapshot
  (session_tests.cpp:383-432).
- Plain re-login on a live session keeps world/instance state
  (networking_tests.cpp:268-283).
- Process restart: everything above degrades to "fresh account" because
  `sessions_` is memory-only — see §1/R1.

## 8. Failure-mode catalogue

Eight documented failure modes with exact error strings, triggers, and
citations are tabulated in `captures/persistence-contract.json` →
`failure_modes` (save.open/save.write/save.rename/load.*/restore.version/
restore.field/restore.value/restore.count). Design shape: all failures are
loud exceptions; there is no code path that returns a default-looking
Simulation from bad bytes except tolerated unknown/duplicate keys.

## 9. Negative control (required by SPEC)

**Realistic case not covered by any current test:** external mangling of a
House file between sessions (cloud-sync conflict resolution, antivirus
quarantine/restore, or a manual copy interrupted mid-transfer). Two
sub-cases:

1. Truncation → restore *should* throw `missing snapshot field` (fail-closed)
   — behavior implemented via required-field reads but **asserted by zero
   tests**.
2. Same-length corruption (a flipped hex digit or digit swap inside a
   decimal) → decodes **silently into different values**: v1 carries no
   digest/checksum, so determinism-across-restarts can break invisibly.

Secondary stale-version case: `restore()`'s rejection of
`schemaVersion=2` (core.cpp:1230-1233) has no covering test; a future
migration refactor could regress it unnoticed.

Smallest locking tests for both cases are specified as L1–L6 in
`captures/persistence-contract.json` → `negative_control` (truncation
throws; wrong version throws; duplicate-key first-wins; corrupt-hex policy
decision; failed-rename leaves no `.tmp`; restart-contract documentation
test that inverts into the D-109-across-restarts proof once disk wiring
lands).

## 10. Deterministic disposable-profile fault matrix (for a successor)

Ten injectable faults F1–F10 with expected outcomes, current coverage, and
successor actions are specified in
`captures/persistence-contract.json` → `fault_matrix_disposable_profiles`.
Rules: every run uses a throwaway `playtestGuestId` prefixed
`fault-matrix-*` (identity pins the seed), temp-dir files only, owner
profiles and port 6500 untouched. Highlights:

- F2/F4/F5/F6/F7 name today's real gaps: orphaned `.tmp` sweep, undetected
  same-length corruption (needs a v2 digest trailer), untested version gate,
  `.tmp` collision under concurrent writers, read-vs-replace sharing
  violation.
- F8 is the P0: wire disk persistence and invert the L6 documentation test
  into the proof that restart preserves progress (D-109 end-to-end).
- F9/F10 are the green guards to keep: disconnect-mid-instance recovery and
  reconnect-after-permadeath.

## 11. Red risks (ranked)

- **R1 (P0)** Production native server never persists; D-109 holds within a
  server lifetime only (grep `write_atomic` → tests only; networking.hpp:275).
- **R2 (P1)** No integrity digest in v1 bytes → silent divergence on
  same-length corruption breaks determinism-across-restarts (ADR-002
  consequence).
- **R3 (P1)** `player:chronicles:save` trusts the entire client payload with
  no version/shape validation; revision++ masks stale overwrites
  (networking.cpp:2574-2584).
- **R4 (P2)** Fixed `.tmp` path collision; no startup sweep (adapter.hpp:25).
- **R5 (P2)** flush-not-fsync + missing POSIX dir fsync narrow power-loss
  durability (adapter.hpp:27-56).
- **R6 (P2)** Windows sharing-violation window; single failed save aborts
  the checkpoint with no retry (adapter.hpp:37-47).
- **R7 (P3)** Version-gate rejection implemented but untested (core.cpp:1230-1233).

## 12. Verification commands executed (literal)

See `STATUS.md` for the full literal transcript with exit codes. Summary:
preflight clean at base; `powershell ... native/build.ps1 -RunTests` → all
suites PASS (legacy denylist, core, networking, camera2d, session incl. the
reconnect/replaced suites); acceptance commands below recorded with exit 0.

## 13. Stop-condition check

No auth failures, base matches, no owned-path conflicts, all gates passed
honestly. Work confined to `orchestration/tasks/TASK-0097-persistence-durability-audit/**`.
