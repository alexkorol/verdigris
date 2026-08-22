---
task: TASK-0149
title: Native owner-launch resilience
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
dependencies: []
base_commit: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2
owned_paths: [native/tools/play-native.ps1, orchestration/tasks/TASK-0149-native-owner-launch-resilience/**]
forbidden_paths: [native/client/**, native/src/**, native/include/**, native/tests/**, native/build.ps1, native/CMakeLists.txt, server/**, src/**, CI]
---

# Outcome

Make the documented one-command Windows owner launch fail-fast, self-explain,
and leave no orphan server for the real remote client path. Preserve the
6520-6539 capsule and default remote mode. Add deterministic script-level
preflight and lifecycle checks without changing the game or requiring admin.

# Acceptance

`play-native.ps1 -Rebuild` launches the real client/server, reports its chosen
port and log, and verifies cleanup after normal close and forced client exit;
`git diff --check` passes. Do not substitute a headless demo for the window.
