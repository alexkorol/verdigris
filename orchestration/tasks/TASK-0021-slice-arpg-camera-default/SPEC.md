---
id: TASK-0021
title: Slice defaults to the ARPG camera with zoom-in Miniature blend (D-107)
state: READY
track: web-demo
priority: medium
base_commit: current program tip (coordinator records the SHA in STATUS.md)
dependencies: []
parallel_safe: true
owned_paths:
  - prototypes/founding-slice/slice.html
  - prototypes/founding-slice/index.html
forbidden_paths:
  - prototypes/founding-slice/build.mjs
  - prototypes/founding-slice/assets/**
  - prototypes/founding-slice/tests/**
  - prototypes/founding-slice/run-checks.mjs
  - native/**
  - src/**
  - server/**
acceptance_commands:
  - node prototypes/founding-slice/build.mjs
  - node prototypes/founding-slice/run-checks.mjs
---

## Goal

The founding slice boots with the owner-ruled D-107 camera: ARPG preset
values as the default `cam` state, and wheel zoom-in blends toward the
Miniature treatment (perspective/tilt rise as zoom passes ~1.05, linear,
named constants). High Table remains available in the lab but is no
longer promoted.

## Scope

1. Edit the default `cam` object in `slice.html` to the ARPG values
   (pitch 62, zoom .85, persp .0006, anchor .52, fog .4, tilt 0).
2. Add the zoom→perspective/tilt blend in the wheel handler (clamped,
   reversible, constants named).
3. Rebuild `index.html` via `build.mjs` (the committed output must match —
   the drift guard in run-checks enforces this).

## Acceptance criteria

Both commands exit 0 (harness 4/4 including drift guard). A short capture
or parameter dump in REPORT.md showing boot defaults = ARPG and the blend
engaging at high zoom.

## Review focus

Drift guard passes; blend is subtle and reversible; presets in the lab
still function.

## Stop conditions

Any need to touch the harness or build script → stop.
