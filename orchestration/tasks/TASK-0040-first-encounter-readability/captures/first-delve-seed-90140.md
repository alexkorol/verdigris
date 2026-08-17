# Deterministic first-delve transcript — seed 90140

Generated 2026-08-17 from the server-authoritative `GameMap.generateInstance`
and encounter transition helpers. This is a headless evidence capture; it does
not claim client visual validation.

## Composition transcript

| Beat | Authoritative observation |
|---|---|
| Entry | Stone warren depth 1; entry `(99, 100)`; 34 instantiated runtime monsters |
| First contact | One common `Dread Vanguard`, behavior `melee`, entry radius 7, attack range 1, 900 ms step interval, 1500 ms attack interval, 320 ms windup |
| Isolation | Nearest later actor is at entry radius 15; no second actor occupies the opening envelope |
| Learn | Runtime stage at 0 kills: 1 active actor; every later actor has its AI update paused and is filtered from player combat targeting |
| Dormant Marksman probe | Runtime position remained `(84,107)` after an update attempt; update returned `false`; no patrol, targeting, melee, support, or aura action ran |
| Win | Scene kill 1 activates exactly 2 more melee actors (3 active total); 0 active Marksmen |
| Pressure | Scene kill 2 activates exactly 3 more actors (6 active total); the first Marksman restores ranged behavior at range 5, 1100 ms step interval, 1900 ms attack interval, and 480 ms windup |
| Branch hold | Scene kill 3 leaves 6 active actors; reward-stage packs remain dormant even if their procedural rooms lie on another branch |
| Reward | Scene kill 4 activates the remaining 28 actors and unlocks 8 later Marksmen; later packs remain capped at 4 and treasure room index 5 follows the three early rooms |
| Separation fixture | One player plus three monsters started on `(2, 2)`; 3 corrections produced four unique tiles: `(2,2)`, `(1,1)`, `(2,1)`, `(3,1)` |

The runtime probe constructed the actual generated definitions as `Monster`
instances. Its kill-threshold sequence alternated two party members. The scene
reached kill count 2 and unlocked the pressure-stage Marksman, demonstrating
that progression belongs to the delve rather than one player's local counter.
At kill 4, reward-stage Marksmen unlocked even though the scene-wide ranged
flag was already true; the gate is evaluated per actor as stages activate.

## D-114 first-delve pressure table

Distances are tiles. Timings are milliseconds unless stated otherwise.

| Constant | Value | Coherence / purpose |
|---|---:|---|
| opening pack cap | 1 | one target for the learn beat |
| early pack caps | 1, 2, 3 | learn → win → pressure |
| later pack cap | 4 | bounded reward/later-pressure beat |
| ranged unlock kills | 2 | one complete melee win before ranged pressure |
| opening spawn radius | 7 | just outside the existing six-tile landing safe radius |
| later monster entry radius | 15 | isolates the opening contact from procedural room overlap |
| melee aggression / pursuit | 8 / 9 | existing close-pressure envelope |
| melee contact / step interval | 1.6 / 900 | existing continuous melee contact and brute speed |
| melee attack interval / windup | 1500 / 320 | readable existing melee cadence |
| melee seconds to contact | 5.76 s | `(8 - 1.6) × 0.9` from aggression edge |
| ranged aggression / pursuit | 8 / 11 | applied only after the kill gate |
| ranged preferred / minimum range | 5 / 2 | existing Marksman spacing |
| ranged step interval | 1100 | existing mystic movement speed |
| ranged attack interval / windup | 1900 / 480 | readable ranged cadence after unlock |
| ranged seconds to firing range | 3.30 s | `(8 - 5) × 1.1` from aggression edge |
| spawn / player tile separation | 1 / 1 | no shared rounded tile |

## Development-only playtest inspection

The protocol harness runs with `NODE_ENV=development` and inspects individual
actors by teleporting one tile beside them. The encounter runtime recognizes
only that handler's fresh zero-length marker (`interrupted: true`,
`walkId: null`, `duration: 0`, null direction and path indices, unblocked, and
at most one second old), consumes it once, and activates one staged actor
within three tiles. Direct WASD always carries a direction and nonzero duration
when successful (or `blocked: true` when rejected), so ordinary interrupted
movement does not match. The transient marker flag is removed immediately, and
production remains strictly kill-driven.

## Driven WebSocket transcript

`drive-first-delve.mjs` connected as a fresh guest through the real WebSocket
envelope, entered Old Barrow through the party/zone handlers, and used real
primary attacks and authoritative monster deaths. Development teleports only
shortened traversal to the already active target. One captured run printed:

```text
initial-payload roster=33 marksmen=10
scene-kills=0 opener=Dread Vanguard rarity=common {"living":33,"active":1,"dormant":32,"activeMarksmen":0,"rangedMarksmen":0}
scene-kills=1 defeated=Dread Vanguard {"living":32,"active":2,"dormant":30,"activeMarksmen":0,"rangedMarksmen":0}
scene-kills=2 defeated=Dread Vanguard {"living":31,"active":4,"dormant":27,"activeMarksmen":1,"rangedMarksmen":1}
scene-kills=3 defeated=Dread Vanguard {"living":30,"active":3,"dormant":27,"activeMarksmen":1,"rangedMarksmen":1}
scene-kills=5 defeated=Dread Vanguard {"living":28,"active":28,"dormant":0,"activeMarksmen":10,"rangedMarksmen":10}
```

The fourth attack cleaved two weakened win/pressure actors, so the observed
scene counter crossed the reward threshold from 3 to 5 rather than stopping at
4. The runtime still demonstrated each boundary: one common melee opener;
two live melee actors after the first death; a live ranged Marksman only after
the second; no reward actors at three deaths; and the complete remaining roster
active after crossing four.

This protocol run also exposes an architect-pending presentation limitation.
The initial `buildScenePayload` serializes the full instantiated roster even
though only one actor is server-active. The run above sent 33 actors including
10 Marksmen; deterministic seed 90140 contains 34 including 9 Marksmen. They
remain dormant and untargetable, but they are visible and collidable on the
client until their kill threshold. Filtering that synchronous payload belongs
in `server/core/world-transitions.js` or `Monster.toJSON`, outside TASK-0040's
owned paths.

## Reproduction

The assertions are executable in:

```text
npx vitest run tests/unit/encounter-readability.spec.js
```

The driven excerpt can be reproduced with a development server on port 6520:

```text
PLAYTEST_WS_URL=ws://localhost:6520 node orchestration/tasks/TASK-0040-first-encounter-readability/captures/drive-first-delve.mjs
```

That suite repeats the physical-first-contact check over seeds
`3, 5, 9, 12, 21, 37, 90140`; the first six were selected because the
pre-fix generator could place a Marksman, support actor, or boss closer to the
entry than the nominal opening monster.
