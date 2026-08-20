---
task: TASK-0075
title: Native ground terrain tiles (kill the flat dark grid)
state: READY
packet: BOUNDED-DESIGN
lane: cursor suggested (client)
priority: CRITICAL (D-124 — largest visual delta in the side-by-side bench)
owned_paths:
  - native/client/**
  - orchestration/tasks/TASK-0075-native-terrain-tiles/**
forbidden_paths:
  - native/src/**
  - prototypes/** (use existing assets only — terrain1.png/terrain4.png
    are ALREADY in prototypes/founding-slice/assets, unused)
---

# Outcome

The world floor renders as textured ground tiles instead of the dark
grid. Evidence: orchestration/benchmarks/side-by-side-2026-08-20/
sxs-02-pack-combat.jpg — the browser side has full terrain texture;
native is flat.

1. Load terrain1.png/terrain4.png through the existing billboard asset
   pipeline (load_sprite); tile the visible floor in world space
   (camera-correct: tiles move with the world, zoom-scaled).
2. Deterministic variation: seed tile choice from tile coords (hash),
   NOT random per frame. Instance theme may bias the mix.
3. Keep a subtle grid/edge treatment only if it aids readability;
   telegraphs/shadows/loot must still dominate (draw order under
   actors and FX).
4. Both modes: local scenarios AND remote (WorldView path). Fallback
   to the current flat fill when assets are missing (same pattern as
   billboards).
5. Update reference scenes: regenerate the 0070 captures (they are the
   BEFORE; commit new AFTER captures alongside, do not overwrite).

# Acceptance

build.ps1 -RunTests -RunClientScenarios green (render-list scenario
extended: Floor/Tile ops present, camera-invariance holds over tiles) +
deterministic two-run capture identity + architect play pass +
side-by-side rerun visibly closes the floor gap.
