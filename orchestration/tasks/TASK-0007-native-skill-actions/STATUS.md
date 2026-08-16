---
task: TASK-0007
state: CLAIMED
worker: Luna native-core skill-actions implementer
worker_branch: codex/TASK-0007-native-skill-actions
worktree: .codex/worktrees/TASK-0007-native-skill-actions
base_commit: 74e58a0
spec_base_commit: e3eb2a7
started_at: 2026-08-16T10:00:00-07:00
expected_verification: powershell -File native/build.ps1 -RunTests
known_risks: resource gating and deterministic regeneration; cooldown/buff expiry off-by-one; no client/build edits
dependencies: TASK-0006 integrated at 269c174
implementation_commit: e7505ad
validator: /root/validate_task_0007
validator_verdict: PENDING
architect_review_required: true
---
