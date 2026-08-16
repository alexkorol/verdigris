---
id: TASK-0018
title: Owner ruling D-106 — no item is destroyed by Scion death
state: READY
track: native
priority: high
base_commit: after TASK-0015 integrates (coordinator records the SHA in STATUS.md)
dependencies: [TASK-0015]
parallel_safe: false
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

Implement the owner's ruling (DECISIONS.md D-106): Scion death never
destroys items. Everything the fallen Scion carried becomes recoverable.

## Why this task exists

Baseline behavior registers only the EQUIPPED item as a relic candidate
and clears the rest of the pack (lost forever). The owner ruled
2026-08-16: "Items stay recoverable on death, at the very least they
return to the loot pool."

## Product and architectural invariants

- Extraction risk stays real: death still delays access (items must
  resurface via the existing deterministic re-entry roll, TASK-0006) —
  recoverable ≠ immediately inherited. Successors do NOT start with the
  dead Scion's inventory (constitution / D-004 unchanged).
- Item identity/history preserved; single-ownership invariant holds.

## Scope

1. `handle_death` (Scion branch): move ALL carried items — equipped and
   pack — into `relic_candidates` (equipped item keeps its existing
   "registered after Scion death" history line; pack items get an
   equivalent line, e.g. "lost at <route>, awaiting recovery"). Carried
   trophies: append to a recoverable pool as well (suggested: a
   `lost_trophies` vector on House with resurfacing alongside items, OR
   the simpler rule that trophies join stored_trophies is NOT acceptable
   — that would remove extraction risk; pick the recoverable-pool
   approach and document it).
2. Re-entry (TASK-0006 seam) continues to draw oldest-first from the
   larger pool; verify the roll cadence still feels per-seed reasonable
   (unchanged constant unless tests show starvation; justify any change).
3. Tests: multi-item death registers every carried item exactly once
   (no duplication, nothing in stored_items); trophies recoverable;
   resurfacing eventually returns pack items with ordered history;
   successor starts empty; replay determinism; existing relic round-trip
   tests updated deliberately where the old "lost forever" assertions now
   contradict D-106 (call these out in REPORT.md).

## Non-goals

Quality/scar transformation of returned items (future Brands & Bonds),
UI, seasonal policy.

## Acceptance criteria

Gates green; the D-106 behaviors each have a named test; the old
lost-forever assertions are removed with justification.

## Review focus

Single-ownership across the enlarged pool, no starvation of the
resurface roll, and that successors still start empty.

## Stop conditions

If trophies-recovery design grows beyond a simple pool + resurface hook →
stop and ask (may be owner-lore adjacent).
