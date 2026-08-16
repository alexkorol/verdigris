---
task: TASK-0008
state: CLAIMED
worker: Luna native denylist-hardening implementer
worker_branch: codex/TASK-0008-denylist-hardening
worktree: .codex/worktrees/TASK-0008-denylist-hardening
base_commit: 74e58a0
spec_base_commit: e3eb2a7
started_at: 2026-08-16T10:00:00-07:00
expected_verification: python native/tools/check_legacy_denylist.py; python native/tools/check_legacy_denylist.py --self-test; powershell -File native/build.ps1 -RunTests
known_risks: false positives from token normalization; extension coverage; preserve native baseline pass and browser exemption
dependencies: TASK-0005 integrated at 70c234f
implementation_commit: 56ac8f0e8b3b12007231db3b51a7387c8f54b1c2
validator: /root/validate_task_0008
validator_verdict: PENDING
architect_review_required: true
---
