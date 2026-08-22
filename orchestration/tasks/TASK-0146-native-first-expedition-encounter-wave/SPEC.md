---
task: TASK-0146
title: Native first-expedition encounter wave
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
dependencies: [TASK-0143 ACCEPTED]
base_commit: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2
owned_paths: [native/include/verdigris/core.hpp, native/src/core.cpp, native/tests/core_tests.cpp, orchestration/tasks/TASK-0146-native-first-expedition-encounter-wave/**]
forbidden_paths: [native/client/**, native/src/networking.cpp, native/include/verdigris/networking.hpp, native/tests/session_tests.cpp, native/tests/networking_tests.cpp, server/**, src/**, playtest/**, .github/**, CI]
---

# Outcome

Make the default C++ first expedition read as an encounter rather than a
single-target wiring demonstration. Using only existing authoritative combat
vocabulary and constants, create one deterministic small Warden pack with a
legible normal/elite composition, spatial separation, and an actual combat
arc through the accepted `SlayWardens -> ExtractCarriedValue` objective.
Preserve fixed-step replay, extraction risk, death/recovery, item identity,
and current client commands. Do not invent production monster names, balance,
skills, magic, or loot tables.

The wave must be owner-visible through the existing client without client
changes: multiple threats appear, the elite's existing telegraph remains
meaningful, clearing the last living Warden advances the objective exactly
once, and the existing loot/extract loop still completes. Add focused core
tests for spawn determinism, pack-clear semantics, replay equality, and
death/recovery interaction.

# Acceptance

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native/build/verdigris_client.exe --scenario first-fight
native/build/verdigris_client.exe --scenario telegraph-dodge
native/build/verdigris_client.exe --scenario loot-to-bank
git diff --check
git diff --name-only 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2..HEAD
```

All commands exit 0. REPORT names the exact existing constants reused and the
deterministic assertions proving the encounter. No scenario may be weakened,
skipped, or made order-dependent.

# Stop conditions

Stop if the change requires client edits, new owner content, arbitrary balance
numbers, protocol changes, or weakening a current scenario. Prefer the
smallest coherent pack that visibly improves the first five minutes.
