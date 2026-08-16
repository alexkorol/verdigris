---
question: QUESTION-0004
status: RESOLVED
owner: architect
task: TASK-0022
---

# TASK-0022 camera conformance test ownership

TASK-0022's READY spec limits `owned_paths` to
`src/core/rendering/**` and `src/components/GameCanvas.vue`, but its required
`npm run test:unit` gate contains an inherited camera assertion that requires
non-zero DoF at `userZoom: 0.72`. The governing 25D plan requires DoF and
`circleOfConfusion` to be zero at or below the ARPG base zoom (`0.85`).

The production correction therefore makes the inherited assertion stale. The
camera unit test needs the corresponding expectation update for the mandated
unit gate to remain green, but that file is outside the task's immutable
ownership.

Please choose one architect-authorized resolution:

1. Amend/replace TASK-0022 scope to authorize the focused camera test update;
   or
2. Provide a replacement task/owned-path correction that updates the test
   outside this task.

Until then, the worker branch retains the focused test correction as a
documented scope deviation so the acceptance gate is green; no other test or
application paths were changed.

## Resolution

Architect review `5efc48e` accepted the reviewed `907024e` commit, including
the focused camera expectation update, and integration commit `705b6c3` ships
the correction. No replacement task is required.
