# Deterministic first-delve transcript — seed 90140

Generated 2026-08-17 from the server-authoritative `GameMap.generateInstance`
and encounter transition helpers. This is a headless evidence capture; it does
not claim client visual validation.

## Composition transcript

| Beat | Authoritative observation |
|---|---|
| Entry | Stone warren depth 1; entry `(99, 100)`; 33 monsters total |
| First contact | One `Dread Vanguard`, behavior `melee`, entry radius 7, attack range 1, windup 320 ms |
| Isolation | Nearest later actor is at entry radius 15; no second actor occupies the opening envelope |
| Learn | Room cap 1; ranged pressure disabled |
| Win | Room cap 2; all 10 generated Marksmen remain server-locked after scene kill 1 |
| Pressure | Scene kill 2 changes `rangedUnlocked` from false to true and unlocks the 10 existing Ashen Marksmen |
| Reward | Later packs remain capped at 4; treasure room is room index 5, after the three authored early rooms |
| Separation fixture | One player plus three monsters started on `(2, 2)`; 3 corrections produced four unique tiles: `(2,2)`, `(1,1)`, `(2,1)`, `(3,1)` |

The kill-threshold probe used two party members, one credited kill each. The
scene reached kill count 2 and unlocked ranged behavior, demonstrating that
pressure progression belongs to the delve rather than to one player's local
counter.

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

## Reproduction

The assertions are executable in:

```text
npx vitest run tests/unit/encounter-readability.spec.js
```

That suite repeats the physical-first-contact check over seeds
`3, 5, 9, 12, 21, 37, 90140`; the first six were selected because the
pre-fix generator could place a Marksman, support actor, or boss closer to the
entry than the nominal opening monster.
