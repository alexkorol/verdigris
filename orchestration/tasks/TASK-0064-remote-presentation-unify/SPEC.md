---
task: TASK-0064
title: Remote mode renders through the real client presentation (Gate A unblock)
state: READY
packet: BOUNDED-DESIGN
lane: cursor suggested (owns single-writer main.cpp tonight)
priority: CRITICAL - Gate A is red solely on this
owned_paths:
  - native/client/**
  - orchestration/tasks/TASK-0064-remote-presentation-unify/**
forbidden_paths:
  - native/src/** (0063 owns server; file notes for gaps)
  - playtest/**
---

# Outcome (0061 review correction, numbered)

1. One presentation, two sessions: the remote window must render
   through the SAME pipeline as local play - render_list painter,
   billboards, combat effects, damage numbers, telegraph rings, HUD
   panes, camera2d - fed from ClientModel + PresentationEvents instead
   of Simulation. The 0061 debug painter (dot/squares/text log) is
   replaced, not polished.
2. Local mode must not regress: all D-119 scenarios stay green.
3. Where the remote model lacks data the painter needs (e.g. monster
   positions until 0063 snapshot work lands), render what the model
   has and leave a note - do NOT read from a local Simulation in
   remote mode, ever.
4. Extend session tests: a remote render_list assertion scenario
   (connect, fight, assert Monster/Swing/Drop ops appear in the render
   list) so presentation parity is machine-checked, not just eyeballed.

# Acceptance

build.ps1 -RunTests -RunClientScenarios green + remote render-list
scenario green + architect PLAYS --remote and rescores the Gate A
rubric (target: no zeroes, >=9/12). Quality bar: the remote window
should be visually indistinguishable from local play wherever the
model carries the data.
