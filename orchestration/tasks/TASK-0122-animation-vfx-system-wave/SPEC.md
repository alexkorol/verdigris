---
task: TASK-0122
title: Native animation/VFX Phase A — readable event beats
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
base_commit: 8eb9589361631f8110e177bc2b63532f84219367
dependencies: [TASK-0116 ACCEPTED at b8030f10, D-113 procedural fallback]
owned_paths:
  - native/client/presentation_events.hpp
  - native/client/presentation_events.cpp
  - native/client/presentation_state.hpp
  - native/client/presentation_state.cpp
  - native/client/local_session.cpp
  - native/client/remote_session.cpp
  - native/client/main.cpp
  - native/tests/presentation_events_tests.cpp
  - native/build.ps1
  - native/CMakeLists.txt
  - orchestration/tasks/TASK-0122-animation-vfx-system-wave/**
forbidden_paths:
  - native/src/**
  - native/include/**
  - native/tests/session_tests.cpp
  - server/**
  - src/**
  - assets/**
  - orchestration/tasks/TASK-0148-native-chronicles-reconnect-runtime/**
worker: ox-pc-x
worker_branch: codex/TASK-0122-animation-vfx-phase-a-ox-pc-x
worktree: Z:\Code\.worktrees\verdigris\ox-pc-x
ports: 7060-7079
---

# Outcome

Ship the first owner-visible native animation/VFX slice without changing
simulation authority or waiting for authored art. Combat and lifecycle events
that already exist must produce distinct, deterministic presentation beats so
the client reads less like a static wireframe.

# Frozen Phase A scope

1. Map local `ScionLost` and `BuffExpired` through the presentation seam and
   give each a readable, time-bounded procedural beat.
2. Consume already-shipped remote `combat:hit` critical/style data and render a
   visibly distinct critical-hit treatment. No server-envelope change is
   allowed in this packet.
3. Add a deterministic spawn/materialization beat when a monster first appears
   in the client presentation state; never create, move, or damage an actor.
4. Remove the proved client-only monster-facing inversion and lock the corrected
   facing in tests.
5. Centralize every new Phase A TTL/pulse/color/style value in one named
   presentation constants table. Do not scatter magic timing literals.
6. Add a dedicated presentation-events test binary and an
   `animation-vfx-phase-a` client scenario. The scenario must prove event
   timing and write fresh 960x600 plus 1366x768 PNG evidence under this task's
   `captures/` folder for architect visual review.

The renderer remains in `native/client/main.cpp` for this bounded wave; a
renderer extraction is explicitly deferred. Telegraph radius/position wire
work and `native/tests/session_tests.cpp` are deferred because TASK-0148 owns
those surfaces now.

# Product bar

- Critical, ordinary hit, spawn, buff expiration, and Scion loss must not be
  indistinguishable flashes of the same color/shape/timing.
- Effects may use procedural vector/GDI primitives and the accepted visual kit;
  no new image asset or final art-direction choice is authorized.
- Presentation may interpolate or decorate authoritative state but may not
  freeze simulation, alter hit/death timing, invent loot, or mutate progression.
- At both capture resolutions, effects must not cover the player, objective,
  identity strip, inventory, or extraction controls.

# Acceptance

Run from the worker worktree on the frozen head:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native/build/verdigris_presentation_events_tests.exe
native/build/verdigris_client.exe --scenario animation-vfx-phase-a
npm run playtest
git diff --check
```

The worker report must include the exact exits, final changed-path inventory,
two fresh PNG paths and dimensions, deterministic timing assertions, and a
negative control showing that presentation effects do not modify simulation
state. `REVIEW_REQUESTED` is a handoff only; the architect must rerun the gates
and inspect both images before acceptance.

# Stop conditions

- Stop for review if any required field is absent from the existing client
  envelope/state; do not edit `native/src/networking.cpp` to manufacture it.
- Stop if TASK-0148 touches an owned client path after this claim.
- Never touch port 6500; use only 7060-7079 for any loopback helper.
- Push only the worker branch. Never merge, rebase the program branch, or
  modify another task's status/evidence.
