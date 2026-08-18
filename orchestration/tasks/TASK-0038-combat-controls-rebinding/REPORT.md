---
task: TASK-0038
state: BLOCKED
branch: codex/TASK-0038-rebinding-kimiwork
commits:
  - 01a12d72
  - 4a8983cb
base_commit: e462c26d
architect_review_required: true
---

## Executive summary

The worker produced a substantive controls/rebinding candidate, but it is not
review-ready under the current task contract. The implementation reaches the
mounted browser seams that QUESTION-0008 identified, while the declared task
ownership does not include those seams. Unit, build, and lint gates pass; the
unchanged smoke gate still encodes the pre-rebind UI and context-menu contract.

## Implementation

- `01a12d72` adds a live persisted action map and input-controller rebinding.
- `4a8983cb` wires LMB/RMB world attacks, Shift+RMB context access, the mounted
  settings bindings panel, and live Quickbar labels.
- The implementation covers the actual seams in `GameCanvas.vue`,
  `slots/Settings.vue`, `hud/Quickbar.vue`, and a new `SettingsBindings.vue`.

## Verification

- Focused controls tests: 24/24 passed.
- `npm run test:unit`: 123 files / 791 tests passed.
- `npm run build`: passed.
- `npm run lint`: passed.
- `npm run smoke:browser`: fails at the unchanged expectation for `Bronze Arc
  [Space / 1]`; the candidate correctly renders `LMB / 1`. The later smoke
  step also expects unmodified right-click context-menu behavior, while the
  candidate intentionally reserves Shift+RMB for that access.

## Review blockers

1. Architect/owner must expand ownership to the mounted component seams or
   provide the alternative event seam in QUESTION-0008.
2. The browser smoke expectations must be revised as a deliberate contract
   update, not hidden by weakening the implementation.
3. Required captures for persisted rebind across reload and LMB/RMB attacks
   landing are absent.
4. D-115 hands-on play review remains outstanding.

This report deliberately keeps the task `BLOCKED`; the source candidate is
preserved and reviewable after those scope and evidence decisions are resolved.
