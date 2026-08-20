---
task: TASK-0075
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0075-native-terrain-tiles-cursor
base_commit: 69d60133b8fe67087a856d837f6d714fc9ddb4c6
architect_review_required: true
---

# TASK-0075 REPORT — native ground terrain tiles

## Executive summary

The world floor now renders as camera-correct tiled `terrain1.png` /
`terrain4.png` plates (loaded from `prototypes/founding-slice/assets/`) instead
of a flat dark grid. Deterministic per-tile variation uses a coord hash with
route-theme bias (marsh/barrow/circle → more terrain4). Missing assets fall
back to the prior flat fill + grid. `Floor` and `Tile` render-list ops are
recorded; scenarios assert camera/zoom invariance over overlapping tiles.
AFTER reference captures committed under this task folder (0070 BEFORE preserved).

## Approach

- `load_terrain_plate` loads full-opacity ground plates (no magenta key).
- `draw_floor` tiles the visible arena in world space at `kTileUnits`, records
  one `Floor` op plus per-tile `Tile` ops (label `terrainN:tx:ty`).
- Local scenarios, remote `paint_scene`, and `--reference-scene` all share the
  same path. Grid lines only on flat fallback.

## Changed files

- `native/client/render_list.hpp` — `Floor`, `Tile` ops
- `native/client/main.cpp` — terrain load/draw, scenario extensions
- `orchestration/tasks/TASK-0075-native-terrain-tiles/captures/after-*` (15 files)

## Verification

`powershell -File native/build.ps1 -RunTests -RunClientScenarios` (2026-08-20,
exit 0):

- denylist PASS; core/networking/camera2d/session tests PASS
- All 7 client scenarios PASS including new terrain tile checks on
  `move-and-camera`, `zoom-invariance`, `remote-render-list`

`native\build\verdigris_client.exe --reference-scene all` (exit 0):

```text
ok 01-route-entrance (311 ops)
ok 02-pack-combat (336 ops)
ok 03-elite-telegraph (313 ops)
ok 04-named-drop-gear (335 ops)
ok 05-critical-health (341 ops)
```

Two-run JSON identity held; AFTER PNG/JSON copied to
`orchestration/tasks/TASK-0075-native-terrain-tiles/captures/after-*`.
0070 captures restored unchanged (BEFORE baseline).

Architect: rerun gates, eyeball `after-02-pack-combat-1920x1080.png` vs
side-by-side benchmark, play pass.

## Deviations

None. No `native/src` edits. Side-by-side composite rerun left for architect
after acceptance.
