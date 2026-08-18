---
task: TASK-0045
state: REVIEW_REQUESTED
branch: codex/TASK-0045-native-protocol-n3
commits:
  - d9c6fc6f
  - a2246893
  - 6d39565c
base_commit: ca0dd2df
architect_review_required: true
---

## Executive summary

The native N3 candidate adds deterministic combat actors, zone roles and aura
state, rare modifiers, the named Old Barrow boss telegraph, combat wire events,
minimum drops, position reconciliation, loopback-only binding, and EOF-idle
server behavior. It is review-ready, not architect-accepted.

## Implementation and interfaces

Rules remain in `WorldSimulation`; `ProtocolSession` translates them to the
existing `{event,data}` wire contract. `WorldMonster` now carries HP, damage,
death, role, rarity/modifiers, Empowered state, boss state, and telegraph
timing. The wire exposes `combat:hit`, `monster:telegraph`, expanded
`dev:state.monsters[]`, and minimum item-shaped `groundItems`.

The implementation was checked against the read-only browser references in
`server/core/monster.js`, the monster combat controller/behaviours, and
`server/core/map.js`. D-114-derived constants are named in `native/src/core.cpp`.

## Changed files

The implementation commit `d9c6fc6f` changes only native files:

- `native/include/verdigris/core.hpp`
- `native/include/verdigris/networking.hpp`
- `native/src/core.cpp`
- `native/src/networking.cpp`
- `native/src/server_main.cpp`
- `native/tests/networking_tests.cpp`

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests`: denylist, core,
  and networking tests all passed.
- Unchanged attach against harness commit
  `babba96fcde5ca1d55610853ad59af4fd57e2374`: `combat`,
  `encounter-variety`, `boss-mechanic`, `quickstart`, `single-session`,
  `movement`, and `zones` passed, 7/7.
- C++ coverage asserts pack population, rare modifier, aura, named Warden,
  telegraph radius/window, and emitted telegraph skill ID.
- Authentic negative: changing telegraph radius 2 to 1 caused the networking
  assertion to fail; restoring 2 returned the gate to green.

## Deviations and architect handoff

Ground drops are intentionally minimum N3-shaped payloads; full item identity,
pickup, and inventory rules remain N4 work. Aura behavior is bounded state and
damage buffering. Continuous AI is not yet an independent timer; fixed-step
combat advances at protocol command/state boundaries for this slice.

The architect must rebuild the branch and personally rerun the seven-scenario
attach before deciding `ACCEPTED` or `REVISE`, with particular attention to
boss timing and the command-boundary combat heartbeat.
