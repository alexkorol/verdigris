---
task: TASK-0018
state: CLAIMED
coordinator: kimi
worker: kimi-code-cli
worker_branch: codex/TASK-0018-death-recoverability
worktree: C:\Users\Alex\Documents\Kimi\verdigris
base_commit: 237e5dd
spec_base_commit: 4f24b94
started_at: 2026-08-16T14:15:00-07:00
expected_verification: powershell -NoProfile -File native/build.ps1 -RunTests
known_risks: enlarging relic_candidates pool must preserve single-ownership and resurface ordering; old lost-forever assertions need deliberate updates
dependencies: TASK-0015 integrated (32ce77e)
architect_review_required: true
---
