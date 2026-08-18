---
task: TASK-0052
title: Harden first-goal and house-treasury against ambient-load flakes
state: READY
priority: medium (MECHANICAL packet — good single-session task)
owned_paths:
  - playtest/scenarios/first-goal.mjs
  - playtest/scenarios/house-treasury.mjs
  - orchestration/tasks/TASK-0052-suite-flake-hardening/**
forbidden_paths:
  - playtest/harness.mjs, playtest/timing.mjs (0043's guard is settled)
  - server/**, src/**, native/**
base: current program tip
architect_review_required: true
---

## Why (WATCH threshold met — 3 sightings)

Under >=150ms ambient event-loop lag, full-suite runs intermittently
fail exactly two scenarios that pass in isolation and in sequence:

1. `first-goal`: "Timed out waiting for Aldwyn names the first-Warden
   objective (8298ms; authored 8000ms)" — the wait already scales but
   the margin is razor-thin under lag.
2. `house-treasury`: "ASSERT FAILED: the scion has carried gold
   available for a House deposit" — a bare assert that reads state
   which arrives slightly late under load; there is no bounded wait in
   front of it.

Sightings: architect 30/32 (2026-08-17 ~19:40, 150ms peak lag),
architect 30/31 pre-0043 (partially firewall-confounded), deepseek
full-suite under self-contention (2026-08-18, 10 failures incl. these).

## Exact work (MECHANICAL)

1. `first-goal.mjs`: convert the objective-naming wait to the same
   bounded load-adaptive retry pattern 0043 used for zone admission
   (use the exported helpers from playtest/timing.mjs — do NOT edit
   timing.mjs itself). Authored deadline stays the floor.
2. `house-treasury.mjs`: in front of the carried-gold assert, add a
   bounded wait (same pattern) for the road-purse/gold state to land;
   the assert itself must remain — only its timing gets slack.
3. No assertion may be weakened or removed; only fixed-instant reads
   become bounded waits.

## Acceptance evidence

1. Literal transcript: both scenarios green solo AND in a full suite
   run under a documented moderate CPU load (0043's spinner method).
2. One authentic negative: temporarily suppress the road purse (or
   objective push) in a scratch copy, show the scenario STILL fails
   (proves asserts weren't weakened), restore.
3. Full default-mode `npm run playtest` green transcript.

Architect reruns the full suite personally before ACCEPTED (G5).
