---
task: TASK-0024
state: REVIEW_REQUESTED
branch: codex/TASK-0024-browser-25d-phase3
coordinator: kimi-work
base_commit: 104535d
---

# TASK-0024 report — Browser 2.5D Phase 3 lighting + atmosphere retune

## Executive summary

The atmosphere stack (ambient day/night grade, mist blobs, god rays, cloud
shadows, vignette) is retuned as a set to the D-108 reference values now that
Phase 1–2 removed the blur/wash the weakened stack was compensating for. Git
history confirmed the original port (`5f57414`) matched the reference exactly;
two pre-overhaul readability commits (`7204499`, `a7458ef`) progressively
diluted it. This task restores reference parity while keeping the owner's
300 s day length and stopping the vignette just short of the reference's
corner crush.

## Approach

Every knob was retuned against the reference demo source
(`docs/reference/25d-overhaul/dist/songs-of-the-mire.html`: `ambient()`,
`buildLight()`, `buildVignette()`, mist/cloud loops) rather than invented.
Rendering-only; no gameplay, no new light emitters (projectile/skill lights
were already hooked into `collectDynamicLights`).

## Atmosphere knobs touched (old → new)

`src/core/rendering/lighting-renderer.js`:
- `AMBIENT_KEYFRAMES` → reference grade exactly:
  - t 0.00: [255,247,231] → [255,244,224]
  - t 0.30: [255,242,218] → [255,240,214]
  - t 0.45: [255,218,176] → [255,205,150]
  - t 0.58: [184,174,219] → [150,140,205]
  - t 0.80: [148,158,211] → [110,120,190] (deeper reference night)
  - t 0.90: [220,193,190] → [210,180,175]
  - t 1.00: [255,247,231] → [255,244,224]
- `DAY_LENGTH_SECONDS`: unchanged at 300 (plan §7 leaves 90 vs 300 to owner
  taste; the 300 s choice carries an owner-authored comment).
- Cloud shadow core colour: rgba(236,238,242) → rgba(196,198,208) (reference).
- Cloud drift speeds ×4 (0.0017/0.0011/0.0015/0.0009 →
  0.0068/0.0044/0.0060/0.0036 of lightmap-width/s) so a crossing takes
  minutes per the reference ("slow, minutes per crossing"), not ~20 minutes.
- Vignette edge alpha: 0.18 → 0.45 (named `VIGNETTE_EDGE_ALPHA`). The
  reference runs 0.55; this frame starts darker than the demo's pastel art,
  so the stack settles just below reference strength to avoid corner crush.
- Light-emitter intensity curve: unchanged (lights already scale with the
  night factor; verified reading at night in `after-night.jpg`).

`src/core/rendering/atmosphere-renderer.js`:
- Mist blob alpha: 0.026±0.009 → (0.10±0.06)×0.9 (reference pulse × gradient
  core), colour (184,207,190) → (190,215,180), drift 36 u @ 0.14 rad/s →
  40 u @ 0.2 rad/s, height offset 18 → 20 × scale. Mist stays below the
  combat layer (deliberate Verdigris ordering, unchanged).
- God rays: composite 'screen' → 'lighter', daylight falloff
  max(0, 1−night×1.25) → clamp(1−night×1.6, 0, 1), alpha
  daylight×(0.38+0.05·sin(t·0.18)) → dayF×(0.55+0.20·sin(t·0.5)), drift
  3 px @ 0.035 rad/s → 14 px @ 0.13 rad/s. Baked beams (built once,
  vertical fade) already matched the reference; untouched.
- Fireflies: unchanged (already reference-faithful).

## Evidence (captures/, all lossy JPEG ≤ 250 KB, 1440×1000)

- [before-arpg.jpg](captures/before-arpg.jpg) / [after-arpg.jpg](captures/after-arpg.jpg)
  — ARPG default at the village spawn (same viewpoint as TASK-0023).
- [before-edge-north.jpg](captures/before-edge-north.jpg) /
  [after-edge-north.jpg](captures/after-edge-north.jpg) — open field toward
  the map edge (village north road, 42,98): horizon, treeline, sky, mist and
  rays all visible (the 0023 review problem-1 shot).
- [before-edge-east.jpg](captures/before-edge-east.jpg) /
  [after-edge-east.jpg](captures/after-edge-east.jpg) — second open-field
  edge angle (63,108).
- [after-night.jpg](captures/after-night.jpg) — night grade at t≈0.80
  (evidence-only `performance.now()` offset in the capture script): deep
  reference blue, fireflies reading, scene still readable.
- [reference-demo.jpg](captures/reference-demo.jpg) — the D-108 demo at its
  morning start.
- [after-vs-reference.jpg](captures/after-vs-reference.jpg) — side-by-side.
- [capture.mjs](captures/capture.mjs) — the capture harness (guest login →
  walk-to-edge via the dev-only `/world/players` endpoint; Playwright).

