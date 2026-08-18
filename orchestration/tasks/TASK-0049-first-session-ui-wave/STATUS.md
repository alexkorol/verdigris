---
task: TASK-0049
state: CLAIMED
coordinator: deepseek
worker_branch: codex/TASK-0049-first-session-ui-wave-deepseek
base_commit: 34b7069f12930933b66fde0e81c27e2cb44007e8
started_at: 2026-08-18T11:23:49-07:00
dependencies: []
architect_review_required: true
expected_verification: npm run test:unit; npm run smoke:browser; npm run playtest (32/32); hard-fail Playwright captures of all five deliverables
known_risks: all five deliverables must read from existing payloads only (server/** is forbidden); tutorial-ticker and zone-preview wording must not bury combat lines or fabricate world data
---

Claiming TASK-0049 (first-session UI wave) as coordinator `deepseek`.

Five deliverables, smallest-that-lands, client presentation only:
1. House/Scion identity in the world HUD.
2. Directive mana-rejection copy (amount + cadence).
3. Tutorial ticker legibility for first-session beats.
4. Zone objective preview in the Adventure panel.
5. Skill-tree first-allocation hint.

No `server/**`, `native/**`, or playtest-assertion-loosening edits. Work on
worker branch `codex/TASK-0049-first-session-ui-wave-deepseek`.
