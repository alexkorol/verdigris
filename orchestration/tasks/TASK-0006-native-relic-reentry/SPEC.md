---
id: TASK-0006
title: Deterministic relic re-entry into the native drop stream
state: READY
track: native
priority: high
base_commit: bc73ce0
dependencies: [TASK-0001]
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

Relics registered at a Scion's death can deterministically resurface in a
later expedition's drop stream, carrying accumulated history, and the
previously dormant `relic_extracted` legend path is exercised end to end.

## Why this task exists

D-004: "lost equipment may later re-enter the wider loot pool … a dead
Scion's item may return with additional history." The core registers relic
candidates (TASK-0001 reviewed this as dormant) but nothing ever returns
them. This closes the death → loss → rediscovery loop, the emotional spine
of House continuity.

## Product and architectural invariants

- D-002 determinism: resurfacing uses the simulation's existing seeded
  `Rng`; identical seed + command stream ⇒ identical resurfacing.
- D-004 mixture: partially predictable (a deeper route can surface a
  relic), never guaranteed on a specific kill, never the whole pool at
  once. Do NOT hand the successor the dead Scion's full inventory.
- Item identity is stable: the resurfaced item keeps its id; history only
  appends. No duplication — an item exists in exactly one of: relic pool,
  ground, carried, stored.
- Denylist gate stays green; no client or build-file edits.

## Inputs and references

- `native/src/core.cpp` (`drop_reward`, `handle_death`, `resolve_extract`,
  `record_legend`), `native/include/verdigris/core.hpp` at base_commit.
- TASK-0001 REVIEW.md (dormant-hook observation), DECISIONS.md D-004.

## Scope

1. In `drop_reward` (or an adjacent seam), when `house_.relic_candidates`
   is non-empty and at least one Scion has died, roll the simulation Rng;
   on success (suggested 1-in-4, as a named constant) move the OLDEST relic
   candidate out of the pool and drop it as a ground item alongside or in
   place of the generated reward (implementer's choice; document it),
   appending a history line naming the route it resurfaced on.
2. Emit a new `RelicResurfaced` event (or equivalently named; keep the
   naming consistent with existing EventType style) when this happens.
3. Record a legend entry (kind `relic_resurfaced`) via the existing
   `record_legend` seam.
4. Confirm the existing flows already compose (and cover them in tests):
   picking the relic up, extracting it (fires `ItemExtracted` plus the
   existing `relic_extracted` legend), and dying while carrying it
   (returns it to `relic_candidates` with another history line).

## Non-goals

- No seasonal policy (D-O1), no relic UI, no client changes, no weighting
  by item quality, no owner-lore naming.

## Deliverables

Code + tests, one coherent commit on a worker branch.

## Acceptance criteria

- `native/build.ps1 -RunTests` exits 0 with new tests green.
- Determinism test: same seed + commands ⇒ identical resurfacing ticks,
  item ids, and legends.
- Round-trip test: death registers relic → successor's expedition
  resurfaces it (drive enough reward drops to guarantee the roll fires) →
  pickup → extract → item is in `stored_items`, still `relic_candidate`
  identity-stable, history contains death, resurfacing, and recovery
  lines in order, and legends contain `relic_resurfaced` then
  `relic_extracted`.
- Loss-again test: dying while carrying a resurfaced relic returns it to
  the pool exactly once (no duplication) with history grown.
- Existing eleven behaviors and Legends tests remain green.

## Required verification

```powershell
powershell -File native/build.ps1 -RunTests
```

List the new test names in REPORT.md.

## File ownership

Only `native/src/**`, `native/include/**`, `native/tests/**`.

## Dependencies

TASK-0001 (integrated at base_commit).

## Parallel-safety assessment

Disjoint from TASK-0002 (build files) and TASK-0004 (client). No other
task owns core sources this wave.

## Review focus

- Rng usage keeps replay determinism (no new RNG streams that reorder
  existing rolls in ways that break the TASK-0001 replay test — if the
  resurfacing roll must consume from the shared Rng, update expectations
  deliberately and say so in REPORT.md).
- Single-ownership invariant for the item across pool/ground/carried/
  stored.
- The 1-in-4 constant is named and documented, not scattered.

## Stop conditions

- Any temptation to design relic *quality* transformation (scars, bond
  changes, upgrades) — that is future Brands & Bonds work; resurfacing
  history lines only.
- Any need to touch client or build files → stop and file a question.
