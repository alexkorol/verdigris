---
task: TASK-0037
state: REVIEW_REQUESTED
coordinator: codex
worker: Luna browser movement-feel worker
worker_branch: codex/TASK-0037-movement-feel-rework
worktree: C:\Users\Alex\Documents\ChatGPT\verdigris\.codex\worktrees\TASK-0037-movement-feel-rework
base_commit: b141cd9f
started_at: 2026-08-17T00:00:00-07:00
dependencies: none
expected_verification: npm run test:unit; npm run playtest; npm run smoke:browser; diagnosis evidence; movement constants table; dense walk/turn/diagonal captures
known_risks: preserve server authority and wire protocol; owned input/player paths overlap TASK-0038, so 0038 waits for this task
architect_review_required: true
implementation_commit: 46c51412
report: orchestration/tasks/TASK-0037-movement-feel-rework/REPORT.md
verification: unit 119 files/761 tests; playtest 31/31; build and alternate browser gate pass; dense movement captures committed
---
