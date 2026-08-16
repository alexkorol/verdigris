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
The intended House layer also includes meta-progression, passive income,
asynchronous trading, and currency exchange. Their rates, sinks, and authority
boundaries remain open until the economy pass.

Verdigris also has a persistent Legends direction: storied items and monsters
should leave records and influence future loot/spawn pools, creating a House
history that can outlive any one Scion. The first implementation may be a
small deterministic record; it must not become an unbounded simulation hidden
inside the combat loop.

## Campaign and endgame

The route from campaign start to endgame targets roughly 6–30 hours depending
on how many optional branches a player completes. It is completed once per
House per season; later Scions level without repeating the mandatory route.
Campaign content is a multizone graph spanning several acts, with optional
branches that may grant specialization directions, item access, knowledge,
league mechanics, routes, or starting opportunities. The repeatable endgame
lets the player choose areas, goals, mechanics, item targets, trophy targets,
and build experiments.

## Actors and combat

Players, monsters, and future mercenaries share one microscopic actor/stat
schema: level, Strength, Dexterity, Intelligence, Life, resources, attacks,
defense, movement, attack speed, resistances, equipment, and effects. Human
enemies are differently built/equipped actors, not a second inflated universe.
Elite difficulty comes from level, build, equipment, actions, and support rather
than arbitrary billion-point Life.

The control reference is systemic depth plus deliberate movement: WASD, mouse
aiming, left/right mouse actions, Q/E/R skills, and Space (or an equivalent)
movement ability. Z toggles loot filtering/highlights, X picks up the nearest
item, and gold auto-picks up. A native client has no context-menu control
scheme. Click-to-move is not primary and the hotbar is not piano-sized. Combat
uses readable physical space (swings, thrusts, slams, leaps, guarded actions,
buffs, war cries, combos, ranged attacks, and magic) with strong impact
feedback.

Player-facing progression includes inventory, stats, a passive skill tree,
equipment that is reflected by the in-world character, and loose quest threads
that guide campaign exploration without turning the durable loop into a rigid
checklist.

The presentation layer should support a pane system (including character +
inventory and trade + inventory diptychs) and two minimap modes: a small side
map and a large overlay. Transparency, zoom, and side placement are options.
Orbs, skill bar, and surrounding UI share one visual language.

Monsters use the same stat and element vocabulary as players. The world needs
pack spawning, rarity, uniques, scarce equipment drops, more generous trophies
and crafting materials, and a fast-travel or town-portal path with an explicit
risk model.

## Magic and future mechanics

Magic is part of Verdigris. Bronze Age does not imply low magic or muted
presentation. The WIZARD Arcane Lattice is documented in
`docs/product/WIZARD_ARCANE_LATTICE_REFERENCE.md`; it is a reference, not a
license to invent a generic mana wizard in this sprint.

Seasonal mechanics must attach to an instance, observe simulation events, own
state, alter risk, and generate distinct rewards without rewriting combat,
items, extraction, or House systems. One tiny automated demonstration proves
this extension boundary.

## WIZARD components intended to mesh

WIZARD is a toolbox, but several components are deliberate Verdigris
integration candidates rather than reference-only curiosities:

- **Vessels of Life & Mana (orbs):** the WebGL2 life/mana presentation and
  status-effect feedback can become a native HUD/presentation adapter. The
  authoritative actor resource/effect state stays in the simulation; the
  shader does not become game logic.
- **Brands & Bonds inventory:** the vessel-slot inventory, stable item identity,
  Brands, Bonds, attunement, awakening, scars, and history align with
  Verdigris's item constitution. Integrate the rules through the native item
  model and provide a platform-appropriate inventory UI; do not blindly copy
  the browser demo's React shell.
- **Verdigris splash:** the floating pre-iron world and atmosphere are an
  intended menu/splash presentation foundation. It may feed a native splash or
  title scene while remaining outside the headless simulation.
- **Cartographer map generation:** the dependency-free seeded generator is a
  candidate native content adapter for deterministic instance layouts. Its
  entrance/exit connectivity and spawn guarantees should be validated against
  Verdigris collision, route, and expedition rules before adoption.

These integrations share the invariant that presentation/content adapters
consume commands, snapshots, seeds, and events; they do not reintroduce DOM,
WebGL, or browser state into the core simulation. Exact porting language and
asset licensing remain open decisions.

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
