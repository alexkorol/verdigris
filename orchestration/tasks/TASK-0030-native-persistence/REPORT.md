---
task: TASK-0030
state: REVIEW_REQUESTED
branch: codex/TASK-0030-native-persistence
commits:
  - 43b796e6
  - 94719ae6
base_commit: 8e4a977c
---

## Executive summary

Native persistence now exposes pure deterministic snapshot/restore bytes for
durable House, Scion, relic/recovery, Legends, campaign, and RNG state. The
format is versioned and canonical; restore tolerates unknown keys while
rejecting malformed known values. Live instance/monster/floor state is not
serialized, and the D-109 boundary preserves carried value while returning
the Scion to the House.

## Implementation

- Added `verdigris::snapshot(const Simulation&)` and
  `verdigris::restore(const std::vector<uint8_t>&)`.
- Implemented canonical line-oriented UTF-8/hex text with mandatory
  `schemaVersion=1`, fixed field order, stable collection indexes, and RNG
  `state` plus `serial`.
- Added the thin `native/persistence/` file adapter with flush plus atomic
  temp-file replacement (`MoveFileEx` on Windows, `rename` elsewhere).
- Documented the format, durable field coverage, and D-109 semantics in
  `native/persistence/README.md`.

## Changed files

- `native/src/core.cpp`
- `native/include/verdigris/core.hpp`
- `native/include/verdigris/persistence.hpp`
- `native/persistence/adapter.hpp`
- `native/persistence/README.md`
- `native/tests/core_tests.cpp`

## Interfaces

- Pure core byte boundary: `snapshot` / `restore`.
- Platform file boundary: `persistence::write_atomic`, `read`, and aliases
  `write_snapshot` / `read_snapshot`.

## Verification

- `powershell -File native/build.ps1 -RunTests` — passed (denylist and core
  tests).
- Added tests cover byte-stable round trip, mandatory version field,
  unknown-field tolerance, D-109 mid-instance carried item/trophy retention
  with floor retirement, deterministic RNG continuation through an enemy kill
  and generated item/trophy ID comparison, explicit relic/lost-trophy pools,
  surfaced relic/trophy pending re-entry without duplicate ownership, and
  atomic adapter replacement/readback.
- Independent validator `/root/validate_task_0030_persistence`: ACCEPT.
- `git diff --check` and owned-path review: pass.

## Manual checks

- Reviewed the emitted format and adapter implementation against the
  fixed-step/headless boundary; no simulation I/O is introduced.

## Specification deviations

None reported by the worker. The implementation intentionally omits active
instance/monster/windup state as required by ADR-002/D-109.

## Risks and limitations

- The v1 format is deliberately internal and un-obfuscated; migrations and
  seasonal transforms remain out of scope.
- Adapter atomicity depends on the host filesystem's rename guarantees.

## Questions for Fable or the owner

None.

## Integration notes

Native-only paths are disjoint from the browser audit task. Architect review
is required before integration.
