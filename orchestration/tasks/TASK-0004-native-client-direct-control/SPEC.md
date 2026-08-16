---
id: TASK-0004
title: Native client direct-control pass per D-007 contract
state: DRAFT
track: native
priority: high
base_commit: TBD (set on promotion; latest integrated tip at that time)
dependencies: [TASK-0002]
parallel_safe: false
owned_paths:
  - native/client/**
forbidden_paths:
  - native/src/**
  - native/include/**
  - native/CMakeLists.txt
  - native/build.ps1
  - src/**
  - server/**
  - prototypes/**
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests -RunClient
---

DRAFT — do not claim until state reads READY and base_commit is a SHA.
(Will be promoted after wave-1 reviews; sequential with any other
client-touching task.)

## Goal

`native/client/main.cpp` implements the D-007 control contract
(`orchestration/DECISIONS.md`) against the existing simulation actions,
with unimplemented skill slots visibly stubbed rather than bound to wrong
semantics.

## Why this task exists

The client still uses the placeholder bindings from Milestone D (P pickup,
E equip, X extract). D-007 fixes the product contract: E is a skill slot,
equip belongs to inventory UI, X is nearest-pickup, F is contextual
interact. Aligning the lab client now prevents the wrong muscle memory from
calcifying and unblocks the feature-checklist client items.

## Product and architectural invariants

- Presentation requests commands; the simulation resolves them (D-002).
  The client must NOT gain gameplay logic or extend the core's action set.
- Control contract (D-007): WASD move; mouse aim; LMB primary (Melee);
  RMB weapon skill (Dash until the core offers more); Space dodge/dash;
  Q/E/R skill slots — render as disabled placeholders in the overlay until
  the core exposes bindable actions; X = pick up nearest ground item or
  trophy; Z = toggle loot-name overlay; F = contextual interact
  (extraction when at the pad); I = gear/House readout overlay.
- `--headless` behavior and output must remain byte-compatible (the smoke
  gate depends on it).

## Inputs and references

- `native/client/main.cpp` at base_commit; `native/README.md` control doc
  (README may be updated by this task ONLY if added to owned_paths at
  promotion — otherwise note needed doc changes in REPORT.md).
- `orchestration/DECISIONS.md` D-007; `docs/rebuild/SPRINTS.md` Milestone D
  extension items.
- HANDOFF 2026-08-15 (later) entry for the driven-input verification
  pattern (PostMessage keys, PrintWindow captures).

## Scope

1. Rebind: X = nearest pickup (items before trophies at equal distance is
   fine; document the tiebreak), F = extract when within the pad radius,
   Space and RMB = Dash, LMB = Melee, remove P/E world bindings.
2. Equip moves into an overlay: I toggles a gear panel listing carried
   items; a single keypress or click equips the highlighted item (simplest
   deterministic mechanism; document it).
3. Z toggles loot name labels above ground drops.
4. Q/E/R render in a compact skill strip as visibly disabled slots with
   the D-007 legend; pressing them shows a "not yet bound" hint, sends no
   command.
5. Update the on-screen help lines to the new contract.

## Non-goals

- No new simulation actions, stats, or events (core is forbidden).
- No art, no menus beyond the gear overlay, no rebinding UI.

## Deliverables

- Updated `native/client/main.cpp`; one coherent commit.

## Acceptance criteria

- `build.ps1 -RunTests -RunClient` exits 0 (build, tests, denylist,
  headless loop unchanged).
- A driven-input pass (PostMessage) proves: X picks up a dropped item,
  I-overlay equip works, F at the pad extracts, Z toggles labels, Q/E/R
  produce the stub hint and no state change. Include PrintWindow captures
  or logged state assertions in REPORT.md.

## Required verification

```powershell
powershell -File native/build.ps1 -RunTests -RunClient
```

plus the driven-input pass described above (script may live in the task
folder or a temp dir; do not commit throwaway scripts outside the task
folder).

## File ownership

`native/client/**` only.

## Dependencies

TASK-0002 (build guard for the windowed define) integrated first.

## Parallel-safety assessment

Sole writer of client files; must not run beside any other client task.

## Review focus

- No gameplay decisions smuggled into the client (damage, ranges, drop
  logic stay core-side).
- Headless output unchanged; window path verified by driven input.
- Contract fidelity to D-007, including the disabled Q/E/R semantics.

## Stop conditions

- Any need to touch `native/src|include` → stop, file a question (that is
  a separate core task).
- Ambiguity about equip UX beyond "simplest deterministic mechanism" →
  choose the simplest, note it; do NOT invent inventory features.
