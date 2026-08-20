---
task: TASK-0076
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0076-native-hud-parity-cursor
base_commit: 69d60133b8fe67087a856d837f6d714fc9ddb4c6
architect_review_required: true
---

# TASK-0076 REPORT — native HUD parity

## Executive summary

Replaced top-left text bars and bottom-left skill strip with browser-style HUD:
life/resource **orbs** (bottom corners, procedural GDI fill + low-life pulse),
centered **quickbar** (LMB/Q/E/R with cooldown overlays), and top-left
**minimap** from `WorldView` + scenery (remote-safe). New render-list ops
`Orb`, `Quickbar`, `Minimap`. HUD safe zones updated for telegraph clipping
(minimap top-left, bottom strip for orbs + quickbar).

## Approach

- `draw_orb`: clip-filled circle with numeric readout; life orb pulses below 25%.
- `paint_quickbar`: four grouped slots bottom-center; cooldown darkens from top.
- `paint_minimap`: player-centered arena map — scenery dots, red monsters,
  gold extraction, facing arrow.
- Pipelined on TASK-0075 terrain tiles (same `paint_scene` path).

## Changed files

- `native/client/render_list.hpp` — Orb, Quickbar, Minimap ops
- `native/client/main.cpp` — HUD painters, safe zones, scenario asserts
- `orchestration/tasks/TASK-0076-native-hud-parity/captures/*`

## Verification

`powershell -File native/build.ps1 -RunTests -RunClientScenarios` (2026-08-20,
exit 0): all unit/session tests PASS; all 7 client scenarios PASS including
Orb/Quickbar/Minimap asserts on `move-and-camera` and `remote-render-list`;
`telegraph-dodge` still PASS (HUD reserve).

Reference scene 02 captures committed:
`hud-parity-1920x1080.png`, `hud-parity-1366x768.png`,
`hud-parity-render-list.json` (336 ops with Floor/Tile/Orb/Quickbar/Minimap).

Architect: play pass vs browser benchmark sxs-02/05.

## Deviations

None. No new art files. No `native/src` edits.
