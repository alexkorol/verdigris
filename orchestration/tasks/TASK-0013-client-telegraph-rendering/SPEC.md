---
id: TASK-0013
title: Render simulation telegraphs in the native client
state: READY
track: native
priority: high
base_commit: TASK-0011 integration tip (Codex records the actual SHA in STATUS.md on claim)
dependencies: [TASK-0011]
parallel_safe: false
owned_paths:
  - native/client/**
forbidden_paths:
  - native/src/**
  - native/include/**
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

The Win32 client renders honest ground-plane warnings for elite windups:
`AttackTelegraphed` events become a forward cone (thrust) or a circle
(sweep) at the monster's position, fading in over the windup and vanishing
when the strike resolves or is cancelled.

## Why this task exists

TASK-0011 made telegraphs simulation truth; constitution §3.9 requires
readable telegraphs; the client currently ignores the event. This
completes the readability loop in the lab client.

## Product and architectural invariants

- Presentation-only: consume `AttackTelegraphed` (actor id, action name in
  `text`, windup ticks in `value`) plus subsequent `AttackStarted`/
  `ActorDied` to end the warning. No combat math client-side — the cone
  drawn is derived from the monster's simulation `facing` and the same
  named ranges the sim exposes (read from the actor snapshot; if a needed
  constant is not visible to the client, approximate visually and note
  it — do NOT add core exports in this task).
- Match the existing procedural-effect vocabulary (projected ellipses/
  wedges on the ground plane, red-tinted like the slice's telegraphs).

## Scope

1. Track active telegraphs per actor from events; expire on strike,
   cancellation (actor death, player death), or ticks elapsed.
2. Render: thrust = forward wedge along the monster's facing at roughly
   thrust range; sweep = circle at melee range. Fade/pulse over windup.
3. Show the pending action name over the elite's life bar.
4. Help/debug overlay counts active telegraphs.

## Non-goals

Sound, screen shake, new core events, non-elite telegraphs.

## Deliverables

`native/client/main.cpp` changes, one coherent commit.

## Acceptance criteria

- `build.ps1 -RunTests -RunClient` exits 0; headless output unchanged.
- A driven-input pass on route:tin:2:0 (elite route) captures a thrust
  and/or sweep telegraph rendering before the corresponding damage, and
  confirms the warning disappears after the strike. Captures or logged
  state in REPORT.md.

## Required verification

The acceptance command plus the driven pass.

## File ownership

Client only. Sole in-flight client task.

## Dependencies

TASK-0011 integrated.

## Parallel-safety assessment

Not parallel with any client task; disjoint from core/tooling tasks.

## Review focus

Telegraph lifetime bookkeeping vs event stream, cancellation handling,
and honest geometry (the drawn cone must not promise more or less than
the sim checks).

## Stop conditions

Needing a core export (range constants, pending-action query) → stop and
file a question rather than adding core code.
