---
task: TASK-0083
title: Native WebSocket server lifecycle soak
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
priority: high (INC-012 handoff regression; post-PR46 thread join)
lane: cursor composer-2.5 calibration or deepseek
base_commit: 1f82623d9a3936513327cc43362703443e14b02a
owned_paths:
  - native/tools/server_lifecycle_soak.cpp
  - native/build.ps1
  - orchestration/tasks/TASK-0083-server-lifecycle-soak/**
forbidden_paths:
  - native/client/**
  - native/src/**
  - native/include/**
  - native/tests/**
  - playtest/**
  - server/**
---

# Outcome

Add an opt-in, machine-verifiable soak around the existing public networking
API. `-RunServerLifecycleSoak` builds a tool that performs 100 sequential
start/connect/login/close/stop cycles on loopback ephemeral ports, then an
eight-client connect/login/close burst before final stop. It writes JSON with
cycle counts, successful upgrades/logins, clean closes, stop durations, total
duration, and failure details. The process exits non-zero on any failed cycle,
hang, crash, or timeout.

This is a regression guard for the reader-thread lifetime bug fixed in PR #46;
it must exercise the real `WebSocketServer`, not a fake transport. No server or
client behavior changes are authorized.

# Acceptance commands

Paste literal transcripts and exit codes in `REPORT.md`:

```powershell
powershell -File native/build.ps1 -RunTests -RunClientScenarios
powershell -File native/build.ps1 -RunServerLifecycleSoak
powershell -File native/build.ps1 -RunServerLifecycleSoak
git diff --check
```

Both soak invocations must complete independently and report 100/100 plus the
eight-client burst. Preserve their JSON under this task's `captures/` folder.

# Stop conditions

STOP if a source fix is needed outside the owned paths. Record the first
failing cycle and exact transcript; do not patch networking behavior inside
this packet.
