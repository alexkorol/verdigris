---
id: TASK-0025
title: Instance lifecycle correctness — pickup boundary and pack-clear rules
state: READY
track: native
priority: high
base_commit: after TASK-0017 integrates (coordinator records the SHA)
dependencies: [TASK-0017]
parallel_safe: false
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

Fix the two core defects found by the PR #4 automated review (both
architect-verified as real):

1. **Pickup boundary**: `resolve_pickup` accepts stale IDs from previous
   instances because extraction/route-entry never clears
   `ground_items_`/`ground_trophies_` and pickup checks no
   active-instance membership. Post-extraction or cross-route pickups
   bypass the expedition/extraction boundary (violates the constitution's
   unextracted-value rule).
2. **Pack-clear**: `handle_death` calls
   `clear_route_and_unlock_children()` on EVERY monster death; with a
   pack via `spawn_monster`, the first kill clears the route and can mark
   the campaign complete mid-encounter.

## Scope

1. Pickup requires `instance_.active` AND membership of the ID in the
   active instance's ground lists; leaving an instance (extract, death,
   enter) retires that instance's ground state (items left behind are
   LOST — consistent with extraction risk; relic-pool items got there via
   death, not via floor-leftovers — document the exact retirement
   semantics in REPORT.md).
2. `clear_route_and_unlock_children()` fires only when no living monster
   remains in the instance; `drop_reward` per-kill behavior is unchanged.
3. Tests: post-extraction pickup rejected; cross-route stale pickup
   rejected; leftover ground items do not survive re-entry; two-monster
   pack does not clear the route until the second dies (and campaign
   completion timing likewise); existing suites green; replay
   determinism.

## Non-goals

Client changes, loot-decay design, multi-instance simultaneity.

## Acceptance criteria

Gates green with the scope-3 tests named in REPORT.md.

## Review focus

Retirement semantics vs relic/death paths (no double-registration or
loss of relic-pool invariants), and pack-clear interaction with
telegraphs/windups.

## Stop conditions

If retirement semantics collide with the death path's item registration
in a way requiring a product call, stop and file a question.
