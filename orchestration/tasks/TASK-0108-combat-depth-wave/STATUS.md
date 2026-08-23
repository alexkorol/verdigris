---
state: REVIEW_REQUESTED
coordinator: codex
worker: ox-sw-a
task: TASK-0108-combat-depth-wave
branch: codex/TASK-0108-combat-depth-wave-ox-sw-a
base_commit: 763684666b07483caeeebc2055c804f80bb1515e
worktree: Z:\Code\.worktrees\verdigris\ox-sw-a
started_at: 2026-08-22T22:51:48-07:00
finished_at: 2026-08-23T01:42:10-07:00
---

W1 ranged behaviour realized (telegraphed marked-tile shots beyond melee contact, cover-aware, replay-locked); full gate + all suites green; see REPORT.md. Note: networking.cpp emit-side split added monster:ranged-telegraph to protect the read-only gate-b slam-reveal contract (deviation 1 in REPORT.md).
