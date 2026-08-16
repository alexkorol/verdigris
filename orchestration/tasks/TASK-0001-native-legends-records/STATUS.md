---
task: TASK-0001
state: CLAIMED
worker: Luna native-core implementer
worker_branch: codex/TASK-0001-native-legends-records
worktree: .codex/worktrees/TASK-0001-native-legends-records
base_commit: 0e02aa7
spec_base_commit: f5b4b72
started_at: 2026-08-15T23:41:33-07:00
expected_verification: powershell -File native/build.ps1 -RunTests
known_risks: bounded eviction ordering; no seasonal policy invention
---
