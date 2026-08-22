---
task: TASK-0163
title: Gate-B ordinary-play journey reliability
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 75ef6b7b
owner_visible_contribution: restores trustworthy release gating for the complete ordinary-play House, fall, succession, heirloom, and reconnect journey
dependencies: [TASK-0148 ACCEPTED, TASK-0155 ACCEPTED]
owner_input_dependency: none
owned_paths: [native/tests/session_tests.cpp, orchestration/tasks/TASK-0163-gate-b-ordinary-play-reliability/**]
forbidden_paths: [native/client/**, native/src/**, native/include/**, native/audio/**, server/**, src/**, gameplay rules, wire/runtime behavior, timeout-only weakening, dev or direct-state shortcuts, everything else]
resource_capsule: loopback ports 7160-7179; never touch port 6500
---

# Outcome

Make the existing Gate-B ordinary-play journey deterministic enough to serve as
a release gate on the current combined program. Preserve the exact real-player
chain: House foundation, first Scion set-out, ordinary combat fall, successor,
normal hunt, exact heirloom recovery, and same-guest reconnect. Diagnose and
remove the observed harness nondeterminism that produced both (a) a seven-minute
four-kill sweep with no named Warden and (b) a retry that failed to observe the
fatal fall. This packet changes the test driver only; it does not change game
rules or make the proof easier.

# Required proof

- Record a compact causal diagnosis for both observed failure surfaces.
- Keep every action on ordinary public client/wire surfaces already used by the
  journey; retain the named-Warden, exact-UUID, mortality, and reconnect checks.
- Add focused deterministic controls for the corrected driver state machine.
- From a clean build, pass the full native gate, then pass the exact session
  executable three consecutive times with no source or fixture changes between
  runs.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests
1..3 | ForEach-Object { native/build/verdigris_session_tests.exe; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE } }
git diff --check
git diff --name-only
```

# Negative controls and STOP conditions

No `dev:*`, direct simulation/state mutation, teleport, seeded reward injection,
hard-coded Warden coordinates, assertion deletion, timeout inflation, runtime
change, or gameplay/balance change. STOP and report a runtime defect if the
journey cannot be made reliable inside `native/tests/session_tests.cpp` alone.
