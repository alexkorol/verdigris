---
task: TASK-0068
title: Remote presentation polish (0064 rubric notes)
state: READY
packet: BOUNDED-DESIGN
lane: cursor suggested (client)
priority: medium (Gate A green; raises 10/12 toward 12/12)
owned_paths:
  - native/client/**
  - orchestration/tasks/TASK-0068-remote-presentation-polish/**
forbidden_paths:
  - native/src/** (notes for server gaps)
  - playtest/**
---

# Outcome (from the 0064 architect review, numbered)

1. Telegraph rings never overlap the HUD safe zones (clamp or fade FX
   entering the HUD reserve).
2. Monster billboards visually distinct from the player at a glance
   (silhouette/tint/size using EXISTING art assets only — new asset
   decisions are owner domain).
3. Extraction affordance: stairs pad / extraction point visibly marked
   in-world (not hint-bar-only); add a render-list assertion for it.
4. Connection state chip on the remote HUD (connecting / ready /
   disconnected — reuse connection_state_label).

# Acceptance

build.ps1 -RunTests -RunClientScenarios green + updated remote
render-list scenario + architect play pass rescoring the Gate A rubric
(target 12/12).
