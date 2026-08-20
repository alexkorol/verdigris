---
task: TASK-0070
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0070-reference-scenes-cursor
base_commit: 27d2be62038bba29abf68735288fd1d177b4c0aa
architect_review_required: true
---

# TASK-0070 REPORT — Stage 1 visual reference scenes

## Executive summary

`--reference-scene all` drives five seeded local setups, presents into an
offscreen DC, and writes 10 PNGs (1920x1080 and 1366x768) plus one
render-list JSON per scene. The driver runs each 1920 setup twice and
fails if the JSON differs. No new art.

Scenes: route entrance; pack combat (2+ monsters, swing); elite telegraph;
named drop with gear pane; critical health (life < 25% with screen pulse).

## Approach

- Flag on `verdigris_client.exe` (same process as `--scenario`).
- PNG via GDI+ `GdipSaveImageToFile` from a 32-bit DIB.
- Captures land in `orchestration/tasks/TASK-0070-reference-scenes/captures/`.

## Changed files

- `native/client/main.cpp` — `--reference-scene`
- `orchestration/tasks/TASK-0070-reference-scenes/captures/` — 10 PNG + 5 JSON

## Verification

`powershell -File native/build.ps1 -RunTests` green (2026-08-20).
`native/build/verdigris_client.exe --reference-scene all` exit 0; two-run
JSON match for all five scenes.

Architect: rerun `--reference-scene all` and eyeball one PNG per resolution.

## Deviations

None. No `native/src` edits.
