---
task: TASK-0022
state: REVIEW_REQUESTED
branch: codex/TASK-0022-browser-25d-phase1
commits:
  - 174d769
  - b035b56
base_commit: b035b569ecc269f79c1113d7f8600db2198a273a
---

# TASK-0022 report — Browser 2.5D Phase 1 camera + focus conformance

## Implementation

- Set the browser perspective camera's named ARPG preset to horizon `-0.6H`,
  focus `0.52H`, and base user zoom `0.85`; the renderer now boots at that
  preset while keeping the legacy renderer toggle intact.
- Set depth-of-field strength to zero at the ARPG default and blend it only
  during close zoom. The existing shader/JS projection seam remains shared and
  numerically aligned.
- Removed the terrain fragment shader's gamma lift and desaturation. Terrain is
  fetched neutrally and receives the single ambient/cloud/light grade in
  `LightingRenderer` after composition.
- Removed per-sprite shadow blur from raised terrain billboards and actor
  sprites. Existing flat foot/contact ellipses remain.
- Removed the canvas-wide CSS grade from perspective mode and scoped the
  historical correction to an explicit `.legacy-renderer` class.

No gameplay, server, native, chronicles, asset, terrain-horizon, haze, or
Phase-2+ atmosphere changes were made.

## Parameter dump

Headless module evaluation against the implementation:

```json
{
  "preset": {
    "horizonRatio": -0.6,
    "focusRatio": 0.52,
    "baseUserZoom": 0.85,
    "maxDofStrength": 0.82
  },
  "default": {
    "horizon": -450,
    "focus": 390,
    "userZoom": 0.85,
    "dofStrength": 0,
    "defaultCoc": 0
  },
  "scaleRatioNearToFar": 3.1366120218579243,
  "closeZoomDofStrength": 0.82,
  "shaderProjectionParityPx": 5.684341886080802e-14
}
```

## Evidence

All captures use the same 1440×1000 headless browser viewport and the same
fresh guest onboarding scene. Each is lossy JPEG under the 250KB limit:

- [before-arpg.jpg](before-arpg.jpg) — baseline at `b035b56` (219,204 bytes)
- [after-arpg.jpg](after-arpg.jpg) — Phase-1 implementation (232,978 bytes)
- [reference-demo.jpg](reference-demo.jpg) — reference demo after its default
  intro is dismissed (138,638 bytes)
- [after-vs-reference.jpg](after-vs-reference.jpg) — labeled side-by-side
  comparison (222,446 bytes)

## Acceptance

- `npm run test:unit` — PASS (115 files, 744 tests)
- `npm run playtest` — pending final clean run
- `npm run smoke:browser` — pending final clean run
- `npm run lint:css -- --quiet` — PASS
- `npx eslint src/core/rendering/perspective-camera.js src/core/rendering/perspective-renderer.js src/core/rendering/terrain-renderer.js src/components/GameCanvas.vue` — PASS
- `git diff --check` — pending final clean run

## Review request

This task is submitted for architect review at the Phase-1 boundary. The
renderer remains runtime-switchable; the legacy correction is scoped rather
than deleted, and the reference's later haze/lighting/atmosphere tuning is
intentionally left to later phases.
