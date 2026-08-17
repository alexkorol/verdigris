---
task: TASK-0027
state: REVIEW_REQUESTED
branch: codex/TASK-0027-browser-25d-phase4-codex
commits:
  - c980cb1b
base_commit: 21495fd461cf941fa7d641e61368da89e5fa4436
---

## Executive summary

The Phase-4 browser rendering defect is corrected without changing the
simulation or HUD pass order. Orb smoothing now survives capture-harness
clock skew, the capture harness keeps `performance.now()` and RAF timestamps
in one clock domain, and sprite DoF blur uses the continuous zoom-coupled
radius rather than quarter-pixel quantization.

## Implementation

- Added finite, lower-and-upper-clamped RAF delta derivation in
  `wizard-orb-renderer.js`; negative or invalid deltas become zero.
- Removed quarter-pixel blur quantization in `perspective-renderer.js`.
- Strengthened camera DoF tests across intermediate zoom samples.
- Added focused orb clock tests and regenerated day/night, edge, reference,
  and side-by-side captures using the clock-safe harness.
- Preserved the governing pass order; no `src/components/**` changes were made.

## Changed files

- `src/core/hud/wizard-orb-renderer.js`
- `src/core/rendering/perspective-renderer.js`
- `tests/unit/perspective-camera.spec.js`
- `tests/unit/rendering-wizard-orb.spec.js`
- `orchestration/tasks/TASK-0027-browser-25d-phase4/captures/*`

## Interfaces

- Exported `frameDeltaSeconds` and `MAX_FRAME_DT_SECONDS` for focused,
  deterministic clock-domain tests. Runtime behavior remains internal to the
  orb renderer.

## Verification

- Focused tests: `npm run test:unit -- --run tests/unit/perspective-camera.spec.js tests/unit/rendering-wizard-orb.spec.js` — 10/10 passed.
- Full unit suite: `npm run test:unit -- --reporter=dot` — 116 files / 747 tests passed.
- Browser smoke: `npm run smoke:browser` — 1/1 passed on the clean rerun.
- Full playtest: first run had the existing timing-sensitive `gear-outcomes`
  threshold miss (30/31); clean rerun passed 31/31 scenarios.
- `git diff --check c0958074..c980cb1b` — passed.
- Capture files are 1440×1000 (or 1440×520 side-by-side) and all are under
  250 KB. HP orb circular-region mean luminance is 85.49 day / 84.96 night;
  MP is 80.92 day / 83.96 night, while the world grade visibly changes.
- Captures visibly retain bright red/blue orbs at night; the night image is
  `captures/after-night.jpg`.

## Manual checks

- Viewed `after-arpg.jpg` and `after-night.jpg` side by side: terrain grade
  changes to night while both HUD orbs remain legible and bright.
- Inspected the commit scope: only the two rendering modules, owned tests,
  and task captures changed.

## Specification deviations

- `src/core/hud/wizard-orb-renderer.js` is outside the literal Phase-4
  rendering glob, but the architect `RELEASE.md` explicitly authorizes the
  `dt` lower clamp as the answer to QUESTION-0005. This is the only source
  path deviation.

## Risks and limitations

- Playtest `gear-outcomes` is timing-sensitive and required one rerun; the
  final full gate is green.
- Capture harness still requires a running local server on port 6500 for
  regeneration.

## Questions for Fable or the owner

None.

## Integration notes

Integrate after architect acceptance. The browser-only commit is independent
of the already-integrated TASK-0028 native client merge.
