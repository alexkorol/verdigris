---
task: TASK-0055
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0055-browser-followups-cursor
base_commit: f71815f351ea9da9ebd6459812b8a25d87a323a5
started_at: 2026-08-19T23:35:00-07:00
finished_at: 2026-08-19T23:58:00-07:00
architect_review_required: true
expected_verification: npm run test:unit (133 files / 838 tests); PLAYTEST_PORT=6580 npm run playtest 32/32; browser smoke 1/1 on 127.0.0.1:6581; hard-fail captures CAPTURES OK at 1366x768 and 1920x1080; adventure-objective-data.js deleted
---

Implemented and pushed for architect review.

Identity chip sits above the HP orb with a 240px cap and full `title` on hover.
Adventure preview fields (bossDisplayName, treasureItemLevel, depth) are additive
on player:login and party:update, sourced from THEME_MONSTERS + instanceItemLevelForDepth;
the client mirror is deleted.

Gates green: unit 838/838, playtest 32/32, browser smoke 1/1, hard-fail capture
CAPTURES OK (both viewports). Authentic negative: zoneObjective without a payload
invents no boss name (unit). Ports 6580-6599; never 6500.
