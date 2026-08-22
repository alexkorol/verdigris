---
task: TASK-0148
title: Native Chronicles reconnect runtime
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
dependencies: [TASK-0081 ACCEPTED, TASK-0086 ACCEPTED]
base_commit: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2
owned_paths: [native/include/verdigris/networking.hpp, native/src/networking.cpp, native/src/server_main.cpp, native/tests/session_tests.cpp, native/tests/networking_tests.cpp, orchestration/tasks/TASK-0148-native-chronicles-reconnect-runtime/**]
forbidden_paths: [native/client/**, native/include/verdigris/core.hpp, native/src/core.cpp, native/tests/core_tests.cpp, server/**, src/**, playtest/**, .github/**, CI]
---

# Outcome

Close the smallest real runtime gap in the networked Gate-B owner journey:
`found House -> create/set-out Scion -> die -> successor -> recover relic ->
disconnect -> reconnect -> same House state`. Use the accepted N5/N6 envelope
contracts and existing in-memory server ownership; do not invent a new
protocol, database, lore, or economy. First prove which step is missing in the
current native server/session tests, then implement that behavior so a normal
client can complete and resume the journey without dev-only mutation events.

If the entire journey already works, do not submit a test-only packet: close
the first concrete player-visible runtime gap exposed by executing it, such as
missing reconnect continuity, an un-emitted accepted state transition, or a
normal-input path that currently requires a dev event. Preserve guest
isolation, session replacement, deterministic state, and port ownership.

# Acceptance

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
git diff --check
git diff --name-only 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2..HEAD
```

All commands exit 0. A new literal session scenario must drive the complete
Gate-B sequence over loopback using only normal accepted envelopes, reconnect
with the same guest identity, and assert the same House/Scion/relic state.
REPORT names the pre-change failing step and post-change owner-visible effect.

# Stop conditions

Stop before client edits, durable database/filesystem design, browser/server
source edits, protocol invention, port 6500, balance/content changes, or test-
only work that leaves the runtime gap unchanged.
