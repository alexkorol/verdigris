# TASK-0015 report — core presentation catalog

## Executive summary

Added a curated, read-only `PresentationCatalog` returned by
`Simulation::presentation_catalog()`. The catalog exposes only the gameplay
constants needed by presentation: Thrust/Sweep/War Cry costs, melee and Thrust
ranges, telegraph windup, War Cry bonus/duration, and resource regeneration.
Mechanics and the catalog now consume one shared named definition for each
field; gameplay values were not changed.

## Changed files

- `native/include/verdigris/core.hpp` — shared presentation constants,
  `PresentationCatalog` value type, equality, and accessor declaration.
- `native/src/core.cpp` — mechanics use the shared constants; accessor and
  value equality implementation.
- `native/tests/core_tests.cpp` — stability and behavior-correlation tests for
  skill costs, telegraph ticks, War Cry state, and catalog reads.

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests` — PASS; denylist and
  `verdigris core tests: PASS`.
- `git diff --check` — PASS.
- Defining-constant audit — PASS: each catalog source constant has exactly one
  `inline constexpr int` definition in `native/include/verdigris/core.hpp`:
  `kMeleeRange`, `kThrustRange`, `kThrustResourceCost`,
  `kSweepResourceCost`, `kWarCryResourceCost`, `kResourceRegenPerTick`,
  `kWarCryAttackBonus`, and `kWarCryDurationTicks`.

## Public interface

`Simulation::presentation_catalog()` is static and returns a value, not mutable
simulation state. `PresentationCatalog::operator==` supports deterministic
stability checks. No serialization, localization, client adoption, or state
snapshot was added.

## Deviations and risks

None. The first test draft exposed a test-only stale vector pointer after actor
insertion; it was corrected before the passing run. No gameplay behavior or
recorded command values changed.
