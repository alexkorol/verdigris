---
task: TASK-0007
state: REVIEW_REQUESTED
branch: codex/TASK-0007-native-skill-actions
commits:
  - e7505ad
base_commit: 74e58a0
---

## Executive summary

The native core now exposes deterministic Thrust, Sweep, and WarCry actions
through the shared actor/action/damage pipeline, with resource costs and
regeneration, cooldown behavior, WarCry buff lifecycle events, and
actor-symmetric targeting. TASK-0009 remains the future client-binding task.

## Implementation

- Thrust uses the horizontal position sign as the existing deterministic
  facing proxy, reaches 1.5× melee range, deals 1.3× damage, costs 10 resource,
  and shares the attack cooldown.
- Sweep hits every living opposing actor in melee range for 0.75× damage,
  costs 15 resource, and uses a named 1.5× cooldown.
- WarCry costs 20 resource, applies +4 attack for 20 ticks, and emits
  `BuffApplied`/`BuffExpired`.
- Resource regenerates by a named +2 per tick up to the actor maximum.
- `spawn_monster` is a general public deterministic content seam used by
  existing enemy spawning and multi-target tests.

## Changed files

- `native/include/verdigris/core.hpp`
- `native/src/core.cpp`
- `native/tests/core_tests.cpp`

## Interfaces

Adds `ActionType::{Thrust,Sweep,WarCry}`, `EventType::{BuffApplied,BuffExpired}`,
actor WarCry state, and public `Simulation::spawn_monster`.

## Verification

```text
powershell -NoProfile -File native/build.ps1 -RunTests — PASS
git diff --check — PASS
```

Tests cover resource gating, Thrust range/damage/cooldown, Sweep multi-target
behavior, WarCry expiry/events, and replay determinism.

## Manual checks

Worker scope review confirms only the three task-owned native core/test paths
changed and the worktree is clean.

## Specification deviations

None. WarCry intentionally does not consume attack cooldown because the spec
assigns cooldown requirements to Thrust and Sweep only.

## Risks and limitations

The core still uses the deterministic horizontal position sign as its facing
proxy until a richer facing representation is product-approved.

## Questions for Fable or the owner

None.

## Integration notes

Independent validation and architect acceptance are required before integrating
`e7505ad`. TASK-0009 must remain sequential and DRAFT until this task lands.
