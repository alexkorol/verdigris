---
id: TASK-0042
title: The first drop feels like finding something
state: READY
track: web-demo
priority: high
base_commit: current program tip (coordinator records the SHA)
dependencies: [TASK-0040]
parallel_safe: false
owned_paths:
  - server/core/combat/loot.js
  - server/core/data/items/verdigris.js
  - src/core/player/events/**
  - src/components/ui/Loot*.vue
  - tests/unit/loot*.spec.js
  - orchestration/tasks/TASK-0042-first-loot-moment/captures/**
forbidden_paths:
  - native/**
  - prototypes/**
  - server/core/items/vesselforge/**
  - package.json
acceptance_commands:
  - npm run test:unit
  - npm run playtest
  - npm run smoke:browser
---

## Goal

Close 0034 major 4: the first item drop of a session is a MOMENT — a
named, visually highlighted drop with a pickup affordance and a one-line
"why it matters" comparison against what you hold, drawn from the
curated Verdigris item data (KEEP-as-data per LEGACY_MATRIX). Loot
excitement is a constitution pillar (§3.3).

## Scope

1. Guarantee the first delve's early kills yield one curated item drop
   (deterministic first-drop rule, named constant), using EXISTING item
   data — no new item design.
2. Drop presentation: ground highlight/beam + name label (the slice and
   the 2.5D stack already have the vocabulary), pickup prompt, and on
   pickup a compact toast/inspect with the stat comparison line.
3. Sound cue if the existing audio seam supports it cheaply (do not
   build an audio system).
4. Tests: first-drop rule; comparison content; playtest loot scenarios
   updated deliberately.

## Acceptance criteria

Gates green; captures of the drop moment and comparison; D-115 play
gate — I should feel the beat.

## Stop conditions

New items/affixes/Vesselforge changes (owner domain); economy changes.
