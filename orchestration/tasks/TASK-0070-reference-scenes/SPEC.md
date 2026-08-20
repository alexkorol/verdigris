---
task: TASK-0070
title: Native visual reference scenes — Stage 1 readability baseline
state: READY
packet: BOUNDED-DESIGN
lane: cursor suggested (client)
priority: high (D-122 axis 3 — feeds the renderer decision)
owned_paths:
  - native/client/**
  - native/tools/** (capture driver)
  - orchestration/tasks/TASK-0070-reference-scenes/**
forbidden_paths:
  - native/src/** (notes for gaps)
  - playtest/**
---

# Outcome (convergence doc, Stage 1)

Five FIXED-SEED reference scenes, reproducibly capturable at 1920x1080
and 1366x768 via a driver flag (`--reference-scene <n>` presenting into
an offscreen DC and writing PNG/JPG):

1. Route entrance / surface.
2. Normal pack combat (2+ monsters, mid-swing).
3. Elite telegraph moment.
4. Named item drop with the gear pane open.
5. Critical-health state (life < 25%, screen pulse active).

Each scene = deterministic setup (seeded local session; scripted
command sequence). Commit the 10 captures + a render-list JSON dump
per scene. These are the BEFORE baseline for every future renderer/
art decision — owner reviews them, then they freeze.

# Non-goals

No new art, palette, or scale decisions (owner domain — D-113); this
records what exists today, readably.

# Acceptance

build.ps1 gates green + `--reference-scene all` exits 0 + the 10
captures committed + scene setups deterministic (two runs, identical
render-list JSON). Architect reruns and eyeballs one scene per
resolution.
