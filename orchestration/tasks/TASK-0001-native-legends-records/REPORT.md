---
task: TASK-0001
state: INTEGRATED
branch: codex/TASK-0001-native-legends-records
commits:
  - 7ed844d8d5a9a6fb2f5a2d2ee9428c10b1cf7fad
base_commit: 0e02aa7
spec_base_commit: f5b4b72
---

## Executive summary

The native House now records a bounded deterministic Legends history. It
records Scion creation, route/branch unlocks, route clears, elite kills,
Scion deaths, relic transitions, and campaign completion, emits a
`LegendRecorded` event, and exposes the record through `Simulation::legends()`.

## Implementation

- Added a `LegendEntry` value type and `House` record with a named 64-entry cap.
- Eviction removes the oldest non-founding entry first, preserving founding
  milestones under pressure.
- Ordinals, stable IDs, and event ordering are deterministic under identical
  seed + command streams.
- Route-two enemies carry an elite flag so elite-kill records are observable
  without inventing a separate stat universe.

## Changed files

- `native/include/verdigris/core.hpp`
- `native/src/core.cpp`
- `native/tests/core_tests.cpp`

## Interfaces

- `LegendEntry` value type and `House::legend_entries` storage.
- `EventType::LegendRecorded`.
- `Simulation::legends()` read accessor.

## Verification

Command executed in the worker worktree:

```powershell
powershell -File native/build.ps1 -RunTests
```

Result: PASS. The run also passed the native legacy denylist. Added tests:

- `test_legends_cover_unlocks_and_campaign_milestone`
- `test_elite_kill_and_recorded_event`
- `test_legends_are_bounded_and_evict_oldest_non_founding`
- `test_legend_stable_ids_and_deterministic_replay`
- extended determinism and succession/death coverage

## Manual checks

- Only the three owned native source/test paths changed.
- Stable IDs and monotonic ordinals remain deterministic under eviction.
- The founding milestone survives cap pressure.

## Specification deviations

None reported. The worker did not edit orchestration files.

## Risks and limitations

The `relic_extracted` hook is implemented for relic-candidate items entering
extraction, but the current baseline creates relic candidates on death and
stores them in the House relic pool, so that extraction path is dormant until
future relic re-entry mechanics exist.

## Questions and follow-up

No owner decision is required for this bounded mechanical slice. Future relic
re-entry should exercise the dormant extraction hook without changing the
record cap or seasonal policy.

## Independent validation

Validator verdict: **ACCEPT**. The validator reran
`powershell -File native/build.ps1 -RunTests`, confirmed denylist and core test
success, inspected the full commit scope, and found no determinism, bound,
event-ordering, succession, or forbidden-path findings.

## Integration notes

Architect review was **ACCEPTED**. The worker commit was integrated locally as
`5487778`; the native acceptance gate was rerun with the client shell.
