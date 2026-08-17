---
id: TASK-0035
title: Native exe triage — stop presenting as a game (D-112)
state: READY
track: native
priority: high
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - native/client/**
  - native/src/**
  - native/include/**
  - native/tests/**
forbidden_paths:
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

The owner played the exe and found it "a total mess." Two verified root
causes get fixed; the rest is honest re-labeling per D-112 (the exe is a
core testbed, not the game).

## The verified defects (owner-reported, architect-confirmed)

1. **Combat ranges are absurd relative to movement** ("enemies attack
   from very far away"): TASK-0017 rescaled movement to ~11 units/tick
   but `kMeleeRange=1100`/`kThrustRange=1650` were left — melee reach is
   now a ~5-second walk. Apply D-114: build ONE constants table deriving
   every distance from seconds-to-contact at the current speed
   (melee contact ≈ 0.5–0.8s of walking ≈ 120–180 units; thrust ~1.5×;
   extraction pad, spawn distance, scenery colliders, arena scale all
   re-derived together and documented in the diff + REPORT). Update
   tests to the new table deliberately.
2. **UI obstruction** ("text and buttons covering the game world"):
   ALL debug scaffolding — help lines, camera line, event log, debug
   counters — moves behind an F3 toggle, DEFAULT OFF. Default view shows
   ONLY: compact life/resource bars (corner), the three-slot skill
   strip (bottom), and nothing else over the playfield. Window title
   becomes "Verdigris Core Testbed" (D-112 honesty).

## Non-goals

Building game UI (inventory panes, orbs, menus — that belongs to the
browser product per D-112), art changes (D-113 is a separate decision),
new features of any kind.

## Acceptance criteria

- Gates green; headless output unchanged.
- D-115 play gate: the architect will PLAY the build before acceptance —
  the driven evidence must include a capture of the default clean view
  (no debug text) and a fight where the enemy closes to visually-
  adjacent range before striking.
- The constants table present in REPORT.md with seconds-to-contact
  derivations.

## Stop conditions

Scope growth toward "make it a real game" — that is explicitly not this
task.
