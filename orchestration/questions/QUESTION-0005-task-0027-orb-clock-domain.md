---
question: QUESTION-0005
related_task: TASK-0027
state: OPEN
owner: architect
---

# TASK-0027 night-orb symptom: compositing or capture clock?

## Decision needed

TASK-0024's accepted review attributes the nearly black night HP/MP orbs to
lighting/vignette compositing over the HUD. A read-only audit found a second,
testable explanation in the existing capture harness: it offsets
`performance.now()` by 240 seconds, while `WizardOrbRenderer` receives the
browser's native RAF timestamp. The first orb update can therefore have a
large negative `dt`; the current smoothing expression can produce `NaN`,
which is passed to the orb shader uniforms.

The HUD is already a DOM sibling above the world canvas (`z-index: 70`), and
the orb shader has no world-light input. The existing night capture should
therefore be treated as unproven until a clock-safe capture reproduces the
black-orb result.

## Evidence

- `orchestration/tasks/TASK-0024-browser-25d-phase3/captures/capture.mjs`
  patches the clock around the capture setup.
- `src/core/hud/wizard-orb-renderer.js` initializes against the patched clock
  but updates from RAF timestamps, with no lower clamp on `dt`.
- `src/components/layout/GameContainer.vue` places `GameHUD` above the world
  canvas; `src/core/rendering/perspective-renderer.js` grades only the world
  buffer.

An executable Node calculation with a representative `dt = -239.9s` gives
`1 - exp(-dt * 12) = -Infinity` and `0 * -Infinity = NaN`, matching the
renderer path when the initial fill is already equal to the target fill.

## Recommendation

Before changing compositing, recapture midday and night with a clock-safe
method and compare orb-region pixels. If the symptom persists, TASK-0027 can
implement and prove the required HUD-safe pass order within its owned
rendering paths. If it disappears, preserve the rendering pass order and
route the clock mismatch to a follow-up task because the current TASK-0027
spec forbids `src/core/hud/**` and capture-harness edits.

## Blocking status

Not blocking: the worker can perform the clock-safe reproduction and the
remaining DoF audit without an architectural decision.
