---
task: TASK-0017
state: INTEGRATED
coordinator: codex
worker: Codex/Luna implementation worker
worker_branch: codex/TASK-0017-native-movement-feel
worktree: .codex/worktrees/TASK-0017-native-movement-feel
base_commit: 8acafc8
started_at: 2026-08-16T16:25:00-07:00
expected_verification: powershell -File native/build.ps1 -RunTests -RunClient
known_risks: preserve fixed-step determinism, actor symmetry, melee/thrust reachability, readable continuous movement, and D-107 camera blend defaults
dependencies: TASK-0015 and TASK-0016 integrated on the program tip
architect_review_required: true
implementation_commit: 5b73a24
verification: powershell -NoProfile -File native/build.ps1 -RunTests -RunClient PASS; denylist PASS; core tests PASS; headless client PASS; git diff --check PASS
validator: /root/validate_task_0017
validator_verdict: ACCEPT
architect_review: ACCEPTED (f4ea440)
integration_branch: codex/integration-task-0017
integration_commit: ed1cd26
---
