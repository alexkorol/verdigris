---
task: TASK-0040
state: REVIEW_REQUESTED
branch: codex/TASK-0040-first-encounter-readability
commits:
  - 8abad0bf
  - cf9282c1
  - 049be9b7
  - d30a1f1c
  - 69995508
  - e0dacfc8
  - 6295298a
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

- `npx vitest run tests/unit/encounter-readability.spec.js tests/unit/instance-balance.spec.js` — 2 files, 16/16 passed.
- Broader affected playtest subset — 4/4 passed.
- `npm run test:unit` — 120 files, 767/767 passed.
- `npm run playtest` — latest full run 25/31; the six failed scenarios
  (`first-goal`, `house-treasury`, `mortality`, `party-stories`, `quest`, and
  `zones`) each passed immediately when rerun in isolation. All
  TASK-0040-sensitive scenarios passed in the latest run.
- `git diff --check` — passed for task changes.

## Manual checks

The deterministic runtime transcript for seed `90140` records the common
Dread Vanguard at entry radius 7, effective 900 ms movement / 1500 ms attack /
320 ms windup, the next actor at radius 15, one active actor at zero kills,
two additional melee actors at kill one, three pressure actors and the first
ranged unlock at kill two, no reward-stage activation at kill three, all
remaining actors at kill four, treasure room index 5, and three overlap repairs
resulting in four unique actor tiles. The focused suite also checks known
pre-fix seeds where a Marksman, support, or boss could otherwise be the first
reachable threat, plus the exact development-only teleport marker used by the
existing playtest harness.

The real WebSocket driver in the capture directory produced this excerpt:

```text
initial-payload roster=33 marksmen=10
scene-kills=0 opener=Dread Vanguard rarity=common {"living":33,"active":1,"dormant":32,"activeMarksmen":0,"rangedMarksmen":0}
scene-kills=1 defeated=Dread Vanguard {"living":32,"active":2,"dormant":30,"activeMarksmen":0,"rangedMarksmen":0}
scene-kills=2 defeated=Dread Vanguard {"living":31,"active":4,"dormant":27,"activeMarksmen":1,"rangedMarksmen":1}
scene-kills=3 defeated=Dread Vanguard {"living":30,"active":3,"dormant":27,"activeMarksmen":1,"rangedMarksmen":1}
scene-kills=5 defeated=Dread Vanguard {"living":28,"active":28,"dormant":0,"activeMarksmen":10,"rangedMarksmen":10}
```

The fourth attack cleaved two already-weakened actors, so the observed counter
crossed 3→5 rather than stopping at 4; every stage boundary was still
observed. Direct WASD fails the dev inspection predicate, while the exact
fresh dev-handler marker activates only the inspected actor once.

Independent validator re-ran the exact-tip full playtest three times: 17/31,
30/31, and 28/31. The 30/31 run missed the `gear-outcomes` 1.15× threshold by
0.03 seconds; that scenario passed immediately in isolation. The final run's
additional build-divergence/session-arc failures also passed in isolated
affected-scenario checks. This remains a real SPEC gate failure, not a claimed
green result, and is returned to the worker/architect for disposition.

## Specification deviations

Evidence includes a real WebSocket-driven runtime transcript rather than a
browser screenshot. The server's normal path is strictly kill-driven. A
narrowly gated `NODE_ENV=development` seam consumes only the exact
dev-teleport marker (`interrupted=true`, `walkId=null`, `duration=0`, null
direction/path indices, unblocked, fresh and within three tiles) once so the
pre-existing playtest can inspect an isolated boss/loot comparison; ordinary
movement and production cannot activate staged actors. D-115 still requires
the architect to play the actual first delve.

## Risks and limitations

The separation pass runs on the existing combat scheduler, so a rounded-tile
overlap can persist until the next approximately 150 ms combat tick. Staged
actors remain instantiated/rendered because client paths were forbidden: the
synchronous initial `buildScenePayload` still serializes the full dormant
roster (driven run 33 actors/10 Marksmen; deterministic seed `90140` 34/9),
so dormant actors are visible and collidable on the client even though they are
server-dormant and untargetable. Filtering that payload requires
`server/core/world-transitions.js` or `server/core/monster.js`, outside this
task's owned paths, and is explicitly pending architect scope direction. They
are filtered from combat and their AI is paused before the first 600 ms
monster tick by the existing 150 ms scheduler. Ranged unlock is scene-wide at
two kills, while later actors remain physically isolated until reached.

Generated `docs/loop-journal.md` playtest lines remain uncommitted and outside
the task scope.

## Questions for Fable or the owner

Architect decision required: either extend ownership to the synchronous scene
payload path to hide dormant actors until activation, or ratify the documented
server-dormant/client-visible limitation. D-115 hands-on play remains the
acceptance gate.

## Integration notes

Requires architect re-review before integration. Integrate the complete chain
through `6295298a` from the worker branch. This is server/core-only and is
disjoint from pending TASK-0035 native and TASK-0036/0037 browser handoffs.
