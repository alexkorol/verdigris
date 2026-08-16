---
task: TASK-0015
state: REVIEW_REQUESTED
coordinator: codex
worker: Codex coordinator
worker_branch: codex/native-reconstitution
worktree: .
base_commit:  b682296
spec_base_commit: 11a5325
started_at: 2026-08-16T13:40:00-07:00
expected_verification: powershell -NoProfile -File native/build.ps1 -RunTests
known_risks: catalog must expose only the listed presentation constants and must not change gameplay values or duplicate definitions
dependencies: TASK-0011 integrated
architect_review_required: true
implementation_commit: 76ff52d
verification: powershell -NoProfile -File native/build.ps1 -RunTests (PASS); git diff --check (PASS); single-definition audit (PASS)
---
