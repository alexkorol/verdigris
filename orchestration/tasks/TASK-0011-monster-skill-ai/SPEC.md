---
id: TASK-0011
title: Monsters use skill actions with telegraphed windups
state: READY
track: native
priority: high
base_commit: TASK-0010 integration tip (Codex records the actual SHA in STATUS.md on claim)
dependencies: [TASK-0010]
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

Elite monsters use Thrust and Sweep through the same
`resolve_actor_action` pipeline as players, with a telegraph event emitted
a fixed number of ticks before the strike so presentation can render
readable warnings.

## Why this task exists

D-003 promised actor symmetry as behavior, not just schema; TASK-0007
built the shared pipeline and this makes monsters actually walk through
it. Constitution §3.9 requires readable telegraphs; the event must come
from the simulation so any client renders the same truth.

## Product and architectural invariants

- D-002/D-003: deterministic, rule-based (no new RNG unless from `rng_`,
  and prefer none); skills resolve via `resolve_actor_action` — zero
  duplicated combat math.
- Telegraph contract: emit `AttackTelegraphed` (append to EventType)
  carrying actor id, action name in `text`, and the windup ticks in
  `value`, exactly `kTelegraphTicks` (named, suggested 3) ticks before the
  action resolves. The pending action must be cancelled if the monster
  dies or the target dies during windup.
- Non-elite monsters keep today's plain melee cadence unchanged (existing
  tests must stay green without edits unless an expectation is genuinely
  invalidated — justify any test change in REPORT.md).

## Inputs and references

`enemy_turn`, `resolve_actor_action`, `Actor::facing/elite`, TASK-0007 and
TASK-0010 diffs.

## Scope

1. Pending-action state on Actor (action + ticks remaining) driven from
   `enemy_turn` for elite monsters: rule — if the player is in the Thrust
   cone at thrust range, telegraph Thrust; else if within melee range,
   telegraph Sweep when resource suffices, else plain melee (no
   telegraph for plain melee, preserving current feel).
2. Resolution consumes the same resource/cooldown gates as players; if
   gates fail at resolution time the action fizzles (no effect, no crash).
3. Tests: telegraph precedes damage by exactly the constant; elite uses
   Thrust only when the cone rule holds; Sweep hits the player via the
   shared pipeline; cancellation on death mid-windup; determinism replay;
   non-elite behavior unchanged.

## Non-goals

Client rendering of telegraphs (a follow-up client task), new monster
kinds, tuning.

## Deliverables

Code + tests, one coherent commit.

## Acceptance criteria

`build.ps1 -RunTests` exits 0 with the scope-3 tests named in REPORT.md.

## Required verification

```powershell
powershell -File native/build.ps1 -RunTests
```

## File ownership

Core sources/tests only.

## Dependencies

TASK-0010 integrated.

## Parallel-safety assessment

Parallel-safe with TASK-0012 (prototypes-only). No other core task in
flight.

## Review focus

Windup bookkeeping in `advance_tick` vs `enemy_turn` ordering, fizzle
semantics, and that plain-melee cadence for non-elites is bit-identical to
before.

## Stop conditions

Any need to touch the client, or ambiguity about which monsters count as
elite → stop and file a question.
