---
task: TASK-0049
state: REVIEW_REQUESTED
coordinator: deepseek
worker_branch: codex/TASK-0049-first-session-ui-wave-deepseek
base_commit: 34b7069f12930933b66fde0e81c27e2cb44007e8
started_at: 2026-08-18T11:23:49-07:00
architect_review_required: true
expected_verification: npm run test:unit; npm run smoke:browser; npm run playtest (32/32); hard-fail Playwright captures of all five deliverables
---

Implemented and pushed for architect review. All five deliverables are
client-presentation-only (no `server/**`, `native/**`, or playtest-assertion
changes):

1. House/Scion identity chip in the world HUD.
2. Directive mana-rejection copy (missing amount + recovery cadence).
3. Transient guide banner for Aldwyn's first-session beats.
4. Adventure zone objective preview (named Warden + guaranteed item-level
   treasure + depth).
5. Skill-tree first-allocation hint (data-driven starter node + highlight).

Gates green on the final code: `test:unit` 128 files / 809 tests;
`smoke:browser` 1/1; `playtest` 32/32. Hard-fail capture
`captures/capture-0049.mjs` produced five rendered PNGs with all on-screen
text assertions passing. Details in `REPORT.md`.
