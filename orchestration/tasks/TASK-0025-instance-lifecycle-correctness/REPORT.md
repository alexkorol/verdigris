---
task: TASK-0025
state: REVIEW_REQUESTED
branch: codex/TASK-0025-instance-lifecycle-correctness
commits:
  - 63df51f
base_commit: aa52054
---

# TASK-0025 report — instance lifecycle boundaries and pack clear

## Executive summary

Native pickup now respects the active instance boundary and its authoritative
ground-ID lists. Retiring an instance removes unextracted floor leftovers,
while surfaced recovery candidates return to their House recovery pools once.
Route and campaign clearing now waits for the final living monster in a pack.

## Implementation

- Added `retire_instance()` and call sites for extraction, Scion death, and
  route entry. It clears ordinary floor items/trophies and the instance ID
  lists, then marks the instance inactive.
- `resolve_pickup` requires an active Scion instance and membership in the
  active instance's item/trophy ID list; stale vector references cannot bypass
  the boundary.
- A surfaced relic item is returned to `house_.relic_candidates` on retirement
  if still on the floor. A surfaced recovery trophy is returned to
  `house_.lost_trophies` once. Ordinary floor drops are lost on retirement.
  Carried items/trophies continue through the existing death registration path.
- `clear_route_and_unlock_children()` now runs only when no living monster
  remains; per-kill `drop_reward()` behavior is unchanged.

## Changed files

- `native/include/verdigris/core.hpp`
- `native/src/core.cpp`
- `native/tests/core_tests.cpp`

## Tests and verification

Added coverage for post-extraction and cross-route stale pickups, same-route
re-entry cleanup, death/relic/trophy retirement without double registration,
two-monster delayed route/campaign clear, and deterministic replay. Existing
relic, death, extraction, and pack behavior remains covered.

- `powershell -NoProfile -File native/build.ps1 -RunTests` — PASS
- `git diff --check aa52054..63df51f` — PASS

## Specification deviations

None. No client, build, tooling, loot-decay, or multi-instance changes.

## Risks and limitations

Floor leftovers are intentionally lost when an instance is retired; only
surfaced recovery candidates are returned to their recovery pools. This keeps
the extraction/death distinction explicit and avoids double registration.

## Integration notes

Independent validator `/root/validate_task_0025` accepted the implementation,
scope, retirement semantics, and native gate. Submitted for architect review;
integration is held until that review lands.
