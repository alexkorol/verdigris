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
forbidden_paths:
  - "** (everything else: this task edits no repository files)"
acceptance_commands: []
---

DRAFT — do not claim. Outline for planning:

## Goal

A read-only audit of the historical browser game (`src/`, `server/`)
producing evidence for the next LEGACY_MATRIX revision: which systems embody
design the constitution keeps (combat timing values, Chronicles/House seams,
world-web structure, inventory identity), which are Delaford residue, and
which contain data worth extracting (maps, item stats, balance curves).

## Notes for the eventual spec

- Output is REPORT.md only (plus optional appendix files inside the task
  folder). No code edits anywhere.
- Good fit for cheap Luna readers; several can split src/ vs server/ vs
  tests/ and merge findings.
- The architect (not the worker) will translate findings into
  `docs/rebuild/LEGACY_MATRIX.md` changes — that file is architect-owned
  per PROTOCOL.md ownership of docs.
