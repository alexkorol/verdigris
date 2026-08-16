---
task: TASK-0014
state: CLAIMED
worker: Codex coordinator
worker_branch: codex/native-reconstitution
worktree: .
base_commit: 74374c7
spec_base_commit: 74374c7
started_at: 2026-08-16T13:04:00-07:00
expected_verification: npm run smoke:browser; npm run test:e2e; verify port 6500 is released
known_risks: keep the change to the package.json smoke:browser script only; preserve the existing build step and port 6500 lifecycle
dependencies: none
architect_review_required: true
---
