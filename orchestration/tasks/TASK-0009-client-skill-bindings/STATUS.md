---
task: TASK-0009
state: CLAIMED
worker: Luna native-client skill-bindings implementer
worker_branch: codex/TASK-0009-client-skill-bindings
worktree: .codex/worktrees/TASK-0009-client-skill-bindings
base_commit: 0c51439
spec_base_commit: 0c51439
started_at: 2026-08-16T10:30:00-07:00
expected_verification: powershell -File native/build.ps1 -RunTests -RunClient; PostMessage-driven Q/E/R client pass
known_risks: HUD state must remain presentation-only; preserve D-007 controls and do not edit core/build files; effect timing and resource regeneration may be off by one tick
dependencies: TASK-0007 integrated at a832b2b; TASK-0004 integrated at 6396a0e
architect_review_required: true
---
