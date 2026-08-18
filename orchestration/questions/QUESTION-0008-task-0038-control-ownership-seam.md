# QUESTION-0008 — TASK-0038 control and settings ownership seam

Related task: TASK-0038 LMB/RMB attacks + key/mouse rebinding UI

## Decision needed

How should TASK-0038 reach the mounted browser seams required by its
acceptance criteria? The declared paths cover controls and input helpers, but
the actual world input and settings wiring live elsewhere.

## Evidence

- `src/components/GameCanvas.vue` owns `@click.left`, `@contextmenu`,
  `leftClick`, `rightClick`, and mounts `InputController`.
- `src/Delaford.vue` mounts `src/components/slots/Settings.vue`; no
  `src/components/ui/Settings*.vue` exists.
- `src/components/hud/Quickbar.vue` owns skill-bar labels.
- Therefore the declared paths cannot wire world-click attacks, preserve the
  context menu, mount rebinding UI, or update live skill labels.

## Options

1. Expand ownership to the minimal seams: `GameCanvas.vue`,
   `components/slots/Settings.vue`, `components/hud/Quickbar.vue`, and, if
   needed, `Delaford.vue`.
2. Split controls-core from a sequential browser-wiring task.
3. Provide another architect-approved event seam and revise the scope.

Recommendation: option 1 is the smallest coherent change.

No source changes were made because an implementation limited to the declared
paths would be unreachable.
