---
id: QUESTION-0007
task: TASK-0042
status: RESOLVED
asked_by: Codex / Luna browser loot-moment worker
---

# TASK-0042 presentation seam

TASK-0042's owned paths include `src/core/player/events/**` and
`src/components/ui/Loot*.vue`, but no `Loot*.vue` exists or is mounted in the
current client. The required ground beam/highlight, world-space name label,
and pickup prompt are rendered by the canvas/perspective path and the overlay
layout (`src/core/rendering/perspective-renderer.js`,
`src/components/layout/GameContainer.vue`), both outside the task's owned
paths. A component added under the allowed glob would be unreachable and
could not satisfy D-115.

## Decision needed

Choose one:

1. Expand TASK-0042 ownership to include the minimal renderer and
   `GameContainer.vue` wiring needed to mount a `LootMoment.vue` overlay, with
   the exact files listed in a numbered review correction; or
2. Define the existing Chatbox + sprite/context-menu path as the accepted
   presentation for this slice and narrow the acceptance criterion accordingly.

No first-drop or UI source changes were made while this seam is unresolved.

## Resolution

Superseded by the accepted and integrated TASK-0042 implementation. The
architect-expanded mounted presentation seam is now repository history; this
question is not a current blocker at `d2423873`.
