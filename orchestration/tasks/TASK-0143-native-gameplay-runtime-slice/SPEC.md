---
task: TASK-0143
title: Native first-expedition gameplay runtime slice
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
dependencies: []
base_commit: d0f74af3d30f238479218f8be412a01d61e21df3
owned_paths: [native/src/**, native/include/**, native/tests/**, orchestration/tasks/TASK-0143-native-gameplay-runtime-slice/**]
forbidden_paths: [native/client/**, native/CMakeLists.txt, native/build.ps1, server/**, src/**, playtest/**, .github/**, CI or machine mutation]
promotion_provenance:
  generator: codex-pc-architect
  parent_packet: PROGRAM_GRAPH T2 owner-playable journey
  dependency_event: native core build is green; current first-fight path must be made explicit and deterministic
  validator: core-only single-writer packet; collision clear at d0f74af3
---

# Outcome

Advance the actual C++ first-expedition loop, not its paperwork. Inspect the
current deterministic core and close one concrete gameplay gap in the path
`House → Scion → enter route → defeat enemy → collect/equip item → extract`.
Prefer a missing authoritative objective/state transition, reward or extraction
invariant found by the existing tests; if the path is already mechanically
complete, add the smallest owner-visible deterministic objective/telemetry
state and regression tests that make the loop explicit rather than inventing
new balance or content. Preserve server-authoritative, fixed-step behavior,
House/Scion recovery semantics, and deterministic replay.

Add focused C++ tests for the changed invariant and ensure existing combat,
death/recovery, extraction, and serialization tests remain green. Do not touch
the client presentation; TASK-0142 owns that surface.

# Acceptance commands

From repository root, record literal output and exit codes in REPORT:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests
git diff --check
git diff --name-only d0f74af3d30f238479218f8be412a01d61e21df3..HEAD
```

The native core tests must exit 0, and the worker must name the concrete gap
closed plus the deterministic assertion proving it. No new dependency, port,
renderer, browser, server, or owner-policy action is acceptable.

# Stop conditions

STOP before editing client files, changing balance/content tables without an
existing authority packet, mutating persistence outside the simulation test
fixtures, or claiming the owner-playable loop without the native test gate.
