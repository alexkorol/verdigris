# TASK-0143 report — native first-expedition gameplay runtime slice

- worker: ox-pc-i
- branch: `codex/TASK-0143-native-gameplay-runtime-slice-ox-pc-i`
- base: routed HEAD `aaf89d3f7fe2fc47b5481c144883c6136b4d0ebf`; immutable task
  base `d0f74af3d30f238479218f8be412a01d61e21df3` verified ancestor of HEAD

## Executive summary

The C++ first-expedition path (`House → Scion → enter route → defeat enemy →
collect/equip item → extract`) was already mechanically complete, so per SPEC
this task adds the smallest owner-visible deterministic objective state that
makes the loop explicit instead of inventing balance or content: an
authoritative `ExpeditionPhase` on `InstanceState` that transitions
`SlayWardens → ExtractCarriedValue` exactly when the last living warden dies,
plus one appended telemetry event and focused regression tests.

## Concrete gap closed

An active expedition had no authoritative objective/phase state: the only way
to know whether the loop wanted "slay" or "extract" was to re-derive it by
scanning actors for living monsters — presentation logic duplicating a
simulation decision. Now the simulation owns a single named transition:

- `ExpeditionPhase::SlayWardens` on instance entry (reset every entry; the
  stale phase of a retired instance cannot leak into the next expedition).
- `ExpeditionPhase::ExtractCarriedValue` when a monster death leaves no
  living warden (recomputed both directions so a later `spawn_monster()`
  seam call restores SlayWardens on the next resolved kill).
- One `EventType::ExpeditionPhaseChanged` event emitted per actual change
  (text `slay-wardens` / `extract-carried-value`, value = enum ordinal).

Deterministic assertion proving it:
`the last kill flips the objective to extraction` plus
`exactly one authoritative phase transition is emitted` in
`test_expedition_phase_makes_the_first_expedition_loop_explicit`
(native/tests/core_tests.cpp), which also pins replay determinism of the
objective timeline, the fresh-entry reset, and that the phase is descriptive
telemetry rather than a command gate (`extraction remains available without a
phase gate`).

## Approach

- Inspected `Simulation` enter/spawn/drop/clear/death/extract flow and the
  existing lifecycle tests (`test_pack_clear_waits_for_the_last_monster`,
  `test_extraction`) before choosing the gap.
- Kept mechanics unchanged: no gate added to extraction or any other command;
  fixed-step, headless, server-authoritative behavior preserved. House/Scion
  recovery semantics untouched.
- Appended `ExpeditionPhaseChanged` as the LAST EventType value so recorded
  command streams keep historical numeric codes; snapshot/restore does not
  serialize events or active-instance state (instance retires at the D-109
  snapshot boundary), so persistence bytes are unchanged and stale persisted
  data keeps loading.
- The client's `event_label` switch has a `default:` case; the new event type
  is client-safe without touching client files (TASK-0142 owns that surface).

## Changed files (all inside owned_paths)

- `native/include/verdigris/core.hpp` — `enum class ExpeditionPhase`;
  `InstanceState::phase`; appended `EventType::ExpeditionPhaseChanged`.
- `native/src/core.cpp` — set phase in `resolve_enter`; authoritative
  recompute + single emit in the monster-death branch of `handle_death`.
- `native/tests/core_tests.cpp` — new regression test registered in `main()`.

## Public interfaces added/changed

- Added: `verdigris::ExpeditionPhase` (`SlayWardens`, `ExtractCarriedValue`);
  `InstanceState::phase`; `EventType::ExpeditionPhaseChanged` (appended).
- Changed: none (no existing symbol, value ordering, command, or save format).

## Test commands + outcomes (literal)

1. `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests`

   Tail of the transcript (full build log above it):

   ```text
   native legacy denylist: PASS
   verdigris core tests: PASS
   [swing] tgt=monster-1-19 now=1787382576954 next=0 range-ok
   ... (networking swing trace) ...
   verdigris networking tests: PASS
   camera2d tests: PASS
   PASS local: start succeeds
   ...
   PASS render-list: Drop op recorded from kill loot
   session tests passed
   ```

   Exit code: `0`.

2. `git diff --check` — no output. Exit code: `0`.

3. `git diff --name-only d0f74af3d30f238479218f8be412a01d61e21df3..HEAD`
   (recorded after the implementation commit):

   ```text
   native/include/verdigris/core.hpp
   native/src/core.cpp
   native/tests/core_tests.cpp
   orchestration/tasks/TASK-0141-procedural-native-visual-kit/SPEC.md
   orchestration/tasks/TASK-0142-native-client-presentation-slice/SPEC.md
   orchestration/tasks/TASK-0143-native-gameplay-runtime-slice/SPEC.md
   orchestration/tasks/TASK-0143-native-gameplay-runtime-slice/STATUS.md
   orchestration/tasks/TASK-0143-native-gameplay-runtime-slice/REPORT.md
   ```

   Exit code: `0`. Native writes confined to owned paths; the TASK-0141/0142
   SPEC files were part of the routed base history (coordination push), not
   edited by this worker.

Pre-existing compiler warnings (`core.cpp:1831 player_level C4100`,
WinSock/inet_addr deprecations, client main warnings) are unchanged from the
base commit; this diff introduces none.

## Manual verification

Not applicable beyond the automated gates: the slice is headless core +
tests. No client, server, browser, port, renderer, dependency, or CI surface
was touched (port 6500 untouched; no servers left running — the test gate's
session harness stops its own loopback servers).

## Commit SHAs

- CLAIMED: `5147cbed` (STATUS.md only).
- IMPLEMENTED: `ef35c911` (native core + tests).
- REVIEW_REQUESTED: this commit (REPORT.md + STATUS flip).

## Deviations

None from SPEC. Note: PROTOCOL's historical "NEVER push" rule is superseded
for this lane by the launch packet/RUN_STATUS correction ("workers still push
only their own branches"); this worker pushed only its own branch.

## Unresolved questions

None. No owner-policy action requested.

## Risks

- A future presentation must read `instance().phase` instead of scanning
  actors; until then nothing changes visually. TASK-0142 can consume the new
  state and event without a core change.

## Follow-ups

- Optional: scenario-harness coverage of the phase HUD once TASK-0142 renders
  an objective indicator (client-owned; not this task).
