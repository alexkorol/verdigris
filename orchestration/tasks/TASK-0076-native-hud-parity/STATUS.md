---
task: TASK-0076
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0076-native-hud-parity-cursor
base_commit: 69d60133b8fe67087a856d837f6d714fc9ddb4c6
started_at: 2026-08-20T07:48:00-07:00
finished_at: 2026-08-20T08:00:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests -RunClientScenarios; Orb/Quickbar/Minimap asserts; captures 1920+1366
notes: Orbs, quickbar, minimap; HUD safe zones updated. Pipelined over 0075.
---

HUD parity REVIEW_REQUESTED. Gates green; Orb/Quickbar/Minimap ops asserted.
See REPORT.md and captures/.
