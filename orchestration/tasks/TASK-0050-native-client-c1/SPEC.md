---
task: TASK-0050
title: Native client wave C1 — clean 2D top-down, visible combat, real inventory
state: READY
priority: critical (D-117/D-118; the owner plays this)
owned_paths:
  - native/**
  - orchestration/tasks/TASK-0050-native-client-c1/**
forbidden_paths:
  - playtest/**, server/**, src/** (browser untouched)
base: current program tip (>= 34b7069f)
architect_review_required: true
---

## Why

The owner played the testbed: scenery slides against movement (broken
billboard projection), attacks are invisible, inventory is a stub.
D-118 rules the fix: drop the broken 2.5D projection for a clean 2D
top-down NOW; correct projection returns later as its own wave.

## Deliverables (owner-visible, all three)

1. **2D top-down presentation.** Remove/disable the billboard parallax
   path entirely — one consistent orthographic top-down camera. Tiles,
   actors, scenery, and drops all move coherently with the camera. No
   element may move against player motion. Keep the existing sprite
   assets (draw them flat); readability over spectacle.
2. **Visible combat.** Render what the core already simulates: attack
   swings/projectiles for player skills (D-007 six-skill kit), boss/
   monster telegraph circles (radius + windup from N3 data), hit
   flashes and floating damage numbers, monster death removal, drop
   spawn visibility. A player watching the screen must be able to
   answer: did my attack land, what hit me, what died, what dropped.
3. **Real inventory pane.** Grid inventory + equip slots rendered from
   the core's authoritative state: pickup adds visibly, equip/unequip
   works via the pane, equipped items affect the stats readout,
   extraction shows what was banked. Match the browser game's
   inventory CONCEPTS (grid, paperdoll seats) at native-testbed
   fidelity — not pixel parity.

## Constraints

- Simulation stays deterministic/headless-independent (native
  boundary): presentation reads state and emits commands, never owns
  rules.
- The `--headless` proof loop must still exit 0 with
  `trophies stored: 1 | items stored: 1`.
- D-114 constants remain authoritative for ranges/TTK.

## Acceptance evidence

1. `powershell -File native/build.ps1 -RunTests -RunClient` literal
   transcript (all PASS lines + headless proof).
2. Short scripted demo: input script (or documented manual sequence)
   showing kill → visible swing/telegraph/damage → drop → pickup into
   the pane → equip → extract. Screenshots or a capture of each beat.
3. Statement of what was REMOVED (the projection path) so the later
   2.5D wave starts clean.

The architect will build and PLAY the exe before ACCEPTED (D-117).
