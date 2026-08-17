---
id: TASK-0040
title: First-encounter readability — the fight a new player can learn
state: READY
track: web-demo
priority: critical
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - server/core/monsters/**
  - server/core/data/monsters/**
  - server/core/combat/**
  - server/core/map.js
  - tests/unit/combat*.spec.js
  - tests/unit/encounter*.spec.js
  - orchestration/tasks/TASK-0040-first-encounter-readability/captures/**
forbidden_paths:
  - src/**
  - native/**
  - prototypes/**
  - server/core/services/**
  - package.json
acceptance_commands:
  - npm run test:unit
  - npm run playtest
---

## Goal

Close 0034 blocker 1 and major 3 server-side: the FIRST hostile
encounter of the forgiving first delve becomes learnable — one readable
melee target first, no ranged pressure until the player has landed
kills, no enemy stacking on the player's tile, and an escalation curve
that rewards each kill before raising pressure.

## Authority

`orchestration/tasks/TASK-0034-playability-evaluation/REPORT.md`
(blockers/major 3) + OWNER-SEED. Constitution §readability. D-114 for
any range/spacing constants (table required).

## Scope

1. Author the first-delve encounter composition: opening enemy is a
   single melee actor with existing telegraph-capable behavior; ranged
   (Ashen Marksman class) enters only after N kills (named constant);
   pack sizes capped early.
2. Enemy separation: enemies may not stack/overlap the player's
   position (server-side spacing akin to native separation logic).
3. Escalation: a simple per-delve pressure curve (named constants
   table) so five minutes has shape: learn → win → bigger wave → reward.
4. Tests: composition rules (first enemy melee, ranged gated),
   separation invariant, curve progression; playtest 31/31 (update
   encounter-variety expectations deliberately if the new composition
   invalidates them — justify).

## Non-goals

Client telegraph/hit-feedback visuals (they exist from the phases; if a
client change is genuinely required, STOP and file a question), loot
changes (0042), death screen (0041), lore/naming (owner).

## Acceptance criteria

Gates green; a driven first-delve transcript in REPORT.md showing the
composition (first contact melee-only, ranged delayed, no overlap
events); constants table per D-114.

## Review focus (D-115)

Architect plays the first delve personally: can I learn the fight
without dying to invisible pressure?

## Stop conditions

Client-side needs; new monster types (compose from existing roster);
anything lore-flavored.
