---
task: TASK-0021
state: REVIEW_REQUESTED
branch: codex/native-reconstitution
commits:
  - d079e70
  - e67a89f
base_commit: 363ad39
---

# TASK-0021 report — ARPG default and reversible Miniature zoom blend

## Implementation

- Changed the founding slice's boot camera to the owner-ruled ARPG values:
  `zoom=.85`, `pitch=62`, `persp=.0006`, `anchor=.52`, `fog=.4`, `tilt=0`.
- Added a canvas wheel handler with named bounds and blend constants. Zooming
  in above `1.05` linearly raises perspective from `.0006` toward `.0013`
  and tilt from `0` toward `1`, clamped through `1.35`; zooming back out
  restores the ARPG treatment.
- Rebuilt the committed `index.html` with the existing build script. No
  harness, build script, or asset files were changed.

## Parameter dump

Headless browser dispatch against the rebuilt `index.html`:

```json
{
  "initial": {"zoom": 0.85, "pitch": 62, "persp": 0.0006, "anchor": 0.52, "fog": 0.4, "tilt": 0},
  "zoomed": {"zoom": 1.15, "persp": 0.000833333333333333, "tilt": 0.3333333333333328},
  "restored": {"zoom": 0.8499999999999999, "persp": 0.0006, "tilt": 0}
}
```

The zoomed sample is halfway through the named `1.05 → 1.35` blend window;
the restored sample demonstrates reversibility.

## Acceptance

- `node prototypes/founding-slice/build.mjs` — PASS
- `node prototypes/founding-slice/run-checks.mjs` — PASS (4/4)
- `git diff --check` — PASS

## Review request

This task is submitted for architect review. High Table and Miniature remain
available as explicit camera-lab presets; the wheel treatment only runs on
gameplay canvas wheel input.
