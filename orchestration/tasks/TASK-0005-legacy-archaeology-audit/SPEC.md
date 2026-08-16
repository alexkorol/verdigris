---
id: TASK-0005
title: Legacy browser-game archaeology audit (read-only)
state: DRAFT
track: research
priority: medium
base_commit: TBD (set on promotion)
dependencies: []
parallel_safe: true
owned_paths:
  - orchestration/tasks/TASK-0005-legacy-archaeology-audit/REPORT.md
  - orchestration/tasks/TASK-0005-legacy-archaeology-audit/appendix/**
forbidden_paths:
  - "everything else — this task edits no repository files outside its own task folder"
acceptance_commands: []
---

DRAFT — do not claim until state reads READY and base_commit is a SHA.
(Promotable any time a reader worker is free; read-only, so parallel-safe
with all implementation tasks.)

## Goal

An evidence-backed audit of the historical browser game that lets the
architect revise `docs/rebuild/LEGACY_MATRIX.md` from facts instead of
folklore.

## Why this task exists

The constitution says no Delaford behavior survives by default, but the
browser game also contains post-Delaford Verdigris design (Chronicles/House
seams, world-web, Brands & Bonds inventory identity, combat pacing) that the
native rebuild should mine deliberately. Nobody has produced a current,
file-level map of which is which.

## Product and architectural invariants

- Read-only: zero edits outside this task's folder.
- Findings are evidence (paths, line refs, data samples), not decisions;
  KEEP/REMOVE classification remains architect-owned.

## Inputs and references

- `src/` (client), `server/` (game logic), `tests/`, `docs/` at base_commit.
- `config/legacy-denylist.json`, `docs/rebuild/LEGACY_MATRIX.md`,
  `docs/product/VERDIGRIS_CONSTITUTION.md` (what the product keeps).

## Scope

Produce REPORT.md (plus optional appendix files) covering:

1. **System inventory**: each major runtime system (login/identity,
   movement, combat, inventory/items, world/map, NPC/monsters, skills/
   professions, crafting, persistence, networking envelope) with entry-point
   paths and a 2-4 sentence description of what it actually does today.
2. **Provenance tags** per system: `delaford-inherited`,
   `verdigris-era`, or `mixed` — with the evidence (commit archaeology via
   `git log --follow`, naming, docs).
3. **Extractable data**: item catalogues, stat curves, map/world data,
   balance constants worth porting as *data* even where code is discarded —
   exact file paths and formats.
4. **Denylist gaps**: legacy identifiers present in the codebase that the
   current denylist would NOT catch if reintroduced natively.
5. **Surprises/risks**: anything contradicting the constitution or the
   current LEGACY_MATRIX classifications.

## Non-goals

- No refactoring, no deletions, no matrix edits, no native code.

## Deliverables

- `REPORT.md` in this folder (findings, organized as above), optional
  `appendix/` data extracts. One commit.

## Acceptance criteria

- Every claim carries a path (and line/commit where relevant).
- All five scope sections present; section 3 lists concrete files.
- `git status` shows changes only inside this task folder.

## Required verification

`git status --short` output included in REPORT.md proving read-only scope.

## File ownership

Only this task's folder.

## Dependencies

None.

## Parallel-safety assessment

Read-only; safe beside anything.

## Review focus

Evidence quality, denylist-gap list, and whether any finding forces a
LEGACY_MATRIX or constitution correction.

## Stop conditions

- Any fix-it temptation (bugs found in legacy code) → record, don't touch.
