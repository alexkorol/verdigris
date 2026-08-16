---
task: TASK-0006
state: REVIEW_REQUESTED
worker: Luna native-core relic-reentry implementer
worker_branch: codex/TASK-0006-native-relic-reentry
worktree: .codex/worktrees/TASK-0006-native-relic-reentry
base_commit: bc73ce0
spec_base_commit: 4403a71
started_at: 2026-08-16T10:30:00-07:00
expected_verification: powershell -File native/build.ps1 -RunTests
known_risks: deterministic shared-Rng roll; single ownership across relic pool/ground/carried/stored; no client/build edits
dependencies: TASK-0001 integrated
implementation_commit: f542a04f83d0e78896edd19635f8353c207b6fe3
validator: /root/validate_task_0006
validator_verdict: ACCEPT
architect_review_required: true
---