Before/after at morning light is intentionally subtle — the big grade
differences land at dusk/night (see after-night). Morning differences:
present mist pulse, warmer road/grass grade, firmer corners.

## Test commands + outcomes

- `npm run test:unit` — PASS (115 files, 744 tests)
- `npm run smoke:browser` — PASS (1/1; own server lifecycle, port 6500
  verified released afterward)
- `npm run playtest` — PASS (31/31 scenarios)
- `git diff --check` — clean

Environment note: `start-server-and-test` shells out to `wmic.exe`, which is
not on Git Bash's default PATH; I ran gates with
`PATH+=":/c/Windows/System32/wbem"`. No repo change needed.

## Public interfaces added/changed

None. Only module-private constants changed; `VIGNETTE_EDGE_ALPHA` is a new
named constant (not exported). Existing exports unchanged.

## Deviations

- `tests/unit/lighting-renderer.spec.js` is not literally matched by the
  spec's owned test globs (`tests/unit/rendering*.spec.js`). The spec's own
  invariant section authorizes "unit-test expectation updates for retuned
  constants … per the QUESTION-0004 precedent," and the inherited night-floor
  assertion (min channel ≥ 140) directly contradicts the mandated reference
  night grade ([110,120,190]). Following QUESTION-0004 exactly: the focused
  expectation update is kept as a documented deviation — and the replacement
  assertion is *stronger* (exact equality with the reference night grade plus
  a ≥100 crushed-black floor), not weaker. No assertion was deleted.
- Vignette settled at 0.45 rather than the reference 0.55 (readability
  margin on darker pixel art; see knob list). Flagging for the architect's
  mood-parity judgment on the captures.

## Manual verification

Walked the guest Scion through the village to both edge shots at ARPG
defaults; night pass verified fireflies and grade depth; reference demo
captured on the same machine for the side-by-side.

## Revision 1 (review 50b4037, verdict REVISE — "renormalize to Verdigris
albedo; numeric luminance bar")

Review finding: the verbatim reference grade double-darkened Verdigris's
darker tile art; the after midday frame lost luminance vs before. Corrections
applied:

1. **Ambient keyframes renormalized** — the reference day/night CURVE is
   kept, anchored so the midday window multiplies at the pre-retune neutral
   grade over Verdigris art. Anchor: t=0.30 = [255,247,231] exactly (the
   pre-retune midday); every other keyframe scaled per channel by
   reference[t] / reference[0.30] against that anchor. Results: t=0.00
   [255,251,242], t=0.45 [255,211,162], t=0.58 [150,144,221], t=0.80
   [110,124,205] (deep night kept), t=0.90 [210,185,189].
2. **Vignette** `VIGNETTE_EDGE_ALPHA` 0.45 → **0.22** (under the review's
   ≤0.30 ceiling with margin; midday corners read clean).
3. **Cloud cores** — the verbatim reference core (196,198,208) was the main
   midfield darkener (regional probe: center band 57.4 → 44.2); renormalized
   in steps to **(232,233,238)**, keeping the 4x drift-speed fix. Mist, god
   rays, night emitters unchanged per correction 3.
4. **Luminance bar (measured, `captures/measure-luminance.py`)**:
   - `before-arpg.jpg` 39.82 vs `after-arpg.jpg` **40.14** (+0.32) ✓
   - `before-edge-north.jpg` 37.33 vs `after-edge-north.jpg` **37.98**
     (+0.65) ✓
   Both after captures now average ≥ their before counterparts at the same
   scene and time-of-day window.
5. Full capture set re-shot with the same harness (all `after-*` files,
   `reference-demo.jpg`, `after-vs-reference.jpg` refreshed).

Night-grade unit expectation updated to the renormalized [110,124,205]
(same QUESTION-0004 precedent; assertion still exact-value, still floors at
≥100).

## Revision 1 verification

- `npm run test:unit` — PASS (115 files, 744 tests)
- `npm run smoke:browser` — PASS (1/1, port 6500 released)
- `npm run playtest` — PASS (31/31). Two earlier attempts flaked on
  `dev:state` timeouts / the known timing-sensitive `gear-outcomes` under
  machine contention (multiple coordinators active); the playtest path never
  loads the client renderer, so these cannot be caused by this diff. A
  clean rerun passed 31/31.

## Commits

- `889f46a` (rebased: `4e9274e`) — the full retune + evidence + report
- revision 1 commit: see STATUS.md `revision_commit`

## Unresolved questions / risks / follow-ups

- If the architect wants full reference vignette strength (0.55), it is a
  one-constant change — but note review rev 1 capped it at ≤0.30.
- Phase 4 (DoF coupling) and Phase 5 (perf) remain untouched per scope.
- Dawn/dusk grades follow from the keyframe adoption; only morning and night
  were capture-verified (day-cycle wall-clock cost). The capture harness can
  offset to any `t` if the review wants dusk evidence too.
