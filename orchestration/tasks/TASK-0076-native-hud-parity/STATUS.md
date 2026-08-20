---
task: TASK-0076
state: CLAIMED
coordinator: cursor
worker_branch: codex/TASK-0076-native-hud-parity-cursor
base_commit: 69d60133b8fe67087a856d837f6d714fc9ddb4c6
started_at: 2026-08-20T07:48:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests -RunClientScenarios; Orb/Quickbar/Minimap render-list asserts; captures 1920+1366
notes: Pipelined over 0075 terrain tiles branch. Ports 6580-6599.
---

Claimed after TASK-0075 REVIEW_REQUESTED. HUD orbs, quickbar, minimap.
