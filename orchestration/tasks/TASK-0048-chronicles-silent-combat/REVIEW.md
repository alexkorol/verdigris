---
task: TASK-0048
verdict: ACCEPTED
reviewed_commits:
  - 810ddd92
  - 3fb4f0bb
---

## Architect verification (2026-08-17 ~19:55)

- **Scope**: one new playtest scenario + task evidence; ZERO product
  code (verified empty diff on server/, src/, native/, package.json).
  Adding a scenario was explicitly allowed and encouraged by the spec.
- **The disproof stands up.** The baseline wire capture from the
  accepted 0046 arc tip shows 14 valid attack frames sent while the
  driver sat ~10 tiles from the only active monster — non-contact, not
  a Chronicles state rejection. Evidence-based, reproducible, and it
  corrects an ACCEPTED review's finding with data. That is the system
  working.
- **Architect gates**: `chronicles-first-combat` rerun personally —
  1/1 solo (kill in 605ms), 3/3 in direct sequence with the two
  suspect scenarios, and full suite **32/32** (109ms peak ambient
  lag). An intermediate 30/32 run at 150ms peak lag flaked
  `first-goal` (8.3s vs 8s authored) and `house-treasury` (gold
  assert) — unrelated to this branch (both pass in direct sequence);
  logged as a board watch item.
- **Deviation accepted**: no authentic negative is possible when the
  investigated bug does not exist; the discriminating scenario IS the
  standing regression guard the spec wanted.

## Rulings folded in

1. **0046 blocker 1 reclassified**: driver artifact (non-contact),
   not a product regression. The 0046 acceptance stands; its arc
   method gains a requirement — future drivers must verify target
   contact (read the authoritative actor position) before attributing
   combat silence to the game.
2. **Mana copy ratified (product steward)**: the resource gate is
   intentional and its NUMBERS stay owner/D-114 territory, but the
   rejection copy SHOULD be directive — state the missing amount and
   the recovery cadence ("Need 12 more mana — recovering 2 every
   2s"). Queued for the next UI wave; no balance change authorized.

## Permanent gain

The suite is now 32 scenarios and the Chronicles mortal-oath path has
first-class combat coverage — the exact blind spot 0046 exposed.

Integration approved; merged at `edc9c794`. Ships in the next batch.
