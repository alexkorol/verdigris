---
task: TASK-0025
state: IMPLEMENTED
coordinator: codex
worker: Codex/Luna implementation worker
worker_branch: codex/TASK-0025-instance-lifecycle-correctness
worktree: .codex/worktrees/TASK-0025-instance-lifecycle-correctness
base_commit: aa52054
started_at: 2026-08-16T17:05:00-07:00
expected_verification: powershell -File native/build.ps1 -RunTests
known_risks: preserve relic/death registration while retiring floor leftovers; delay route unlock until the final living monster; keep deterministic replay
dependencies: TASK-0017 integrated at aa52054
architect_review_required: true
implementation_commit: 63df51f
verification: powershell -NoProfile -File native/build.ps1 -RunTests PASS; git diff --check PASS
---
