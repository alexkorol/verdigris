---
id: TASK-0015
title: Core-owned presentation catalog (skill costs, ranges, telegraph data)
state: READY
track: native
priority: high
base_commit: current program tip (Codex records the actual SHA in STATUS.md on claim)
dependencies: [TASK-0011]
parallel_safe: true
owned_paths:
  - native/src/**
  - native/include/**
  - native/tests/**
forbidden_paths:
  - native/client/**
  - native/CMakeLists.txt
  - native/build.ps1
  - native/tools/**
  - src/**
  - server/**
  - prototypes/**
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests
---

## Goal

The simulation exposes a read-only catalog of the gameplay constants
presentation legitimately needs — skill resource costs, melee/thrust
ranges, telegraph windup ticks, war-cry duration — so clients stop
mirroring core constants by value (the drift risk flagged in the 0009 and
0013 reviews).

## Why this task exists

Two accepted client tasks now hardcode copies of core numbers. The next
rebalance silently makes the UI lie. The authority boundary (D-002) says
presentation asks, simulation answers.

## Scope

1. A `PresentationCatalog` (or similarly named) plain-struct value,
   obtainable from `Simulation` (static or const accessor), containing:
   per-skill resource cost, melee range, thrust range, telegraph ticks,
   war-cry bonus and duration, resource regen per tick. All fields sourced
   from the same named constants the mechanics use (single definition —
   move constants to a shared home if needed; do not duplicate).
2. Tests: catalog values match observed behavior (e.g., the cost the
   catalog reports is exactly what a Thrust deducts; the telegraph tick
   count matches the event's value field); catalog is stable across a run.
3. Do NOT change any gameplay values.

## Non-goals

Client adoption (follow-up client task), serialization, localization/lore
naming.

## Acceptance criteria

`build.ps1 -RunTests` green with the scope-2 tests; a grep in REPORT.md
proving each catalog field has exactly one defining constant.

## File ownership / parallel-safety

Core+tests only; parallel-safe with TASK-0017 (client-only).

## Review focus

Single-source-of-truth discipline; no behavior change (existing tests
untouched).

## Stop conditions

Temptation to expose internals beyond the listed fields → stop; the
catalog is a curated contract, not a state dump.
