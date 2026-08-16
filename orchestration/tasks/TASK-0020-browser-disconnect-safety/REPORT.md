# TASK-0020 report — browser disconnect safety (D-109)

## Executive summary

The browser server now treats socket loss as a forgiving persistence boundary.
On close/logout it marks the live Player as disconnecting, cancels pathfinding
and auto-attack, persists the authoritative player snapshot, and only then
removes the Player from the world roster and scene. Monster targeting rejects a
disconnecting player, so a close cannot turn into posthumous damage. Existing
guest and Chronicles persistence paths remain the only save seams; their
instance snapshots already surface at the saved pre-instance/town position,
and Chronicles admission places the next session in the House town.

## Before/after audit

Before this task, `Delaford.close` called `world.removePlayerBySocket` before
`player.update()`. That made cleanup immediate, but persistence happened after
world removal and there was no explicit disconnecting state to stop delayed
combat/movement work. Guest/Chronicles snapshot builders already avoided saving
raw instance coordinates, but the close ordering did not make that boundary
explicit.

After this task, `Delaford.close` finds the player without removing it, marks
the player disconnecting, cancels delayed work, awaits `player.update()`, and
then calls `world.removePlayer`. The socket transport cleanup and party/wagon/
departure notifications still run even when persistence or account logout
reports an error.

## Changed files

- `server/Delaford.js` — persist-before-remove close ordering, duplicate-close
  guard, disconnect teardown.
- `server/core/player.js` — explicit `disconnecting` runtime flag.
- `server/core/entities/player/movement-handler.js` — delayed/direct movement
  guard during disconnect.
- `server/core/entities/monster/combat-controller.js` — disconnecting players
  are not targetable.
- `tests/unit/ws-message-handler.spec.js` — guest, account, Chronicles,
  ordering, and failure-tolerant cleanup coverage.
- `tests/unit/unarmed-combat.spec.js` — pending monster hits are discarded
  during the disconnect window.

No wire envelope, native, prototype, or persistence repository schema changed.

## Verification

- `npm run test:unit` — PASS: 114 files, 741 tests.
- Targeted disconnect/monster tests — PASS: 38 tests.
- ESLint over all changed JS files — PASS.
- `npm run playtest -- gear-outcomes` — PASS on isolated rerun.
- `npm run playtest` — all core scenarios exercised; one timing-sensitive
  `gear-outcomes` comparison fell below its pre-existing 13% polling threshold
  on the full-suite runs (30/31). The same scenario passed independently;
  no D-109 scenario or persistence assertion failed. This is recorded for
  architect review rather than changing the unrelated playtest threshold.

## D-109 behavior coverage

- Persist-before-remove ordering is asserted with a deferred save: the world
  roster remains present while `update()` is pending and is removed only after
  the promise resolves.
- Guest and Chronicles-shaped players both exercise the close path.
- A disconnecting player cannot receive a pending monster hit.
- Movement/path timers are invalidated by the existing cancellation seam and
  direct movement is rejected while disconnecting.
- The existing guest and Chronicles snapshot seams surface instance state to
  town/pre-instance coordinates on the next login; no parallel save path was
  introduced.

## Deviations and risks

No product or wire-protocol deviations. The full playtest suite has a known
timing-sensitive gear comparison that passed in isolation but failed during
two full-suite runs; this is the only non-green acceptance observation and is
not caused by the disconnect code path. Architect review is required.
