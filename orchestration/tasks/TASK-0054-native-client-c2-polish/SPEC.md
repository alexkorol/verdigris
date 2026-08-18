---
task: TASK-0054
title: Native client C2 — polish pass (title fix, combat juice, zoom)
state: READY
priority: medium (MECHANICAL packet; owner-visible)
owned_paths:
  - native/**
  - orchestration/tasks/TASK-0054-native-client-c2-polish/**
forbidden_paths:
  - playtest/**, server/**, src/**
base: current program tip (after 0051 integration)
architect_review_required: true
---

## Deliverables (all small, all owner-visible)

1. Inventory pane title: "House House Verdigris" → "House Verdigris"
   (0050 review nit 1).
2. Combat juice: hit-impact flash on the TARGET sprite (brief tint),
   damage numbers rise/fade over ~600ms (currently short-lived), and
   a 150ms screen-edge red pulse when the PLAYER takes damage.
3. Mouse-wheel zoom (clamped 24–96 px/unit around camera2d.zoom;
   Home resets to default) — uniform zoom only, no reintroduction of
   any position-dependent scale (camera2d invariants stay locked).
4. New harness scenarios (D-119, mandatory): combat-juice (render-list
   asserts for target flash + player-damage pulse + number lifetime)
   and zoom-invariance (uniform delta invariant holds at min/default/
   max zoom).

## Acceptance evidence

1. `powershell -File native/build.ps1 -RunTests -RunClientScenarios`
   literal transcript — all PASS including the two NEW scenarios.
2. Headless proof line intact (1|1, exit 0).
3. One authentic negative on a new scenario (suppress the flash →
   fail → restore).

Architect reruns the scenario set and spot-plays the exe (D-117).
