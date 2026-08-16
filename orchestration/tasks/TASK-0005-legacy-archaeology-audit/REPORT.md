---
task: TASK-0005
state: REVIEW_REQUESTED
branch: codex/TASK-0005-legacy-archaeology-audit
commits:
  - ff2ea30f0cb95003a61a0b3d1494abd7ec1a3fe6
base_commit: 1a41393
---

## Executive summary

The read-only legacy archaeology audit inventories the inherited browser-game
systems relevant to Verdigris, records their provenance, identifies extractable
data and formats, and highlights denylist and constitution risks. It makes no
runtime or product changes.

## System inventory

The report covers login/identity, movement, combat, inventory/items, world/map,
NPCs/monsters, skills/professions, crafting, persistence, and the networking
envelope. Each system is classified as inherited, Verdigris-era, or mixed, with
concrete source paths and line references.

## Provenance ledger

The worker report records path/line evidence and 40 resolving commit IDs,
including the Delaford-inherited runtime, Verdigris/WIZARD-era combat and
passive-lattice material, and Vesselforge-curated crafting rules. It preserves
the distinction between archaeology and native product authority.

## Extractable data and formats

The audit identifies JSON/JavaScript data, item/profile schemas, world layouts,
inventory-footprint constants, combat/skill tables, and persistence/network
formats that can inform future specs without porting browser implementation.

## Denylist gaps

It documents exact case-folded denylist coverage and gaps for camel/Pascal and
hyphenated identifiers, Delaford-derived zone/guest identifiers without the
literal name, and denied content in non-C++ native data files. These are audit
findings only; the task does not broaden the denylist.

## Surprises / risks against constitution and matrix

The report flags inherited starter-item/coin behavior, legacy mining and zone
identifiers, reward behavior, and passive-lattice/crafting seams that conflict
with the Bronze Age constitution or the legacy removal matrix. It explicitly
avoids deciding owner-only questions.

## Changed files

Only `orchestration/tasks/TASK-0005-legacy-archaeology-audit/REPORT.md` is
changed by worker commit `ff2ea30f0cb95003a61a0b3d1494abd7ec1a3fe6`.

## Verification

- Worker scope check: `git diff-tree --no-commit-id --name-only -r HEAD` listed
  only the task report; worker worktree was clean.
- Independent validator `/root/validate_task_0005`: ACCEPT.
- Validator checked 134 cited path/line references, all 40 commit IDs,
  `git diff --check`, and confirmed no forbidden repository edits.
- No tests were required for this read-only task.

## Specification deviations

None.

## Risks and limitations

This is evidence for future architecture work, not a recommendation to port
legacy systems or expand the denylist. Browser paths remain historical inputs
until an accepted native specification says otherwise.

## Questions for Fable or the owner

None introduced by the audit. Existing owner-only decisions remain untouched.

## Integration notes

The report is review-ready but must not be integrated as an accepted task until
the architect writes `REVIEW.md`. A read-only merge-tree check found the
expected same-path overlap with this coordinator summary; on acceptance,
integrate the worker's fuller `REPORT.md` from
`ff2ea30f0cb95003a61a0b3d1494abd7ec1a3fe6`, preserving its 382-line evidence
packet rather than deleting either side silently.
