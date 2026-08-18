---
task: TASK-0053
state: REVIEW_REQUESTED
worker_commits:
  - de03538
  - 9351908
base_commit: 4178234
---

## Summary

Verify-first found the 25d overhaul did NOT already cover deliverables 1
and 2; both are implemented renderer-side in
`src/core/rendering/perspective-renderer.js` (the only source file
changed). Deliverable 3 (clustered accents) is generation-side and lives
in forbidden paths — QUESTION-0011 filed with evidence and options; the
task is submitted with 2 of 3 deliverables landed and the third awaiting
the architect's ownership ruling.

## Per-deliverable verdicts

1. **Exposed-face-only indoor walls — IMPLEMENTED.** The 25d stack drew
   every wall-background cell in range as a raised billboard (the
   "repeated brick wallpaper" read — see `captures/before-01...`). Now a
   wall cell earns a billboard only when its 8-neighbourhood touches
   walkable floor (`isExposedWallCell`); interior mass stays dark
   room-mass (the ground bake already skips wall gids). Evidence:
   `captures/before-01-barrow-interior.jpg` vs
   `captures/01-barrow-interior.jpg` — same Old Barrow room, same build
   parameters; wall faces trace the rooms, mass beyond goes dark.
2. **Tree-line collision boundaries — IMPLEMENTED.** Grove/wilds/marsh
   boundaries are vine-wall gids (`groups.wall.vines`), which rendered as
   flat vine/masonry carpet (`captures/before-02-grove-treeline.jpg` —
   the player stands inside a wall of vines). Exposed vine-wall cells now
   render as living-tree billboards chosen by a deterministic coordinate
   hash (`treeLineGidFor`), producing a dense tree-line
   (`captures/02-grove-treeline.jpg`). Collision untouched — presentation
   only, exactly as specced.
3. **Clustered accents — BLOCKED ON OWNERSHIP (QUESTION-0011).** Accent
   scatter is generated authoritatively server-side
   (`server/core/map.js:861-874`, flat 12% per-tile) and baked into the
   ground texture via `src/core/map.js` — both outside this task's owned
   paths. The scatter is visible in `captures/02-grove-treeline.jpg`
   (isolated flower cells). Options in the question; recommended: expand
   ownership to the floorPicker accent pass so clustering is generated
   with the existing seeded rng.

## Perf (spec constraint: no regression)

Method of 0029 phase 5 (synchronous RAF callback duration, 1440x1000,
12s WASD workload), but measured INSIDE the Old Barrow where the changed
path runs (`captures/_measure-frame-time.mjs`):

- before: mean 65.224ms, p95 212ms, max 271.2ms (198 samples)
- after:  mean 40.910ms, p95 114.9ms, max 128.4ms (286 samples)

Skipping unexposed wall billboards is a ~37% mean frame-time
IMPROVEMENT on this path, not a regression.

## Evidence

- `npm run test:unit` — 826/826 PASS (includes new
  `tests/unit/wall-exposure.spec.js`, 5 tests: exposure rules,
  out-of-bounds handling, deterministic tree gid variety).
- `npm run playtest` — 32/32 PASS (isolated run; p99 lag 32.3ms).
- ESLint on changed files — clean (also via lint-staged pre-commit).
- Browser gate: alternate-port pattern (port 6500 is the owner's
  persistent server, PID 17960; documented in 0041/0042 reports): own
  development server on 127.0.0.1:9882 +
  `PLAYWRIGHT_BASE_URL=http://127.0.0.1:9882 npx playwright test
  tests/e2e/browser-critical-loop.spec.mjs` — 1/1 PASS (16.2s).
- `npm run build` — PASS for both before and after captures.
- Captures are real Playwright screenshots of this branch's build, taken
  by `captures/_driver.mjs` (hard-fails if the scenes are never reached).

## Deviations

- Deliverable 3 not implemented (ownership question filed — see above).
- `docs/loop-journal.md` carries harness-appended session-arc rows from
  the mandated playtest runs.

## Risks / follow-ups

- Wall exposure is 8-neighbour vs BACKGROUND walkability; a room sealed
  behind a non-walkable foreground door tile still counts as exposed via
  its floor cells, so door faces keep their walls. Town buildings do not
  use dungeon wall gids and are unaffected.
- The tree-line applies wherever vine walls appear (grove, wilds, marsh).
  Marsh reading as swamp-trees is intentional-looking; flag if the
  architect wants marsh excluded.
- If QUESTION-0011 resolves to option 1, the accent clustering is a small
  seeded-blob change to `floorPicker` — happy to take it as a follow-up
  task.
