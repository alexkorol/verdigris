---
id: TASK-0041
title: Death becomes a decision moment
state: READY
track: web-demo
priority: critical
base_commit: current program tip (coordinator records the SHA)
dependencies: [TASK-0036]
parallel_safe: false
owned_paths:
  - src/components/**
  - src/core/player/events/**
  - server/player/handlers/**
  - tests/unit/death*.spec.js
  - orchestration/tasks/TASK-0041-death-decision-moment/captures/**
forbidden_paths:
  - native/**
  - prototypes/**
  - server/core/combat/**
  - package.json
acceptance_commands:
  - npm run test:unit
  - npm run playtest
  - npm run smoke:browser
---

## Goal

Close 0034 blocker 2: when HP hits 0, the world pauses for the player
and a death screen states plainly — what was lost, what is protected
(D-106 recovery pools), what the mortal-oath consequence is (if oathed),
and what happens next — with an explicit continue action. No more
"HP 0 and the world just keeps going."

## Scope

1. Server emits a death-summary payload (loss list, recovered-to-pool
   list, oath status, respawn destination) at the death event — data the
   D-106 implementation (TASK-0032) already computes; expose, don't
   recompute.
2. Client death overlay: full-screen, readable, Bronze-Age-consistent
   with existing UI style; shows the summary; one primary action
   (Return/Continue) and, for mortal deaths, the succession framing the
   Chronicles flow already has — reuse those screens, do not fork them.
3. Input is captured by the overlay (no world control until continue).
4. Tests: overlay triggers on death; summary contents match the D-106
   transfer results; oathed vs unoathed variants; playtest death/respawn
   scenarios updated deliberately.

## Acceptance criteria

Gates green; captures of both death variants; architect D-115 play
gate: die on purpose and judge the moment.

## Stop conditions

New lore text beyond plain mechanical statements (owner domain — use
plain language placeholders); reworking Chronicles succession flow.
