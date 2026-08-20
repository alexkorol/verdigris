---
task: TASK-0076
title: Native HUD parity — orbs, iconed quickbar, minimap
state: READY
packet: BOUNDED-DESIGN
lane: any client-capable lane (PIPELINED-soft: rebase over 0075 if it lands first)
priority: high (D-124 — second-largest side-by-side delta)
owned_paths:
  - native/client/**
  - orchestration/tasks/TASK-0076-native-hud-parity/**
forbidden_paths:
  - native/src/**
  - prototypes/** (existing assets only; procedural drawing is fine)
---

# Outcome (benchmark: sxs-02/05 — browser HUD vs native text bars)

1. Life/Resource as circular ORBS (bottom corners, browser-style):
   procedural GDI fill-level rendering (red/blue radial), numeric
   readout, low-life pulse. No new art files.
2. Quickbar: slot boxes bottom-center with key labels and cooldown
   sweep overlays (existing Q/E/R + LMB slots), visually grouped like
   the browser quickbar.
3. Minimap: top-left panel rendering the known WorldView — walls/
   scenery dots, monsters red, player arrow, extraction marker gold.
   Works in remote mode from the model (no Simulation reads).
4. HUD safe zones respected (0068 rule); all elements in render-list
   as ops so scenarios can assert them.

# Acceptance

Gates green + render-list asserts for Orb/Quickbar/Minimap ops +
capture at 1920x1080 and 1366x768 + architect play pass compares
against the browser side of the benchmark.
