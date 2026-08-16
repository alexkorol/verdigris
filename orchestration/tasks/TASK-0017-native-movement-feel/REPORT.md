---
task: TASK-0017
state: IMPLEMENTED
branch: codex/TASK-0017-native-movement-feel
commits:
  - 5b73a24
base_commit: cb2718c
---

# TASK-0017 report — continuous native movement and D-107 camera defaults

## Executive summary

Native movement now advances in deterministic fixed-step increments derived
from the actor's per-second speed. The player runs at 220 world units/sec,
which produces 11 units per 50 ms tick (about 2.2 tiles/sec), and dash is a
named ten-tick directional burst. The client defaults to the owner-ratified
D-107 ARPG camera and blends perspective toward the Miniature treatment only
at close zoom.

## Implementation

- Added the named `kSimulationTickMs`, `movement_step_per_tick`, and
  `kDashMovementTicks` constants in the native public core header.
- Converted player movement from full-tile command displacement to integer
  per-tick movement; preserved the `MoveIntent` command shape and deterministic
  actor math. Enemy pursuit cadence remains unchanged.
- Reduced the enemy spawn distance from 2000 to 1500 world units so the slower
  player can reach the existing encounter without changing melee, thrust, or
  extraction ranges. This is the only reachability constant changed.
- Set native camera defaults to zoom 0.85, pitch 62°, perspective 0.0006,
  anchor 0.52, and fog 0.4. Projection and mouse unprojection share the anchor.
- Added named wheel-zoom blending toward perspective 0.0013 after zoom 1.05,
  reaching the Miniature endpoint by zoom 1.35.
- Updated headless movement loops and tests to exercise the slower cadence.

## Changed files

- `native/include/verdigris/core.hpp`
- `native/src/core.cpp`
- `native/client/main.cpp`
- `native/tests/core_tests.cpp`

## Interfaces and tests

- `movement_step_per_tick(int)` is a deterministic header-level derivation
  shared by core tests and simulation code.
- Added coverage for cardinal and diagonal derivation, deterministic replay,
  named dash distance/event, and movement reachability.

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests -RunClient` — PASS
  (legacy denylist, core tests, headless client, and Win32 client build).
- `git diff --check cb2718c..5b73a24` — PASS.

## Manual driven-input evidence

The worker's driven-input run logged cardinal positions at 0, 1, 2, and 3
seconds as 0, 220, 440, and 660 world units: 2.2 tiles/sec, with each 50 ms
sample advancing 11 units. A dash advanced 110 units along the facing vector
and emitted the dedicated `ActorMoved` dash event.

## Specification deviations

None. No combat, melee, thrust, or extraction ranges were changed.

## Risks and limitations

- Camera fog is currently a named client presentation parameter surfaced in
  the debug line; the native renderer remains a Win32 presentation shell.
- The worker reports no monster pursuit locomotion redesign; the task keeps
  existing enemy cadence and uses the shared movement derivation for player
  movement and dash.

## Questions for Fable or the owner

None.

## Integration notes

Awaiting independent validator review and architect acceptance before
integration. The base includes accepted browser Phase-2 work but this diff is
native-path-only.
