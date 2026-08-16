---
task: TASK-0017
state: CLAIMED
coordinator: kimi
worker: kimi-code-cli
worker_branch: codex/TASK-0017-native-movement-feel
worktree: C:\Users\Alex\Documents\Kimi\verdigris
base_commit: 1ae1b01
spec_base_commit: 4f24b94
started_at: 2026-08-16T14:45:00-07:00
expected_verification: powershell -NoProfile -File native/build.ps1 -RunTests -RunClient
known_risks: movement step change may cascade into range/reachability constants (stop condition); recorded command streams break via constant change only
dependencies: TASK-0015 and TASK-0016 integrated
architect_review_required: true
---
