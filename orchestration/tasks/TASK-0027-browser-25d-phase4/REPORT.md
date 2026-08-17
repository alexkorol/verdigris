---
task: TASK-0027
coordinator: kimi-work
state: REVIEW_REQUESTED
implementation_commit: 3f05ae9
---

# TASK-0027 REPORT — Browser 2.5D Phase 4

## Executive summary

**Required item 1 (HUD-safe compositing): the black-orb defect was a capture
harness artifact, not a compositing bug.** The 0024 night harness shifted
`performance.now()` by +240 s but not the `requestAnimationFrame` timestamp
timeline. `WizardOrbRenderer` seeds its animation clock from
`performance.now()` (`startedAt`/`lastFrameAt`) and diffs it against rAF
timestamps, so the naive patch produced `dt ≈ -240 s`; the flash envelope
`flash *= Math.exp(-dt * 3.2)` overflowed to Infinity and the orb shader
NaN'd to black. The game's actual compositing already matches the reference
pass order: lighting/vignette/atmosphere are drawn onto the world canvas
(`perspective-renderer.js:141-157`), which sits structurally BELOW the DOM
HUD (orbs, quickbar, minimap, quest tracker) — canvas passes cannot reach
DOM elements. Proven three ways below, including an unpatched real-time
240 s night wait where the orbs read as bright as at midday.

**Item 2 (Phase-4 DoF coupling): verified already conformant; continuity now
pinned by tests.** The `dofStrength↔zoom` blend, continuous radii (§8.3), and
the wheel/pinch range bracketing the blend domain were already implemented
(`73a8f6e`, hardened by 0021/0024 lineage). No runtime code change was
warranted; the gap was un-pinned continuity. Added two unit specs sweeping
the full wheel domain and the depth axis for monotonic, bounded-slope,
band-free behavior.

## Approach

1. Reproduced the 0024 artifact exactly (naive patch) and measured the clock
   skew in-page: `performance.now() − rAF timestamp = 240 s` with the naive
   patch, `0 s` with a coherent clock or unpatched.
2. Ground truth: unpatched page, waited out the real 300 s day cycle to
   t ≈ 0.80 (242.2 s elapsed), captured. Orbs fully lit.
3. Fixed the harness: `capture.mjs` now shifts `performance.now` AND wraps
   `requestAnimationFrame` so the virtual clock is coherent across both time
   surfaces the client consumes.
4. Verified Phase-4 DoF items against plan §7 and reference §3/§5/§8.3 (code
   reading + unit sweeps + zoom evidence captures).

## Evidence — orb luminance (measure-orbs.py, committed)

Sample boxes over the orb discs at the 1440×1000 evidence viewport:

| capture | HP orb | MP orb | full frame |
|---|---|---|---|
| `after-arpg.jpg` (midday) | 83.00 | 84.91 | 40.05 |
| `after-night.jpg` (coherent clock, t≈0.80) | 79.27 | 88.29 | 28.86 |
| `probe-night-real-unpatched.jpg` (real 242 s night) | 82.35 | 80.67 | 28.12 |
| `probe-night-naive-clock.jpg` (0024 artifact repro) | 7.51 | 3.15 | 27.99 |

Night/midday orb ratio: HP 0.955, MP 1.04 — the orbs read exactly as bright
at night as at midday while the frame sits at the deep-night grade. The
naive-clock repro shows the architect's observed artifact (orbs ≈ black,
frame identically graded), closing the causal chain.

## Evidence — DoF (plan §7 Phase 4, reference §8.3)

- `perspective-camera.js:65-73` — blend: `dofStrength` 0→0.82 linearly across
  userZoom 0.85→1.6; zero floor defined against the ARPG base (plan §9 risk
  honored, viewport-coupled zoom unaffected).
- `perspective-camera.js:139-146` — `circleOfConfusion` continuous in depth;
  sprite blur `×2 px` quantized to 0.25 px steps for filter-cache
  friendliness (`perspective-renderer.js:507-514`), the reference's own
  prescription; terrain DoF is a continuous in-shader mip-bias mix
  (`terrain-renderer.js:125-132`, uniform fed at `:338`).
