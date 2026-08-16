---
task: TASK-0023
state: REVIEW_REQUESTED
branch: codex/TASK-0023-browser-25d-phase2
commits:
  - 48725e3
base_commit: 355aa168d78834c97cb48907cf6b05fec2402c11
---

# TASK-0023 report — Browser 2.5D Phase 2 terrain + horizon conformance

## Executive summary

The browser perspective terrain now uses the governing reference saturating
haze curve, keeping the playfield clear while building a permanent horizon.
The sky/treeline pass is tuned to the ARPG camera and the finite terrain edge,
while the legacy fallback and shared projection/height seams remain intact.

## Implementation

- Replaced the weak midfield haze with the reference curve:
  `clamp((depth / focusDepth - 1.12) / 1.02, 0, 1) * 0.96`.
- Kept the terrain fetch neutral and preserved Phase-1 zero-DoF behavior;
  no Phase-3 lighting/atmosphere or Phase-4 coupling changes were made.
- Tuned the perspective sky gradient and treeline colors for the new ARPG
  horizon, selecting the skyline from both the virtual fog horizon and the
  finite terrain edge to avoid a hard map seam.
- Preserved the legacy ground fallback, renderer switch, and one
  projection/height contract.

## Evidence

All four captures are 1440×1000 lossy JPEGs under 250KB in
`captures/`:

- [before-arpg.jpg](captures/before-arpg.jpg) — 247,943 bytes
- [after-arpg.jpg](captures/after-arpg.jpg) — 248,992 bytes
- [reference-demo.jpg](captures/reference-demo.jpg) — 162,029 bytes
- [after-vs-reference.jpg](captures/after-vs-reference.jpg) — 227,971 bytes

## Verification

- `npm run test:unit` — PASS (115 files, 744 tests)
- `npm run smoke:browser` — PASS (1/1)
- `npm run playtest` — PASS (31/31)
- Production build — PASS
- ESLint — PASS
- `git diff --check` — PASS

## Independent validation

Validator `/root/validate_task_0023` independently accepted commit
`48725e3`. It confirmed the rendering-only scope, governing haze curve,
finite-edge horizon handling, preserved legacy/projection/height seams, four
task-folder captures under the size limit, and the 744/744 unit, 1/1 smoke,
31/31 playtest, ESLint, and diff checks.

## Scope and risks

Changed paths are limited to `src/core/rendering/**` and the task evidence
folder. No gameplay, server, native, component, prototype, package, lighting,
atmosphere, or DoF coupling changes were made. The reference curve follows
the documented `* 0.96` cap; its on-screen saturation is verified visually in
the captured horizon comparison.

## Review request

Submitted for architect review at the Phase-2 boundary. Phase 3 lighting and
atmosphere retuning remains intentionally deferred.
