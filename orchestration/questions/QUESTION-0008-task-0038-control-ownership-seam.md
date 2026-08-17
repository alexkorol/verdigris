# QUESTION-0008 — TASK-0038 control and settings ownership seam

Related task: TASK-0038 LMB/RMB attacks + key/mouse rebinding UI

## Decision needed

How should TASK-0038 be allowed to reach the mounted browser seams required by
its acceptance criteria? The task is currently scoped to
`src/core/config/controls.js`, `src/core/utilities/input-controller.js`,
`src/components/ui/Settings*.vue`, `src/core/player/**`, and control tests.
The actual world input and settings wiring live outside those paths.

## Evidence

- `src/components/GameCanvas.vue` owns the canvas event bindings:
  `@click.left="leftClick"` and `@contextmenu.prevent="rightClick"`.
  Its `leftClick` and `rightClick` methods are the only current world-click
  dispatch/context-menu seam, and `initialiseInputController` is also mounted
  there.
- `src/Delaford.vue` imports `./components/slots/Settings.vue` as
  `SettingsPane` and registers it as the `settings` pane.
- There is no `src/components/ui/Settings*.vue` file in the current tip.
  `src/components/slots/Settings.vue` is the actual settings component.
- The skill-bar labels are rendered by `src/components/hud/Quickbar.vue`, also
  outside TASK-0038's owned paths.
- Therefore a worker restricted to the declared paths cannot wire LMB/RMB
  attacks, preserve/re-route context-menu access, mount the rebinding UI, or
  make live binding labels appear in the existing skill bar. Creating an
  unreferenced `src/components/ui/Settings*.vue` would not satisfy the product
  behavior or acceptance gate.

## Options

1. Amend TASK-0038 ownership to include the minimal browser integration seams:
   `src/components/GameCanvas.vue`, `src/components/slots/Settings.vue`,
   `src/components/hud/Quickbar.vue`, and (if needed for pane registration)
   `src/Delaford.vue`; keep server/native/prototype paths forbidden.
2. Split the work into a controls-core task and a sequential browser-wiring
   task whose owner includes those paths, with the current task explicitly
   unable to claim the full acceptance criteria until the wiring task lands.
3. Provide another architect-approved seam (for example a pre-existing event
   adapter) and update the immutable task scope before implementation.

## Recommendation

Option 1 is the smallest coherent change. The requested behavior is a client
interaction change, and the current component ownership makes a complete,
reviewable implementation impossible without touching the actual mounted
handlers and pane.

## Status

Awaiting architect direction. No source changes were made because any
implementation limited to the declared paths would be unreachable or would
silently fail the stated acceptance criteria.
