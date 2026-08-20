---
task: TASK-0055
state: CLAIMED
coordinator: cursor
worker_branch: codex/TASK-0055-browser-followups-cursor
base_commit: f71815f351ea9da9ebd6459812b8a25d87a323a5
started_at: 2026-08-19T23:35:00-07:00
architect_review_required: true
expected_verification: npm run test:unit; full npm run playtest 32/32 (PLAYTEST_PORT=6580); browser smoke on 127.0.0.1:6581; hard-fail captures at 1366x768 and 1920x1080
---

Claimed per RELEASE.md (kimi-work quota-released). Fresh implementation from SPEC. Ports 6580-6599. Never 6500.
