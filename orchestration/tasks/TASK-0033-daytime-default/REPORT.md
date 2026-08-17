---
task: TASK-0033
state: INTEGRATED
branch: codex/TASK-0033-daytime-default
commits:
  - bf404b3d236fc4402adcdd1a0327a134de7fff9a
  - f409b99a3174769fee92e1ccdeb3514e79846ed3
base_commit: 96601378
---

## Executive summary

The browser now opens with the authored midday ambient grade by default. A
single existing-settings toggle opts into the full day/night cycle, persists
the choice in local storage, and restores it after reload. Dynamic emitter
lights remain active in both modes.

## Implementation

- Added `ambient-clock.js` with an explicit persisted opt-in key.
- Added a fixed `MIDDAY_AMBIENT` grade and mode-aware sampling while retaining
  the existing keyframes and cycle.
- Wired PerspectiveRenderer to read the preference each frame.
- Added one “Day/night cycle” checkbox to the existing settings surface.

## Changed files

Implementation is limited to the ambient config, lighting/perspective
renderers, existing settings component, unit tests, and two task captures.
The repository has no `src/components/ui/Settings*.vue`; the actual imported
settings surface is `src/components/slots/Settings.vue`. This is a mechanical
spec-path correction; no architect-owned SPEC file was changed.

## Verification

- Focused lighting/settings tests: 8 passed.
- Full `npm run test:unit` on the implementation revision: 117 files / 752
  tests passed.
- Build passed as part of the browser smoke attempt.
- Direct Playwright probe on alternate port 6511 passed. Raw structured
  output:

  ```json
  {
    "defaultChecked": false,
    "defaultStored": null,
    "enabledStored": "true",
    "reloadStored": "true"
  }
  ```
- Captures: `capture-default-settings.png` and `capture-cycle-enabled.png`.
- `git diff --check`: passed.
- Standard `npm run smoke:browser` could not complete because the pre-existing
  port-6500 process occupied the pinned port; it was not terminated.

## Manual checks

The captures show the existing settings pane with the toggle unchecked by
default and checked after opt-in. The alternate-port browser probe exercised
the settings surface, storage write, and a fresh reload read.

## Specification deviations

The spec’s `src/components/ui/Settings*.vue` path does not exist in this
checkout; `src/components/slots/Settings.vue` is the live component imported by
the application. The implementation uses that exact existing surface and adds
only one toggle row.

## Risks and limitations

The pinned smoke wrapper remains environment-blocked by the existing listener
on port 6500. The cycle and emitter-light code remains available for future
owner direction.

## Questions for Fable or the owner

None beyond the mechanical settings-path discrepancy documented above.

## Integration notes

Architect accepted the task at Fable commit `0b12a0a`. Integrated in the
dedicated branch `codex/integrate-TASK-0033` as `b494c1fe` and `91f61d92`.
The full unit suite passed at 119 files / 761 tests and the production build
passed. The pinned smoke wrapper reached the existing port-6500 owner
listener but its API probe received the SPA HTML, so the listener was not
terminated or mutated; rerun on a free pinned port before the next browser
release gate.
