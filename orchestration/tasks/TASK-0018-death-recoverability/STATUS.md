---
task: TASK-0018
state: REVIEW_REQUESTED
coordinator: codex
worker: Codex coordinator
worker_branch: codex/native-reconstitution
worktree: .
base_commit: 32ce77e
spec_base_commit: 2af6b2d
started_at: 2026-08-16T14:35:00-07:00
expected_verification: powershell -NoProfile -File native/build.ps1 -RunTests
known_risks: enlarge the deterministic re-entry pools without duplicating ownership or collapsing extraction risk; preserve successor-empty semantics
dependencies: TASK-0015 integrated
architect_review_required: true
implementation_commit: 37ab720
verification: powershell -NoProfile -File native/build.ps1 -RunTests (PASS); git diff --check (PASS); named D-106 recovery/replay tests (PASS)
---
