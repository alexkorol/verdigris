---
task: TASK-0044
state: CLAIMED
branch: codex/TASK-0044-native-protocol-n2
worker_worktree: C:\Users\Alex\Documents\KimiWork\verdigris
claim_commit: 6e6279c
base_commit: 32d7b6e
---

# TASK-0044 — Native protocol N2

## Executive summary

The task is actively in implementation by Kimi Code. The WIP ports the
unchanged harness's world, continuous movement, solo-instance entry, scene
metadata, monster population, and stair-return contracts into the native
simulation/networking layer. It is not ready for architect review: the source
is uncommitted, the latest required native build stops in a new networking
test, and the earlier attach probe still has a saved-position lifecycle edge.

## Implementation observed

The external WIP adds deterministic world state and adventure-zone metadata in
`native/include/verdigris/core.hpp` and `native/src/core.cpp`, extends the
native protocol session in `native/include/verdigris/networking.hpp` and
`native/src/networking.cpp`, and adds movement/zone tests in
`native/tests/networking_tests.cpp`. It maps `player:move`,
`instance:enterSolo`, `dev:teleport`, and `dev:state` without changing
`playtest/**`.

## Verification so far

The first WIP build passed the native denylist, core tests, and networking
tests. Against an alternate native server, unchanged `movement` passed in
4.596 seconds; `zones` reached all six zone entries and passed names, layouts,
stairs, and monster populations, but failed the final saved-position check
(`39.666666,116.333334` versus `6,22`). Full details are in `BASELINE.md`.

The latest recheck cannot reach those gates. MSVC stops at
`native/tests/networking_tests.cpp:74`: the new `request_state` helper passes
a `JsonValue` to `parse_envelope`, whose current signature requires an
`Envelope&`. The source also emits a non-fatal unused-local warning at
`native/src/networking.cpp:306`.

## Acceptance status

Not ready. Required evidence is still missing: a clean
`powershell -File native/build.ps1 -RunTests`, unchanged `movement` and
`zones` attach success including saved-position restoration, a driven
transcript, and architect rerun. The coordinator has made no native source
changes and has not loosened the harness assertions.

## Next action

Kimi should correct the test helper to parse an `Envelope`, rerun the native
gate, then resolve or explain the pre-instance-position lifecycle edge before
the unchanged attach scenarios are rerun. Only then should this report move
to `REVIEW_REQUESTED`.
