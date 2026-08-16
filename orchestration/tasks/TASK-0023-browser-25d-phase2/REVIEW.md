---
task: TASK-0023
verdict: ACCEPTED
reviewed_commits:
  - 48725e3
---

## What was reviewed

The 35-line rendering-only diff (haze curve in the terrain shader, sky/
treeline horizon selection in the renderer), the report's evidence, and
the after-vs-reference capture. Gates: the diff touches only
`src/core/rendering/**` (verified), so server playtest is unaffected by
construction; coordinator validation ran unit+smoke green, consistent
with the Phase-1 precedent on this exact pipeline.

## What is correct

- The shader now carries the exact reference §3 curve
  (`clamp((depth/focus − 1.12)/1.02, 0, 1) × 0.96`), replacing the weak
  0.24-cap midfield grey — playfield clear, far depth saturating into the
  sky, documented in-shader.
- Horizon selection from both the virtual fog horizon and the finite
  terrain edge avoids the hard map seam; legacy fallback and the shared
  projection/height contract untouched; Phase-1 zero-DoF preserved.
- Captures conform to the placement/size rules this time.

## Problems

None blocking.

1. (Evidence, for the next phase) The after capture is a village-center
   shot where walls/trees occlude the horizon, so the new curve barely
   shows. Phase-3's evidence must include an open-field shot toward the
   map edge where the horizon effect is actually visible.

## Required corrections

None.

## Architectural effect

Phase 2 done. Phase 3 (lighting/atmosphere retune) is specced as
TASK-0024. Integration approved.
