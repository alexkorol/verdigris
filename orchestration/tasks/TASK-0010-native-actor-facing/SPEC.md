---
id: TASK-0010
title: Real actor facing replaces the Thrust +x proxy
state: READY
track: native
priority: high
base_commit: TASK-0009 integration tip (Codex records the actual SHA in STATUS.md on claim)
dependencies: [TASK-0009]
parallel_safe: false
owned_paths:
  - native/src/**
  - native/include/**
  - native/tests/**
  - native/client/**
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

Actors have an explicit facing the simulation owns, set by movement and an
aim command; Thrust hits along facing instead of only +x, and the client's
mouse aim feeds it.

## Why this task exists

TASK-0007's Thrust uses the horizontal-delta sign as a facing proxy — a
monster behind the player is unhittable. D-007 makes mouse aim a core
control; aim must therefore be a simulation input, not a client fiction.
This task deliberately owns BOTH core and client paths (sequential, no
other task in flight) so the seam lands coherently.

## Product and architectural invariants

- D-002: facing is deterministic simulation state. The client sends an aim
  command; it never computes hits.
- D-003: facing lives on `Actor` (any kind); monsters get facing from
  their movement/AI, players from aim/movement.
- Command-stream compatibility: append a new `CommandType` (suggested
  `AimIntent` with dx/dy); do not reorder existing enum values.
- Integer math only. Facing can be a normalized direction pair or an
  8-way/16-way quantized heading — pick the simplest deterministic
  representation and document it; do NOT introduce floats into core state.

## Inputs and references

`native/src/core.cpp` (`resolve_actor_action` Thrust branch,
`resolve_move`, `enemy_turn`), `core.hpp`, `native/client/main.cpp`
(mouse aim already computes a world-space angle), TASK-0007 REVIEW
observation.

## Scope

1. Add facing state to `Actor` (default +x for compatibility) and a
   command (e.g. `Command::aim(dx, dy)`) that sets the player's facing
   without consuming a turn beyond the normal dispatch tick.
2. `resolve_move` updates facing from the move direction (when moving);
   aim overrides between moves. Monsters face their pursuit target in
   `enemy_turn`.
3. Thrust's target filter accepts candidates within a forward half-plane
   (or quantized cone) of the attacker's facing instead of `delta_x >= 0`.
   Keep Melee/Sweep omnidirectional as today.
4. Client: throttle-send aim (e.g. only when the aimed direction's
   quantized heading changes) so command streams stay small; remove the
   "+x limitation" caveat from the skill strip if present.
5. Tests: facing follows movement; aim command overrides; Thrust hits a
   target behind the player after aiming at it and misses a target outside
   the cone; monster facing tracks the player; replay determinism remains
   byte-equal; all existing tests green.

## Non-goals

Projectiles, animation, strafe mechanics, client-side prediction.

## Deliverables

Code + tests, one coherent commit (core and client may be two commits if
cleaner).

## Acceptance criteria

`build.ps1 -RunTests -RunClient` exits 0; the scope-5 behaviors each have
a named test; a brief driven-input note in REPORT.md confirms aiming left
then Q hits a monster to the player's left.

## Required verification

```powershell
powershell -File native/build.ps1 -RunTests -RunClient
```

plus the driven-input check above.

## File ownership

Core + tests + client (sole in-flight task; sequential by design).

## Dependencies

TASK-0009 integrated.

## Parallel-safety assessment

NOT parallel-safe (owns core and client simultaneously). Nothing else may
be in flight while this runs.

## Review focus

Determinism of the facing representation, cone/half-plane math in integer
arithmetic, command-stream compatibility, and that the aim throttle
doesn't starve facing updates.

## Stop conditions

Any need for floating-point core state or for redefining existing
commands → stop and file a question.
