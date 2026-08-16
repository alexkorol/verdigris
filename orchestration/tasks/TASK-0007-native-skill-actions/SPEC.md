---
id: TASK-0007
title: Core skill actions — Thrust, Sweep, War Cry
state: READY
track: native
priority: high
base_commit: wave-2 integration tip (Codex records the actual SHA in STATUS.md on claim)
dependencies: [TASK-0006]
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

The simulation exposes three bindable skill actions with mechanically
distinct behavior and resource costs, so the client's disabled Q/E/R slots
(TASK-0004) can become real without the client inventing gameplay.

## Why this task exists

Constitution §3.9: combat vocabulary includes wide swings, thrusts, slams,
war cries. The core currently has only Melee/Dash/Wait; D-007 reserved
Q/E/R for skills. This is the smallest slice that makes builds and the
resource stat meaningful (actor symmetry: monsters can use them later).

## Product and architectural invariants

- D-002 determinism; D-003 actor symmetry — implement on `Actor`, driven
  through the same `resolve_action`/damage pipeline for any actor kind,
  even if only the player dispatches them today.
- Resource (`ActorStats::resource`) pays for skills and regenerates
  deterministically (suggested: +2 per tick up to max, named constant).
  Insufficient resource ⇒ the action does nothing (no partial effects).
- No new RNG streams; reuse `rng_` only if randomness is genuinely needed
  (prefer none).
- Names stay mechanical (`Thrust`, `Sweep`, `WarCry`) — no lore text.

## Inputs and references

`native/src/core.cpp` (`resolve_action`, `enemy_turn`, cooldowns),
`core.hpp` (`ActionType`, `ActorStats`), DECISIONS D-003/D-007,
constitution §3.9.

## Scope

1. Extend `ActionType` with `Thrust`, `Sweep`, `WarCry`.
2. Mechanics (suggested constants; name every number):
   - Thrust: single target within 1.5× melee range in front (use position
     delta sign as facing proxy), 1.3× damage, shares the attack cooldown,
     costs ~10 resource.
   - Sweep: hits EVERY living monster within melee range for 0.75× damage,
     1.5× attack cooldown, costs ~15 resource.
   - WarCry: no damage; grants a flat attack bonus (suggested +4) for a
     tick-bounded duration (suggested 20 ticks) tracked on the actor;
     costs ~20 resource. The bonus feeds the existing damage pipeline the
     same way equipment bonuses do.
3. Emit sensible existing events (`AttackStarted`, `DamageApplied`,
   `ActorDied`) plus a new `BuffApplied`/`BuffExpired` pair (or similarly
   named) for War Cry.
4. Tests: resource gating (too poor ⇒ no-op, no cost), Thrust range/damage
   vs Melee, Sweep multi-target when two monsters are in range (spawn via
   existing seams or a test-visible hook — do NOT special-case production
   code for tests; if a second-monster seam is missing, add a general
   `spawn_monster`-style command/hook that any caller may use), WarCry
   buff application/expiry determinism, replay determinism still
   byte-equal, all existing tests green.

## Non-goals

No client changes, no magic/Lattice content (D-O3), no monster AI usage of
skills yet (structure must allow it), no balance polish.

## Deliverables

Code + tests, one coherent commit on a worker branch.

## Acceptance criteria

`build.ps1 -RunTests` exits 0; each scope-4 behavior has a named test;
existing Legends/relic/replay tests unchanged and green.

## Required verification

```powershell
powershell -File native/build.ps1 -RunTests
```

## File ownership

Core sources/tests only.

## Dependencies

TASK-0006 integrated (same files).

## Parallel-safety assessment

Disjoint from TASK-0008 (tools/config). No other core-file task in flight.

## Review focus

Resource-gating edge cases, cooldown interaction, buff expiry off-by-one,
determinism of the added mechanics, and that Sweep's multi-target seam did
not distort the single-enemy baseline.

## Stop conditions

Needing new player-facing vocabulary beyond the three named skills, or any
client/UI work → stop and file a question.
