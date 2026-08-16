---
task: TASK-0011
state: REVIEW_REQUESTED
branch: codex/TASK-0011-monster-skill-ai
commits:
  - 8c68aed
base_commit: e6d3f48
---

## Executive summary

Elite monsters now use the shared Thrust/Sweep action pipeline with a
deterministic three-tick telegraph. Pending actions carry the selected skill,
emit `AttackTelegraphed`, recheck gates at resolution, and cancel safely when
the monster or player dies. Non-elite monsters retain the existing plain
melee cadence.

## Implementation

- Appended `EventType::AttackTelegraphed` and exposed
  `kTelegraphTicks = 3`.
- Added bounded pending action state to `Actor`.
- Elite AI selects an outer-range Thrust when the player is in the facing cone,
  close-range Sweep when affordable, and otherwise plain melee.
- Resolution decrements windup deterministically and calls
  `resolve_actor_action`; resource/cooldown/range gates are rechecked.
- Death cancellation and stop-after-player-death behavior prevent stale
  pending actions or extra enemy processing.

## Changed files

- `native/include/verdigris/core.hpp`
- `native/src/core.cpp`
- `native/tests/core_tests.cpp`

## Interfaces

- New appended `EventType::AttackTelegraphed` event.
- New public `kTelegraphTicks` constant.
- New `Actor::pending_action` and `pending_action_ticks` state.

## Verification

- `powershell -File native/build.ps1 -RunTests` — PASS.
- `git diff e6d3f48..8c68aed --check` — PASS.
- Legacy denylist self-test passed as part of the native gate.
- Independent validator `/root/validate_task_0011` — ACCEPT; exact timing,
  gate/fizzle, cancellation, non-elite cadence, and replay checks passed.

Named tests:

- `test_elite_thrust_telegraph_timing`
- `test_elite_skill_cone_gating`
- `test_elite_skill_fizzles_when_resolution_gates_fail`
- `test_elite_sweep_uses_shared_pipeline`
- `test_elite_telegraph_cancels_on_death`
- `test_elite_skill_replay_is_deterministic`
- `test_non_elite_melee_cadence_is_unchanged`

## Manual checks

The worker reran the native test target and confirmed a clean worktree; no
client changes were made.

## Specification deviations

The elite selection uses a deterministic thrust-only outer band beyond the
plain melee range. This keeps close-range Sweep reachable even though monsters
face their pursuit target every turn; it is called out for architect review.

## Risks and limitations

The exact boundary between the thrust band and close-range Sweep is a tuning
choice within the spec's range/cone rule and should be reviewed alongside
telegraph readability. No new RNG was introduced.

## Questions for Fable or the owner

Please confirm whether the outer-band interpretation is the desired elite
skill selection behavior; no implementation blocker remains.

## Integration notes

TASK-0010 is integrated at `afcd1d3`; this task touches only native core and
tests and is parallel-safe with TASK-0012.
