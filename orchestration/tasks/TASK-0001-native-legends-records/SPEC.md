---
id: TASK-0001
title: Bounded deterministic Legends records in the native core
state: READY
track: native
priority: high
base_commit: f5b4b72
dependencies: []
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
  - docs/**
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests
---

## Goal

The native core records a bounded, deterministic "Legends" history for the
House: notable deeds of living and fallen Scions, queryable by presentation,
capped in size, stable across identical seed+command replays.

## Why this task exists

House continuity is the product's spine. Lineage exists (`fallen_scions_`)
but nothing records *what* a Scion did in a form the House (and future
season/history features per the constitution §3.2) can keep. The sprint map
(Milestone C, `docs/rebuild/SPRINTS.md`) names "bounded Legends records" as a
working-House requirement.

## Product and architectural invariants

- Core stays headless, deterministic, dependency-free (D-002).
- Events out, commands in: Legends entries are produced by the simulation
  from events it already emits, never pushed in by presentation.
- Bounded: the record cannot grow without limit (seasonal resets and
  serialization depend on this).
- No Delaford identifiers (denylist gate must stay green).

## Inputs and references

- `native/include/verdigris/core.hpp`, `native/src/core.cpp`,
  `native/tests/core_tests.cpp` at base_commit.
- `docs/product/VERDIGRIS_CONSTITUTION.md` §House loop, §Seasonal loop.
- `orchestration/DECISIONS.md` D-002/D-003/D-004.

## Scope

1. Add a `LegendEntry` value type (suggested fields: ordinal, tick,
   scion_id, scion_name, kind, subject, detail) and a bounded
   `std::vector<LegendEntry>`-backed record on `House` (cap as a named
   constant, suggested 64; evict oldest non-founding entries first).
2. Record entries for at least: Scion creation, first clear of a route,
   branch/knowledge unlock, elite kill (enemy level >= scion level + 2 or a
   flagged elite), Scion death (with killer and route), item extraction of
   relic-candidate items, House founding-equivalent milestones already
   modeled (campaign_complete).
3. Emit a `LegendRecorded` event when an entry is added.
4. Public read accessor on `Simulation`/`House` for the record.
5. Tests: determinism of the record under identical seed+commands; bound
   enforcement (overflow evicts, cap holds); death and route-clear entries
   survive succession; record content references stable ids.

## Non-goals

- No serialization/persistence work.
- No client/UI display.
- No seasonal reset logic (D-O1 is owner-only).
- No renaming of existing events or types beyond what the record needs.

## Deliverables

- Code in `native/src` + `native/include` implementing the above.
- New/extended tests in `native/tests/core_tests.cpp` (or a sibling file
  wired into the existing test build — note build files are forbidden here;
  if a new file cannot be added without touching CMake/build.ps1, extend
  `core_tests.cpp`).
- One coherent commit on the program branch (or worker branch merged to it).

## Acceptance criteria

- `native/build.ps1 -RunTests` exits 0: build clean, denylist PASS, all
  existing 11 behaviors still green, new Legends tests green.
- Same seed + same command stream ⇒ byte-identical Legends record
  (asserted in a test).
- Record length never exceeds the cap (asserted by a test that generates
  more than cap entries).
- A successor Scion can read the fallen Scion's death entry (asserted).

## Required verification

```powershell
powershell -File native/build.ps1 -RunTests
```

Report the full test-name list added.

## File ownership

Edit only `owned_paths`. Everything else is forbidden, including build
scripts and docs.

## Dependencies

None (wave-parallel with TASK-0002/0003; path sets are disjoint).

## Parallel-safety assessment

TASK-0002 touches build/CI files only; TASK-0003 touches `prototypes/**`
only. No shared files. Safe.

## Review focus

- Determinism: no wall clock, no unseeded randomness, no address-dependent
  ordering in the record.
- Bound/eviction logic correctness at the cap boundary.
- Event emission ordering relative to the state change.
- Whether entry kinds map to real constitution moments rather than noise.

## Stop conditions

- Adding an entry kind would require inventing product canon (e.g., titles,
  honors) → stop, file a question.
- The cap/eviction policy materially interacts with a future seasonal rule
  → implement the mechanical cap, note the interaction in REPORT.md, do not
  design the seasonal rule.
