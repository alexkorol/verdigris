---
id: TASK-0017
title: Continuous movement — fix the fast/jumpy native client feel
state: READY
track: native
priority: critical
base_commit: after TASK-0015 and TASK-0016 integrate (Codex records the SHA in STATUS.md)
dependencies: [TASK-0015, TASK-0016]
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

The native client's character moves at a readable action-RPG pace with
smooth motion — the owner reports it currently "moves very very fast,
jumping around."

## Why this task exists (root cause, verified by architect)

`resolve_move` displaces by `move_speed` (100 units = one full tile) per
MoveIntent, and the client dispatches a MoveIntent every 50ms timer tick →
2000 units/sec in 100-unit teleport steps. The slice's tuned feel
(~220 units/sec continuous) is the reference (D-108 demo also moves
smoothly).

## Product and architectural invariants

- D-002: fixed timestep, deterministic integer math. Recorded command
  streams may break ONLY via a documented constant change, not a schema
  change.
- Actor symmetry: monsters use the same per-tick movement scale.
- The camera defaults change below implements owner ruling D-107.

## Scope

1. Core: introduce a named per-tick movement step derived from
   `move_speed` and the fixed tick rate (suggested: `move_speed` stays
   the per-second value ~220; per-tick displacement =
   `move_speed * kTickMs / 1000` with integer math, ~11 units/tick at
   50ms). Dash becomes a short multi-tick burst or a single larger hop —
   named constants either way. Adjust `kMeleeRange`/`kThrustRange`/
   `kExtractionRange`/spawn positions ONLY if play distances become
   unreachable in reasonable time; justify every constant change in
   REPORT.md and update tests deliberately.
2. Client: camera follow already lerps; verify motion renders smoothly at
   the new step size (no per-input InvalidateRect thrash needed beyond
   the timer). Movement keys must feel continuous (no per-keypress
   dispatch bursts).
3. Client camera defaults per D-107: pitch 62, zoom 0.85, perspective
   0.0006, anchor 0.52, fog 0.4 — the slice's ARPG preset values —
   and wheel zoom-in blends toward the Miniature treatment (raise
   perspective toward 0.0013 as zoom passes ~1.05; simple linear blend,
   named constants).
4. Tests: per-tick displacement equals the named derivation; determinism
   replay updated; melee/thrust reachability still proven by the existing
   loop tests.

## Non-goals

Pathfinding, collision shapes, animation, gamepad.

## Deliverables

Core + client changes, coherent commit(s).

## Acceptance criteria

- Gates green; headless demo still completes its loop (command counts in
  the demo/tests may need adjusting for the new step — do it).
- Driven-input pass: walking a fixed real duration moves ~2-2.5 tiles/sec
  on screen (log positions per second in REPORT.md), motion visibly
  smooth in captures, dash reads as a dash not a teleport.

## Review focus

Constant derivations, reachability of the existing content, replay
determinism, and the D-107 camera blend behavior.

## Stop conditions

If the step change cascades into redesigning combat ranges wholesale →
stop, report the minimal consistent constant set, and ask.
