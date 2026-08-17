---
id: TASK-0038
title: LMB/RMB attacks + key/mouse rebinding UI
state: READY
track: web-demo
priority: critical
base_commit: current program tip (coordinator records the SHA)
dependencies: [TASK-0037]
parallel_safe: false
owned_paths:
  - src/core/config/controls.js
  - src/core/utilities/input-controller.js
  - src/components/ui/Settings*.vue
  - src/core/player/**
  - tests/unit/controls*.spec.js
  - orchestration/tasks/TASK-0038-combat-controls-rebinding/captures/**
forbidden_paths:
  - server/**
  - native/**
  - prototypes/**
  - package.json
acceptance_commands:
  - npm run test:unit
  - npm run smoke:browser
---

## Goal

Owner directive: (1) LMB = primary attack and RMB = secondary
attack/skill in the world (aim-at-cursor, per D-007's contract — clicks
on UI still operate UI; a click on a target or ground attacks); (2) a
rebinding UI in settings — every skill/action key and the two mouse
buttons remappable, persisted client-side, with reset-to-default.

## Scope

1. Input layer: default binding map (D-007 shape: LMB primary, RMB
   skill, Space dodge/dash, Q/E/R/F etc.), conflict detection (no two
   actions on one binding), persistence (localStorage), live apply.
2. World-click semantics: LMB/RMB in the playfield attack toward cursor
   (server-authoritative resolution as today); context-menu access moves
   to a modifier or keeps右-click-hold if needed — document the choice
   and keep the existing context menu reachable somehow.
3. Settings pane: a bindings section listing actions, click-to-rebind,
   capture next key/button, ESC cancels, reset-all.
4. Skill bar reflects current bindings on its labels.

## Acceptance criteria

Gates green; captures of the rebinding UI, a completed rebind surviving
reload, and LMB/RMB attacks landing; architect plays it (D-115) —
attacking must feel immediate.

## Stop conditions

Gamepad support, server protocol changes, native client changes.
