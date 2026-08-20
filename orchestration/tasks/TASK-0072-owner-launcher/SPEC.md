---
task: TASK-0072
title: One-command owner launcher (play-native)
state: READY
packet: MECHANICAL
lane: any (PowerShell scripting)
priority: medium (D-117 owner-visible convenience)
owned_paths:
  - native/tools/play-native.ps1 (new)
  - native/README.md (launch section only)
  - orchestration/tasks/TASK-0072-owner-launcher/**
forbidden_paths:
  - native/src/**, native/client/**
  - orchestration/tasks/TASK-0061-networked-guest-slice/** (history)
---

# Outcome

`powershell -File native/tools/play-native.ps1` gives the owner one
command that: builds if exes are missing or stale (delegating to
build.ps1), starts verdigris_server on a free port from 6520-6539
(owner-play range — NOT coordinator capsules, never 6500), starts the
client in --remote against it, tees server output to a log file in
native/build/logs/, and on client exit shuts the server down cleanly
and prints where the log went. Flags: -Local (local sim client
instead), -Port NNNN (explicit), -Rebuild (force). Document in
native/README.md with a Windows desktop-shortcut one-liner.

# Acceptance

Fresh-clone simulation: delete native/build, run the script, reach the
playable remote window, Esc quits, no orphan processes (script proves
it: post-exit process check printed). Architect reruns exactly that.
