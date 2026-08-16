---
task: TASK-0022
state: CLAIMED
coordinator: codex
worker: Codex coordinator
worker_branch: codex/TASK-0022-browser-25d-phase1
worktree: .codex/worktrees/TASK-0022-browser-25d-phase1
base_commit: b035b569ecc269f79c1113d7f8600db2198a273a
spec_base_commit: b56b767
started_at: 2026-08-16T15:20:00-07:00
expected_verification: npm run test:unit; npm run playtest; npm run smoke:browser
known_risks: keep the legacy renderer switchable, preserve one projection/height seam, and stop at Phase-1 mud fixes without atmosphere or terrain-horizon retuning
dependencies: TASK-0019 accepted/integrated; TASK-0020 revision submitted for review
architect_review_required: true
---
