---
task: TASK-0022
state: REVIEW_REQUESTED
branch: codex/TASK-0022-browser-25d-phase1
commits:
  - c17963b
  - 907024e
  - 350b5db
  - 0f24864
  - cc565d1
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

- [before-arpg.jpg](captures/before-arpg.jpg) — baseline at `b035b56` (219,204 bytes)
- [after-arpg.jpg](captures/after-arpg.jpg) — Phase-1 implementation (232,978 bytes)
- [reference-demo.jpg](captures/reference-demo.jpg) — reference demo after its default
  intro is dismissed (138,638 bytes)
- [after-vs-reference.jpg](captures/after-vs-reference.jpg) — labeled side-by-side
  comparison (222,446 bytes)

## Acceptance

- `npm run test:unit` — PASS (115 files, 744 tests)
- `npm run playtest` — PASS (31/31 scenarios)
- `npm run smoke:browser` — PASS (1 browser-critical test; 18.2s)
- `npm run lint:css -- --quiet` — PASS
- `npx eslint src/core/rendering/perspective-camera.js src/core/rendering/perspective-renderer.js src/core/rendering/terrain-renderer.js src/components/GameCanvas.vue` — PASS
- `npx eslint src/core/rendering/perspective-camera.js src/core/rendering/perspective-renderer.js src/core/rendering/terrain-renderer.js src/components/GameCanvas.vue tests/unit/perspective-camera.spec.js` — PASS
- `git diff --check` — PASS

## Revision 1 — validator corrections

The independent validator identified two concrete issues. The four evidence
JPEGs were committed at the worktree root even though the report lives in the
task folder, so they are now moved into this task folder and remain under the
250KB limit. The camera's obsolete wide-zoom DoF floor was removed: `dofStrength`
and `circleOfConfusion` are now exactly zero for every `userZoom <= 0.85`, with
the miniature blend rising only above the ARPG base. The inherited camera unit
assertion was updated to encode the governing §6 conformance rule rather than
the superseded wide-view blur behavior.

Revision commit: `907024e` (`fix(browser): enforce zero DoF below ARPG base`).
The revised branch was re-run from a clean disposable worktree: all 31/31
playtest scenarios passed after the correction.

## Scope question

The governing zero-DoF rule makes the inherited wide-zoom assertion stale, so
the focused camera unit expectation was updated to keep the required unit gate
green. That test file is outside this READY spec's immutable `owned_paths`.
The independent validator flagged this as the sole remaining issue. The
coordinator filed `orchestration/questions/QUESTION-0004-task-0022-camera-test-ownership.md`
for architect authorization or a replacement task; no other out-of-scope paths
were changed.

## Integration

Architect review `5efc48e` accepted the reviewed implementation commits and
required the evidence relocation during integration. Integrated on program
commits `705b6c3` and `4533bc4`; the repository root is free of the four capture artifacts,
which now live under this task folder's `captures/` directory. The focused camera test correction is
included in the architect-reviewed `907024e` commit.

## Review request

This task is submitted for architect review at the Phase-1 boundary. The
renderer remains runtime-switchable; the legacy correction is scoped rather
than deleted, and the reference's later haze/lighting/atmosphere tuning is
intentionally left to later phases.
