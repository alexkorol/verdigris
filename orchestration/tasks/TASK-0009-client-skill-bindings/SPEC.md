---
id: TASK-0009
title: Bind Q/E/R to the core skill actions in the native client
state: DRAFT
track: native
priority: medium
base_commit: TBD (set on promotion after TASK-0007 integrates)
dependencies: [TASK-0007, TASK-0004]
parallel_safe: false
owned_paths:
  - native/client/**
forbidden_paths:
  - native/src/**
  - native/include/**
  - native/CMakeLists.txt
  - native/build.ps1
  - src/**
  - server/**
  - prototypes/**
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests -RunClient
---

DRAFT — do not claim until READY with a SHA base_commit.

## Goal (outline)

Replace the disabled Q/E/R placeholders (TASK-0004) with real bindings to
`Thrust` / `Sweep` / `WarCry` (TASK-0007): skill strip shows name, cost,
cooldown/buff state; resource bar appears in the HUD; existing procedural
effects render Sweep as a full-circle arc and WarCry as a brief aura ring.
Verification mirrors TASK-0004: build gate + PostMessage driven pass
proving each key dispatches its action and the HUD reflects resource
spend/regen. Sequential with any other client task.
