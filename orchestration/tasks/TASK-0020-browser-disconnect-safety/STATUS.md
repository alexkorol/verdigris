---
task: TASK-0020
state: CLAIMED
coordinator: codex
worker: Codex coordinator
worker_branch: codex/native-reconstitution
worktree: .
base_commit: 42297ed
spec_base_commit: 237e5dd
started_at: 2026-08-16T14:55:00-07:00
expected_verification: npm run test:unit; npm run playtest
known_risks: preserve the existing guest/Chronicles persistence seams while making persistence precede world removal and preventing all post-close combat/movement work
dependencies: none
architect_review_required: true
---
