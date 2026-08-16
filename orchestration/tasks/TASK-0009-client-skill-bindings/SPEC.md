---
id: TASK-0009
title: Bind Q/E/R to the core skill actions in the native client
state: READY
track: native
priority: medium
base_commit: 0c51439
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

READY (promoted 2026-08-16; base 0c51439 = wave-3 integration containing
Thrust/Sweep/WarCry and the D-007 client).

Additional promotion notes: Q → Thrust, E → Sweep, R → WarCry. Skill strip
shows name + resource cost, greys while on cooldown or unaffordable;
resource bar joins the HUD (life already shown); render Sweep as a
full-circle arc effect and WarCry as a brief aura ring using the existing
procedural effect vocabulary. Thrust's +x facing limitation (0007 REVIEW
observation) is accepted for now — do not try to fix facing client-side.
Acceptance adds a PostMessage driven pass proving each key dispatches its
action, resource drains and regenerates on the HUD, and Q/E/R no longer
show the disabled hint.

## Goal (outline)

Replace the disabled Q/E/R placeholders (TASK-0004) with real bindings to
`Thrust` / `Sweep` / `WarCry` (TASK-0007): skill strip shows name, cost,
cooldown/buff state; resource bar appears in the HUD; existing procedural
effects render Sweep as a full-circle arc and WarCry as a brief aura ring.
Verification mirrors TASK-0004: build gate + PostMessage driven pass
proving each key dispatches its action and the HUD reflects resource
spend/regen. Sequential with any other client task.
