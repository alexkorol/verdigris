---
task: TASK-0038
state: BLOCKED
branch: codex/TASK-0038-combat-controls-rebinding
commits:
  - f4df0b99
base_commit: e462c26d
---

## Executive summary

TASK-0038 was audited but not implemented. Its acceptance behavior is wired
through browser components outside the task's owned paths.

## Implementation

No source changes. The actual mounted seams are GameCanvas (world clicks and
InputController), slots/Settings (settings pane), and hud/Quickbar (skill
labels). An unreferenced UI component under the declared glob would not work.

## Changed files

Coordinator metadata and QUESTION-0008 only; worker evidence commit f4df0b99
contains the ownership-seam note.

## Interfaces

None added or changed.

## Verification

- Targeted baseline — 5/5 passed.
- `npm run test:unit` — baseline passed, 122 files / 779 tests.
- `npm run smoke:browser` — invalid baseline: port 6500 was occupied and the
  endpoint returned HTML instead of JSON.

## Manual checks

Read-only source audit traced click handlers, settings registration, and
skill-label rendering to the files listed in QUESTION-0008.

## Specification deviations

The controls/rebinding implementation is deferred; a partial helper-only
change would not satisfy the world-click, mounted UI, or live-label criteria.

## Risks and limitations

The task remains blocked until Fable expands ownership, splits the task, or
provides an approved seam.

## Questions for Fable or the owner

See QUESTION-0008.

## Integration notes

No source commit is available for integration.
