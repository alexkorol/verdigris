---
task: TASK-0067
title: Native journey CI (build + tests + scenarios + remote journey in Actions)
state: READY
packet: BOUNDED-DESIGN
lane: any (CI yaml + scripting; mac-claude or cursor)
priority: high (D-122 phase 3 — Gate A regression guard)
owned_paths:
  - .github/workflows/** (the native workflow)
  - native/tools/** (CI helper scripts only)
  - orchestration/tasks/TASK-0067-native-journey-ci/**
forbidden_paths:
  - native/src/**, native/client/** (file notes for gaps)
  - playtest/** assertions
---

# Outcome

The native GitHub Actions workflow proves what the owner launches:
configure/build → denylist → core/networking/camera/session tests →
local client scenarios → the remote journey session tests (they start
their own server on ephemeral loopback) → clean shutdown check. On
failure upload artifacts: build log, test transcripts. Windows runner
(MSVC). Keep total runtime sane (under ~15 min) — no density bench in
CI.

# Acceptance

Green Actions run on the worker branch (link the run in the report) +
a deliberately broken canary commit (reverted before review) proving
the journey step actually fails CI — include both run links. Architect
re-triggers the workflow once personally.
