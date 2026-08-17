---
id: TASK-0028
title: Native client — scenery billboards and catalog adoption
state: READY
track: native
priority: medium
base_commit: current program tip (coordinator records the SHA)
dependencies: [TASK-0016, TASK-0015]
parallel_safe: true
owned_paths:
  - native/client/**
forbidden_paths:
  - native/src/**
  - native/include/**
  - native/CMakeLists.txt
  - native/build.ps1
  - native/tools/**
  - src/**
  - server/**
  - prototypes/**
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests -RunClient
---

## Goal

Two coherent client improvements in one pass:

1. **Scenery billboards**: the lab arena gains grounded scenery from the
   existing keyed-plate loader (TASK-0016) — `tree`, `ruin`, `dwelling`,
   `shrine` plates (read-only from `prototypes/founding-slice/assets/`),
   placed deterministically (seed from route id), depth-sorted with the
   actors, contact shadows, simple circle colliders matching the slice's
   pattern, capsule/absent fallback intact.
2. **Catalog adoption**: the skill strip and telegraph geometry read
   `Simulation`'s `PresentationCatalog` (TASK-0015) instead of the
   client-side mirrored constants (0009/0013 watch items) — delete the
   mirrors.

## Invariants

Simulation untouched; headless byte-identical; assets read-only with
graceful fallback; D-107 camera unchanged.

## Acceptance criteria

- Gates green; headless unchanged.
- Driven pass captures (task folder, lossy): scenery visible and
  depth-sorting correctly against actors (walk behind/in front of a
  tree), collision blocking movement through a dwelling, and the skill
  strip still correct (costs from catalog).
- `grep` proof in REPORT.md that the old mirrored constants are gone from
  the client.

## Review focus

Depth-sort correctness at plate boundaries, collider feel, catalog
single-sourcing.

## Stop conditions

Any need for core exports beyond the existing catalog → question, not
code.
