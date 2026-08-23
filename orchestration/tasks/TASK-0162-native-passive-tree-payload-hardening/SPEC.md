---
task: TASK-0162
title: Native passive-tree payload hardening
state: SUPERSEDED
superseded_by: integrated (reviewed head 98068dfc, 2026-08-23)
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
base_commit: dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17
owner_visible_contribution: prevents malformed progression payloads from becoming negative, absurd, or falsely authoritative gear-pane state
dependencies: [TASK-0156 ACCEPTED]
owner_input_dependency: none; validation only, with no tree design or balance authority
owned_paths: [native/client/remote_session.cpp, native/tests/session_tests.cpp, orchestration/tasks/TASK-0162-native-passive-tree-payload-hardening/**]
forbidden_paths: [native/client/main.cpp, native/client/presentation_state.cpp, native/client/client_model.hpp, native/src/**, native/include/**, server/**, src/**, protocol-schema changes, balance, authored tree content, everything else]
---

# Outcome

Fail closed when the existing native client receives a malformed or incomplete
`passiveTree` envelope. The mirror added by TASK-0156 may become present only
when schema version, points, earned count, nodes, and conduits have the expected
types and sane nonnegative integral values. Invalid updates must preserve the
last valid authoritative snapshot and surface a deterministic parser/session
diagnostic; they may not silently become zero or absurd UI counts.

# Acceptance

Add focused session tests that drive the production envelope parser with a
valid login/update followed by missing fields, wrong types, fractional,
negative, non-finite/overflow-like, and oversized-array payloads. Prove valid
absent/zero/nonzero behavior remains unchanged, invalid data never mutates the
last valid snapshot, diagnostics are stable, and no server/wire authority is
changed.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests
native/build/verdigris_session_tests.exe
git diff --check
git diff --name-only
```

# Negative controls and STOP conditions

No invented passive-tree limits that encode balance. Structural safety caps may
only prevent integer/memory abuse and must be documented as transport bounds,
not product rules. No server, protocol, UI, save, gameplay, or content change.
