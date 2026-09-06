---
task: TASK-0108
state: CLAIMED
coordinator: kimi-work
worker: kimiwork-subagent-1
branch: kimiwork/TASK-0108-ranged-rev3
worktree: Z:\Code\Games\delaford\kimiwork_verdigris\.worktrees\task-0108
base: e7b65360
spec_revision: 3
started_at: 2026-09-05T22:41:58-07:00
---

# STATUS — TASK-0108 Readable ranged combat successor (rev 3)

CLAIMED by kimi-work (worker kimiwork-subagent-1) at
2026-09-05T22:41:58-07:00, base e7b65360 (program head at claim).

## Scope note (coordinator constraint)

The SPEC's owned_paths include `native/client/presentation_state.cpp`,
`native/client/render_list.hpp`, `native/client/main.cpp`, and
`native/tests/presentation_events_tests.cpp`, but the `native/client/**`
surface is under an ACTIVE exclusive lease by another agent (Cursor, HUD
wave) this cycle. This claim therefore implements only:

- `native/src/core.cpp`, `native/include/verdigris/core.hpp`
- `native/src/networking.cpp`
- `native/tests/core_tests.cpp`, `native/tests/networking_tests.cpp`
- this task folder

The client-presentation sub-part of the SPEC outcome (client-visible
Telegraph render op for the ranged warning + the
`presentation_events_tests.cpp` lock) is DEFERRED pending release of the
Cursor `native/client/**` lease. Exactly what remains will be enumerated in
REPORT.md.

## Progress log

- 2026-09-05 22:41 -07:00 — Claimed. Surveyed base: ranged volley sim already
  exists at e7b65360 (windup event currently reuses WorldCombatEvent type
  "telegraph" and rides `monster:telegraph` on the wire — the exact D-129
  violation rev 3 must fix). JS payload parity source read:
  `server/core/entities/monster/combat-controller.js:212-223`
  (`world:projectile` keys: fromX, fromY, toX, toY, travelMs, kind).