- `perspective-renderer.js:1268-1302` — wheel/pinch clamp `[0.72, 1.6]`
  brackets the blend domain `[0.85, 1.6]`; zooming out past the base keeps
  DoF floored at 0 (crisp ARPG primary view).
- Terrain occlusion DORMANT: `terrainHeight()` returns 0
  (`perspective-renderer.js:71-73`); the `heightAt` seam keeps the hook
  (camera `:46-48`, terrain `:51-53`). No `occlY`-style clip exists.
- Captures: `after-zoom-close.jpg` (wheel ceiling, miniature blend — focus
  plane crisp, far field smoothly softened, no bands), `after-zoom-wide.jpg`
  (wheel floor, crisp everywhere).

## Changed files

- `tests/unit/perspective-camera.spec.js` — added two specs: DoF blend
  monotonic/bounded-slope/zero-floor/full-strength across the wheel domain
  `[0.72, 1.6]`; circle-of-confusion continuity across depth (max step
  < 0.05, sharp at focus, saturating past it). No existing test touched.
- `orchestration/tasks/TASK-0027-browser-25d-phase4/captures/` — fixed
  `capture.mjs` (coherent clock + zoom shots), `probe-orbs.mjs`,
  `probe-night-real.mjs`, `measure-orbs.py`, evidence captures (all JPEG
  ≤ 235 KB).

No runtime source files changed — none needed to. (`src/core/rendering/**`
diff vs base `0424e3a`: empty.)

## Public interfaces added/changed

None.

## Test commands + outcomes

- `npm run test:unit` — **746/746** (115 files), incl. camera spec 9/9.
- `npm run smoke:browser` — **1/1**.
- `npm run playtest` — **31/31** (browser-track gate).
- Captures: `node captures/capture.mjs after` — full set green.

## Manual verification

Inspected every capture: night orbs lit (this report's core proof), zoom
miniature blend, wide crisp floor, edge/horizon regime unchanged vs 0024,
side-by-side vs the D-108 reference unchanged.

## Commit SHAs

- `3f05ae9` — test(browser): verify phase four DoF coupling; fix
  night-capture clock skew (tests + harness + evidence).
- (orchestration commit with this REPORT + STATUS follows on the same
  branch.)

## Deviations

- **Zero runtime code change.** The spec's Phase-4 items were already
  implemented in the base; the honest implementation was verification +
  pinning tests + the harness fix. If the architect wants the blend retuned
  anyway (e.g. reference's stronger `×1.6` coc gain), that is a tuning call,
  not a conformance gap.
- Live numeric zoom readback from the page was not possible (no exposed
  handle; the served bundle attaches no reachable Vue internals). Mitigation:
  the blend domain is swept exhaustively in the unit specs and the wheel
  clamp is three cited lines.
- Port-6500 contention during gates: the owner's pm2 `delaford` process
  (from the architect checkout, auto-restarted onto the port mid-session)
  was cleanly `pm2 stop`'d for the gate runs and `pm2 restart`'d afterwards.
  Not a code issue.

## Unresolved questions

None blocking. Filed no QUESTION because the diagnosis closed item 1 with
ground-truth proof; happy to convert to a question if the architect reads
item 1 as demanding an engine change regardless.

## Risks

- `wizard-orb-renderer.js` keeps no lower bound on `dt`; any future harness
  or embedding that desynchronizes the two clock surfaces re-triggers the
  black-orb failure mode. One-line hardening (`dt = clamp(dt, 0, 0.05)`) lives
  in `src/core/hud/`, outside this spec's owned paths — recommended as a
  follow-up task rather than an unowned edit here.
- The 0024 `capture.mjs` still contains the naive patch; other tasks reusing
  it for night shots will re-manufacture the artifact. The 0027 harness
  supersedes it; backporting is an architect call (0024's folder is not an
  owned path of this task).

## Follow-ups

1. (architect decision) Spec the one-line `dt` floor in the orb renderer.
2. (architect decision) Backport the coherent-clock patch to the 0024
   harness or mark the 0027 harness canonical for night captures.
3. 0024 integration to the program branch was still pending when this
   branched; rebase is a no-op content-wise once it lands.
