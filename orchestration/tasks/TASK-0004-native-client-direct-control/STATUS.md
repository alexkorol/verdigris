---
task: TASK-0004
state: ACCEPTED
worker: Luna native-client implementer
worker_branch: codex/TASK-0004-native-client-direct-control
worktree: .codex/worktrees/TASK-0004-native-client-direct-control
base_commit: bc73ce0
spec_base_commit: 4403a71
started_at: 2026-08-16T10:30:00-07:00
expected_verification: powershell -File native/build.ps1 -RunTests -RunClient; driven-input pass with logged assertions/captures
known_risks: preserve headless byte-compatible output; client-only ownership; TASK-0002 define guard remains in revision
dependencies: TASK-0002 review/revision in flight; no build-file edits permitted
implementation_commit: 7ac51d4
validator: /root/validate_task_0004
validator_verdict: ACCEPT
architect_review_required: true
architect_verdict: ACCEPTED
architect_reviewed_commits: 7ac51d4
integration_state: QUEUED
---
