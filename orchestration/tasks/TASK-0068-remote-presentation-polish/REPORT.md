---
task: TASK-0068
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0068-remote-presentation-polish-cursor
base_commit: 1f45eb337b29995485ba2b5adf60f5cdb00393c3
architect_review_required: true
---

# TASK-0068 REPORT — remote presentation polish

## Executive summary

The four 0064 review notes are in `paint_scene` (local and `--remote` share
it). Telegraph rings clip and clamp out of the HUD reserve. Monster
billboards read as foes (rust silhouette + squat fallback vs the tall green
scion; existing raider/boss sprites, larger). Stairs-up is an in-world EXIT
pad (`render::Op::Extraction` labeled `stairs-up`). Remote HUD always shows
a connection chip whose text is `connection ` + `connection_state_label`.

## Approach

- HUD safe zones: vitals (top-left LIFE/RESOURCE) and the skill strip.
  `paint_telegraphs` `ExcludeClipRect`s those, and sweep/thrust origin rings
  shrink until the recorded circle misses them.
- Foes: rust ground halo + 1.58/1.85 tile height (elites larger). Fallback
  billboard is short-wide rust vs the scion's tall green.
- Extraction pad: concentric green/gold rings, chevron stairs, EXIT caption.
- Chip: top-right pill for any `state.session`, Ready/connecting/lost colors.

## Changed files

- `native/client/main.cpp`
- `native/client/presentation_state.cpp` — Extraction label `stairs-up`

## Verification

`powershell -File native/build.ps1 -RunTests -RunClientScenarios` (2026-08-20):
denylist, core, networking, camera2d, session tests, and all D-119 scenarios
green, including:

- `telegraph-dodge: telegraph stays outside HUD reserve`
- `remote-render-list: Extraction pad marked stairs-up`
- `remote-render-list: connection chip uses connection_state_label`

Architect: `--remote` play pass and Gate A rubric rescore (target 12/12).

## Deviations

None. No `native/src` edits. No new art.
