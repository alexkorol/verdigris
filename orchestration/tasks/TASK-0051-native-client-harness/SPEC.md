---
task: TASK-0051
title: Native client test harness — scripted pipeline scenarios (D-119)
state: READY
priority: high (gates all future client waves; pairs with TASK-0050)
owned_paths:
  - native/**
  - orchestration/tasks/TASK-0051-native-client-harness/**
forbidden_paths:
  - playtest/**, server/**, src/**
base: current program tip (>= 34b7069f); coordinate with TASK-0050 if
  both are in flight — disjoint files or sequence C1 first (claimant
  states the plan in STATUS)
architect_review_required: true
---

## Goal (D-119, owner-ruled)

The client gets what the server has: an automated harness that drives
the REAL client loop through scripted scenarios and asserts outcomes,
so client regressions are caught the way playtest catches server
regressions. "Test the whole pipeline as we build."

## Shape

1. **Scenario runner** (`verdigris_client.exe --scenario <name>` or a
   dedicated runner target): feeds a deterministic input script
   (fixed-timestep injected commands) through the real
   input→simulation→presentation pipeline, headless-capable.
2. **Assertions on three layers per scenario:**
   - authoritative core state (positions, HP, inventory, banked items),
   - presentation output (render list / draw calls: "a telegraph
     circle was drawn at X with radius R", "damage number spawned"),
   - HUD/pane state (inventory grid contents, stat readout).
   Render-list assertions make visible-combat regressions testable
   without pixel comparison.
3. **Starter scenario set** (grows with every client wave):
   - move-and-camera: player moves 4 directions; every scenery entity's
     screen delta matches camera delta (would have caught the sliding-
     billboard bug — write this one FIRST),
   - first-fight: approach, attack, assert swing drawn + damage number
     + monster death removal,
   - loot-to-bank: drop visible, pickup fills grid slot, equip changes
     stats, extraction banks (the headless proof loop, promoted),
   - telegraph-dodge: boss telegraph drawn, moving out avoids damage.
4. **Gate wiring**: `native/build.ps1 -RunTests` (or a new
   `-RunClientScenarios` flag wired into CI) runs the scenario set;
   non-zero on any failure.

## Acceptance evidence

1. Literal transcript: all scenarios green at the branch tip.
2. One authentic negative: intentionally break presentation (e.g.
   suppress the swing draw), show the scenario failing, restore.
3. Doc note in native/README.md: how to add a scenario (every future
   client wave must add its own).

The architect will run the scenario set personally before ACCEPTED.
