---
task: TASK-0052
state: CLAIMED
coordinator: kimi
worker: kimi-code-cli
worker_branch: codex/TASK-0052-suite-flake-hardening
worktree: C:\Users\Alex\Documents\Kimi\verdigris
base_commit: cc67a15
started_at: 2026-08-18T12:45:00-07:00
expected_verification: both scenarios green solo; full suite green under documented CPU load (0043 spinner); authentic negative (suppressed push still fails); default-mode npm run playtest green
known_risks: none structural - harness-only, asserts unchanged
architect_review_required: true
---

Claimed per RUN_STATUS routing (kimi → 0052). Harness-only: bounded
load-adaptive waits in front of existing asserts in first-goal.mjs and
house-treasury.mjs, using playtest/timing.mjs helpers unchanged.
