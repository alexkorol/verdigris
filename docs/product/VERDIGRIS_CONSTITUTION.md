# Verdigris Product Constitution

**Status:** authoritative product guidance for the native reconstitution.

This document outranks inherited Delaford code, obsolete tests, archived plans,
and accidental behavior in the browser reference. The browser game remains
valuable historical material, but nothing crosses into native production merely
because it already exists.

## Product identity

Verdigris is a proprietary native Windows/macOS Bronze Age/pre-iron fantasy
action RPG presented as a readable 2.5D billboard-style world. Adults, grounded
materials, dangerous expeditions, bright and spectacular magic, and memorable
items are all in scope. Medieval defaults, chibi proportions, and generic
starter kits are not.

## The durable loop

The fundamental loop is:

```text
choose a specific expedition goal
→ enter an instance or chain of instances
→ fight and survive
→ acquire items, trophies, materials, and knowledge
→ push farther or return safely
→ extract value to the House
→ craft, unlock, prepare, and grow the House
→ continue or replace the current Scion
```

Players usually leave with a concrete goal (a trophy, material, route,
specialization, item family, or House upgrade), not a checklist quest.

## Nested progression

- **Scion:** one mortal character with an individual level, freeform build,
  equipment, and expedition history. Scions may survive, retire, wait in
  reserve, or die; eventual death is expected.
- **House:** the persistent player identity: a lineage, tribe, political family,
  or extended kin group, not a literal building. It owns stores, crafting,
  route knowledge, unlocks, relics, training, and future starting opportunities.
- **Season:** an optional future league layer. Active progression may reset while
  a limited historical continuity remains. The inheritance rule is unresolved.

## Items, trophies, and history

Items are a primary reason to play. Equipment changes reach, cadence, attack
form, movement, defense, resources, techniques, or build direction; loot is not
just interchangeable percentage upgrades. Expeditions produce trophies,
materials, relic components, and items. Unextracted value can be lost on death,
creating a meaningful return decision.

Items have stable identities and can gain history through ownership, use,
survival, loss, and rediscovery. Brands and Bonds are the reference direction;
their complete formula is intentionally not finalized here. Death may return a
carried item to the wider loot pool as a scarred, bonded, or otherwise
transformed relic candidate, with a mixture of predictable recovery and
uncertainty. A successor does not receive a dead Scion's full inventory.

House crafting is a social/material process belonging to the House, not a
generic hideout bench. The House turns extracted value into durable advancement.

## Campaign and endgame

The campaign is approximately 6–12 hours for an average player and is completed
once per House. Later Scions level without repeating the full mandatory campaign.
Campaign content is a graph of connected instances, routes, and optional
branches. Branches may grant specialization directions, item access, knowledge,
mechanics, routes, or starting opportunities. The repeatable endgame lets the
player choose areas, goals, mechanics, item targets, trophy targets, and build
experiments.

## Actors and combat

Players, monsters, and future mercenaries share one microscopic actor/stat
schema: level, Strength, Dexterity, Intelligence, Life, resources, attacks,
defense, movement, attack speed, resistances, equipment, and effects. Human
enemies are differently built/equipped actors, not a second inflated universe.
Elite difficulty comes from level, build, equipment, actions, and support rather
than arbitrary billion-point Life.

The control reference is systemic depth plus deliberate movement: WASD, mouse
aiming, left/right mouse actions, a compact nearby skill set, and Space (or an
equivalent) movement ability. Click-to-move is not primary and the hotbar is not
piano-sized. Combat uses readable physical space (swings, thrusts, slams,
leaps, guarded actions, buffs, war cries, combos, ranged attacks, and magic)
with strong impact feedback.

## Magic and future mechanics

Magic is part of Verdigris. Bronze Age does not imply low magic or muted
presentation. The WIZARD Arcane Lattice is documented in
`docs/product/WIZARD_ARCANE_LATTICE_REFERENCE.md`; it is a reference, not a
license to invent a generic mana wizard in this sprint.

Seasonal mechanics must attach to an instance, observe simulation events, own
state, alter risk, and generate distinct rewards without rewriting combat,
items, extraction, or House systems. One tiny automated demonstration proves
this extension boundary.

## Native architecture invariant

The native runtime separates core simulation, platform/client, renderer,
networking, persistence, content, and tools. The simulation is fixed-step,
seeded where practical, headless, and command/event driven:

```text
presentation requests actions
→ simulation resolves actions
→ simulation emits results
→ presentation displays results
```

The first implementation is intentionally small: House, Scion, shared actors,
movement, melee, one graph, one trophy, one generated item, pickup/equip,
extraction, death/loss, one relic candidate, one route unlock, and one seasonal
extension. It is a proof of the loop, not a bulk port or general engine.

## Delaford firewall

Delaford is historical source material. Bronze dagger starters, generic starting
coins, fishing/cooking/mining/smithing, old graphics, obsolete catalogues,
medieval terminology, and tests that force them back are denied by default.
Intentional carry-over must be named in the legacy allowlist.
