---
task: TASK-0040
state: REVIEW_REQUESTED
branch: codex/TASK-0040-first-encounter-readability
commits:
  - 8abad0bf
  - cf9282c1
base_commit: 25ecd77f
---

## Executive summary

The first stone/warren delve now teaches combat in a readable sequence: one
melee opener arrives near the entry, later threats are kept away from the
landing zone, early packs cap at 1/2/3 and later packs at 4, and Ashen
Marksmen remain melee-range shells until the scene reaches two kills. The
server also repairs rounded-tile player/monster and monster/monster overlaps.

## Implementation

- Added `server/core/combat/encounter.js` as the server-authoritative
  first-delve rule module.
- Added the D-114 composition/spacing/cadence table, including derived
  seconds-to-contact values.
- Integrated encounter metadata into map generation and clone paths.
- Added scene-wide kill progression and ranged unlock behavior without a
  protocol or client change.
- Kept the existing monster roster; the opener is the existing Dread Vanguard.
- Moved first-delve treasure/reward placement after the three authored early
  rooms so reward pressure does not undercut the opening lesson.

## Changed files

- `server/core/combat/encounter.js`
- `server/core/combat/index.js`
- `server/core/map.js`
- `tests/unit/encounter-readability.spec.js`
- `orchestration/tasks/TASK-0040-first-encounter-readability/captures/first-delve-seed-90140.md`

## Interfaces

No client or public protocol shape changed. Kill progression is included in
the existing slay progression context as `encounterKills` and
`encounterRangedUnlocked`.

## Verification

- `npx vitest run tests/unit/encounter-readability.spec.js tests/unit/instance-balance.spec.js` — 2 files, 15/15 passed.
- Broader combat/encounter focused run — 8 files, 85/85 passed.
- `npm run test:unit` — 120 files, 766/766 passed.
- `npm run playtest` — 31/31 scenarios passed on the final rerun.
- `git diff --check` — passed for task changes.

## Manual checks

The deterministic transcript for seed `90140` records the Dread Vanguard at
entry radius 7, the next actor at radius 15, ten locked Marksmen after kill
one, all Marksmen unlocked after kill two, treasure room index 5, and three
overlap repairs resulting in four unique actor tiles. The focused suite also
checks known pre-fix seeds where a Marksman, support, or boss could otherwise
be the first reachable threat.

## Specification deviations

Evidence is a deterministic headless transcript rather than a browser
screenshot. The spec's required gates are green; D-115 still requires the
architect to play the actual first delve.

## Risks and limitations

The separation pass runs on the existing combat scheduler, so a rounded-tile
overlap can persist until the next approximately 150 ms combat tick. Ranged
unlock is scene-wide at two kills, while later actors remain physically
isolated until reached.

Generated `docs/loop-journal.md` playtest lines remain uncommitted and outside
the task scope.

## Questions for Fable or the owner

None; architect D-115 hands-on play is the acceptance gate.

## Integration notes

Requires architect review before integration. Integrate `8abad0bf` and
`cf9282c1` together from the worker branch. This is server/core-only and is
disjoint from pending TASK-0035 native and TASK-0036/0037 browser handoffs.
