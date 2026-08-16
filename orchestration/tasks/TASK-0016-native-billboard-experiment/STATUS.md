---
task: TASK-0016
state: REVIEW_REQUESTED
coordinator: codex
worker: Codex coordinator
worker_branch: codex/native-reconstitution
worktree: .
base_commit: b1ef7c2
spec_base_commit: 11a5325
started_at: 2026-08-16T14:10:00-07:00
expected_verification: powershell -NoProfile -File native/build.ps1 -RunTests -RunClient; driven PrintWindow asset and fallback captures
known_risks: runtime GDI+/AlphaBlend ABI and path resolution must degrade cleanly without build or simulation changes
dependencies: TASK-0013 integrated
architect_review_required: true
implementation_commit: 6d1b7d6
verification: powershell -NoProfile -File native/build.ps1 -RunTests -RunClient (PASS); git diff --check (PASS); driven asset/fallback PrintWindow captures (PASS)
---
