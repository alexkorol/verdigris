---
task: TASK-0020
state: INTEGRATED
branch: codex/native-reconstitution
commits:
  - e5d87a5
  - b73ca16
  - 174d769
base_commit: 42297ed
---

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

- `npm run test:unit` — PASS: 114 files, 742 tests (current checkpoint; the
  implementation run recorded 741 before the adjacent test-count change).
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

## Current checkpoint follow-up

After submission, a clean aggregate run reached 25/31 but showed additional
cross-scenario timing failures. Each affected scenario was then rerun against
its own fresh server and port:

- `first-goal` — PASS
- `gear-outcomes` — PASS
- `house-treasury` — PASS
- `mortality` — PASS
- `quest` — PASS
- `zones` — PASS

The failures are not reproducible in isolated runs, so the aggregate result
remains recorded as harness timing noise rather than a product acceptance
result.

## Revision 1 — replacement-session race

The architect's 42fd837 review identified a real asynchronous lifecycle race:
`world.addPlayer` replaces an existing same-UUID session, while the old
disconnect's awaited save later called UUID-based `world.removePlayer`, which
could remove the replacement object. That explains the clustered zone and
`dev:state` failures after rapid reconnects.

The fix makes `WorldManager.removePlayer` identity-safe: it removes only the
exact player object that is still registered, and filters only that object's
scene membership. A two-case unit regression test covers both replacement
preservation and ordinary current-player removal.

### Revision verification

- `npm exec vitest run tests/unit/world-player-lifecycle.spec.js tests/unit/ws-message-handler.spec.js tests/unit/unarmed-combat.spec.js --reporter=dot` — PASS (40 tests).
- `npm run test:unit` — PASS (115 files, 744 tests, including the two new lifecycle tests).
- `npx eslint server/core/world.js tests/unit/world-player-lifecycle.spec.js server/Delaford.js server/core/entities/monster/combat-controller.js server/core/entities/player/movement-handler.js server/core/player.js tests/unit/ws-message-handler.spec.js tests/unit/unarmed-combat.spec.js` — PASS.
- `git diff --check` — PASS.
- `npm run playtest` on fresh port 6520 — PASS (31/31 scenarios).

The prior post-fix run reached 30/31 only because the known timing-sensitive
gear comparison missed its 13% threshold; the clean rerun passed all 31.

## Integration

Architect review `0c69920` accepted revision 1 after independently verifying
the replacement-session race fix, `npm run test:unit` (744/744), and
`npm run playtest` (31/31). The implementation was already present on the
program tip as `b035b56`; no additional source merge was required.
