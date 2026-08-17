---
task: TASK-0036
state: CLAIMED
coordinator: codex
worker: Luna browser UI regression worker
worker_branch: codex/TASK-0036-ui-regression-sweep
worktree: C:\Users\Alex\Documents\ChatGPT\verdigris\.codex\worktrees\TASK-0036-ui-regression-sweep
base_commit: b141cd9f
started_at: 2026-08-17T00:00:00-07:00
dependencies: none
expected_verification: npm run test:unit; npm run smoke:browser; before/after inventory captures at 1920x1080 and ~1366x768; complete pane gallery and ranked defect report
known_risks: fix inventory only; report other pane defects without changing them; do not touch rendering internals or package/server files
architect_review_required: true
---
