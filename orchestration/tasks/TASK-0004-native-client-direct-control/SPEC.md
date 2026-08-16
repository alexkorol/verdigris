---
id: TASK-0004
title: Native client direct-control pass per D-007 contract
state: DRAFT
track: native
priority: high
base_commit: TBD (set on promotion; after TASK-0001 integrates)
dependencies: [TASK-0001, TASK-0002]
parallel_safe: false
owned_paths:
  - native/client/**
forbidden_paths:
  - native/src/**
  - native/include/**
  - src/**
  - server/**
  - prototypes/**
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests -RunClient
---

DRAFT — do not claim. Will be promoted with a fixed base_commit after wave 1
review. Outline for planning:

## Goal

Bring `native/client/main.cpp` to the D-007 control contract
(`orchestration/DECISIONS.md`): LMB primary / RMB weapon skill / Space
dodge / Q,E,R skill slots (E is NOT equip) / X nearest pickup / Z loot
filter toggle / F contextual interact (extraction) / I gear readout.
Equip moves out of world keys.

## Notes for the eventual spec

- The core currently exposes only Melee/Dash/Wait actions; the client task
  must NOT extend the simulation (that is a separate native-core task this
  architect will spec after reviewing TASK-0001). Until then, unbound skill
  slots render as disabled placeholders in the debug overlay.
- Sequential with any other client-touching task; depends on build guard
  from TASK-0002 so the windowed define cannot regress silently.
- Verification: build.ps1 gate plus a driven-input PostMessage/PrintWindow
  pass equivalent to the one recorded in HANDOFF (2026-08-15 later entry).

## Stop conditions (will carry into the promoted spec)

- Any temptation to add simulation actions from the client side → stop.
