# Vision

## Status (updated 2026-09-03)

**The world web at the village gates** (docs/crossroads-world-web.md): the
rebuilt 2.5D Delaford Village remains the campaign's town, and the
House-charted world web now hangs off four road gates beside its portals — a
procedurally generated web of zones (four roads, branching tiers, a Warden
per zone gating the next, ~15-minute zone persistence, zones private to a
player/party), plus wagon pitches, the daily road purse, deposits, and House
outfitting on the plaza. The full Crossroads conversion (trading-hub town,
sanctuary truce-ground, wagon pane UI in the client shell) is the planned
follow-up; the server systems and playtests for it are already live. One
rule holds: walked ground holds; forsaken ground weathers.

Landed from the WIZARD prototypes (skill tree, Vesselforge inventory engine,
Chronicles account creator):

- **Skill tree** — nine-ring Verdigris geometric lattice (271 nodes + 34
  subtree nodes behind six gateway annexes), unified 123-point economy
  (100 from levels, 23 from quests; 1 point per node, 1 per path), ring-8
  Signs (birthsign keystones), ring-7 class masteries
  (Champion/Acrobat/Archmage/Spellsword). `src/core/passives/`.
- **Itemisation** — Vesselforge engine + Verdigris content pack ported to
  `server/core/items/vesselforge/`: vessel slots, Brands ✦ / Bonds ◈ /
  Trophies ✧ / Scars ✕, patience crafting, material firing, attunement and
  awakening. The loot pool now draws from an explicit 13-form native Vessel
  catalogue; one roll owns each item's name, material, footprint, ratings,
  resource bonuses, and rarity-colored keyboard-reachable card. Supported
  damage, ward, attributes, life, spirit, and selected Brands affect live
  combat stats. Bloodgroove/Macuahuitl bleed, Long Reach, Surefoot/Sandals
  movement, Riverblessed/Emberward resistance, Atlatl projectile range, Sling
  armour penetration, and Grips attack speed now drive authoritative combat
  and travel. Every mechanics-bearing line in the shipped form and Brand
  catalogue is active; the Spear's narrative-only reach line remains labelled
  Dormant rather than promising an effect it does not own. Existing catalogue
  items remain legacy gear.
  Each native form also has distinct reference-driven item art in its own
  reproducible atlas. Equip/Unequip preserves the generated UUID through the
  server-backed flow. Generated rewards bind after real world admission, merge
  fungible currency into its existing stack, and fall at the player's feet
  without losing identity when the spatial backpack is full.
- **Character identity** — no classes. Every character is a blank with
  10/10/10 base attributes shaped by tree/gear/quests. The Warrior/Rogue/Mage
  picker was reverted; the intended identity layer is Chronicles
  **Houses & Scions** (account = House meta, characters = permadeath scions),
  tracked in `docs/archive/fix-plan-2026-07-04.md` Phase 6.

Sprint of 2026-07-04 (`docs/archive/fix-plan-2026-07-04.md`) delivered: combat feel
(hit tint, unarmed retaliation, bottom xp bar, crisp orbs), pane close
buttons, skill-tree corrections (plain names, correct conduit bias, level-
scaled points), PoE-style quickbar, inventory slot texture + varied drops,
solo zone/instance entry (Adventure menu), and the Chronicles Houses & Scions
persistence model (`src/core/chronicles/houses.js`).

Chronicles creation, authoritative persistence, mortal Scion entombment, and
fallen-heirloom circulation are now playable end to end. The first twelve real
quests form a server-owned progression chain. **Aldwyn's Charge** turns
movement → combat → loot → Adventure entry into onboarding; **Proof of
Temper** follows with an elite hunt, guaranteed native Vessel, equipment
objective, second passive point, House renown, and Scion deed. **The Pale
Crown** then validates a specific named zone, named generated boss, and real
floor-two descent before awarding the third point and campaign deed. **Rot in
the Reeds** carries the chain into Marsh of Reeds, requires the named Rotfather
boss, and completes only after the server returns the expedition to the
surface.

The second act binds the campaign to the procedural world web: **Oath of
Tin**, **The Salt Reckoning**, **The Chalk Vigil**, and **The Copper
Testament** each require authoritative entry, Warden defeat, and a living
return on their exact road. The four-road covenant raises the campaign total
to eight quest points and opens **The Deep Roads** rather than prematurely
sealing the campaign. Tier-two Tin, Salt, Chalk, and Copper commissions then
name and require the Quarry Saint, Brine Widow, Ossuary Bell, and Cinder Judge.
The final living return seals the twelve-point campaign and grants the first
consumable endgame tablet. Road clears are House-persistent, and deeper node
requests are rejected until their actual parent Warden is recorded dead.

The charted endgame now has an atlas-like Wayfinder Mastery layer rather than
only a lifetime clear counter. Barrow, Reeds, Crown, and Thorns tablets each
contribute tier 1-16 first-clear objectives. Mastery belongs to the House,
awards tier-scaled renown once, and raises a bounded next-tier ascent chance;
repeat clears remain valid runs without minting the first-clear reward again.
The post-campaign Framekit journal renders the complete 64-objective ledger and
the selected tablet identifies whether it offers new mastery.

Primary melee combat now has an authoritative three-beat rhythm instead of an
endless identical swing. Cut and Return build into a slower, 160% Finisher
that briefly staggers non-boss enemies; the chain expires after a short pause
or resets when a named skill is used. Exact cadence state travels through the
server protocol and reconnect snapshot. The Framekit quickbar shows the live
step, while the finisher's verdigris flare and larger damage number remain
presentation-only reactions to the resolved server hit.

Monster pack roles now create different decisions in the same authoritative
combat loop. Ranged foes paint a violet one-tile volley destination for 800 ms
and only hit a Scion who remains inside it. Support foes choose the most-wounded
nearby non-boss ally and publish the exact mend and resulting health. The melee
finisher can interrupt a pending volley, making cadence and positioning work
together; the client merely renders the server's target and heal facts.
Nearby packs also move through an obstacle-aware authoritative search: melee
and Wardens pursue, ranged enemies create space before casting, and supports
reform around wounded allies. Accepted 400 ms tile steps travel over the wire
and are interpolated only at paint time, while finisher interruptions now
remove cancelled warnings immediately.

The character sheet now reuses the generated player sprite for its portrait
and each equipped item's real atlas frame; generic slot glyphs remain only for
empty equipment positions.

The delaford-era resource loops (ore mining, furnace/anvil smithing) are
retired: Verdigris is an ARPG, and crafting arrives through the Houses meta
systems instead (Vesselforge brand-searing in town is already live).

The live bestiary and townsfolk are now reference-driven 64px actors as well.
All 15 surface-campaign monsters have named silhouettes, and the 28 Adventure
identities are mapped by seven themes × four combat roles instead of every
generated floor silently showing monster frame zero. Baynard, both merchants,
and the bank gnome likewise retain their own server-selected frames. The
deterministic source-to-atlas contract lives in `docs/actor-art-pipeline.md`.

Party play is now exercised as a real two-client protocol loop: distinct guests
create a party, invite and ready together, enter the same generated instance,
split safely when one member leaves, and return to town. Admission snapshots
also clear readiness before they reach either client. The built-browser smoke
test separately proves canvas and inventory context menus, UI-focus-safe WASD,
skill-tree reopening, zone labels, and physical pointer equip/unequip.

## Release runway toward 1.0

The release gate is `npm run verify`: lint and style checks, the complete unit
suite, production build, every real-server playtest scenario, and the built-game
browser loop. The remaining product work is intentionally narrower than the
original prototype roadmap:

1. **Campaign breadth:** extend the authoritative twelve-commission campaign
   toward the 23 quest points reserved by the passive tree, with named zones,
   bosses, rewards, persistence, and playtest coverage for each arc.
2. **Vesselforge progression:** expose the existing server-owned crafting,
   attunement, and awakening loops through an inspectable player-facing town
   service now that every mechanics-bearing form and Brand line is authoritative.
3. **Inventory depth and access:** implement real nested containers and finish
   keyboard-first grid/equipment navigation; pointer equip/unequip, rotation,
   spatial placement, overflow, and tooltips are already live.
4. **Production ownership:** choose a guest-only launch or bundle the non-guest
   account service, then document and test backup/restore for guest saves and
   Chronicles data.
5. **Operations:** add structured balance telemetry and the minimum safe GM,
   rollback, and recovery tools needed to run a persistent world.

LLM-backed naming remains optional until its hosting, privacy, failure mode,
and offline fallback are decided; deterministic local validation remains the
release-safe baseline.

## Pillars

- **WASD-first ARPG**: keyboard movement/combat with optional mouse context menus.
- **Character Identity**: permadeath, name validation, player-tied loot.
- **Rich Itemisation**: spatial inventory, nested containers, brands & bonds.
- **Shared Stat Ecosystem**: players and monsters use the same attribute pipeline.
- **Party-Based Instances**: towns are persistent hubs; adventures occur in instanced realms.

## High-Level Themes

### Foundation & Tooling
- Upgrade dependencies and build tooling.
- Improve developer experience with one-command setup and tasks.
- Establish CI/testing pipelines and documentation.

### Gameplay Core
- ~~Implement Str/Dex/Int, health/mana, and scaling rules.~~ ✅ shared stat
  pipeline + archetype attribute spreads.
- ~~Create permadeath/cheat-death mechanics.~~ ✅ mortal Chronicles,
  cheat-death, duplicate-hit-safe soft death, and protected instance respawns.
- Add LLM-backed RP naming enforcement.
- ~~Design the skill tree with a Flower-of-Life-inspired layout.~~ ✅
  nine-ring geometric lattice with Signs, masteries, and gateway annexes.

### Inventory & Items
- Build 127 backpack and ragdoll equipment slots.
- Support nested containers (bags, cube) with recursive grids.
- ~~Introduce brands/bonds (prefix/suffix) affix system.~~ ✅ legacy affix
  engine plus the full Vesselforge brand/bond/trophy model.
- ~~Bind items to player identity.~~ ✅ bind-on-pickup plus Vesselforge bond
  kinship (bonds estrange when worn by another archetype).

### UI/UX
- Left stats pane, right inventory pane inspired by PoE.
- Semi-transparent, closable chat overlay.
- Pixel-perfect rendering that avoids sprite squish via letterboxing/scrolling.
- Full keyboard navigation, configurable hotkeys.

### Monsters & Combat
- Shared stat pipeline, monster categories, rarity tiers.
- Balanced combat loop with interpolated movement, responsive named skills,
  and an authoritative three-beat primary cadence with a readable finisher.
- ~~AI behaviors for different archetypes.~~ ✅ close-pressure melee,
  dodgeable ranged volleys, and ally-targeted support mends.

### Networking & World
- Persistent towns with social features.
- Party instancing and semi-random tile-based maps.
- Infinite realm activities (Abyss/Pandemonium analogues).
- Player-modifiable town structures.

### Supporting Systems
- Logging/analytics for balance.
- Live operations tools (GM commands, rollback, event triggers).
- Localization scaffolding.
- Expand the authoritative campaign beyond its first three commissions toward
  the 23-point quest budget reserved by the passive tree.
- Expose Vesselforge crafting, attunement, and awakening as player-facing town
  progression now that all mechanics-bearing form and Brand lines are live.
  Shield block, Keen Eye critical chance, Wealthy loot bonuses, and Beastbane
  damage against explicitly tagged creatures are now live and persisted, with
  explicit `BLOCK`, `CRIT`, and `BANE` combat feedback. Bloodgroove bleed,
  Long Reach, Surefoot movement, and River/Ember resistance are likewise live
  across item derivation, worn totals, combat/movement, protocol snapshots,
  and Framekit presentation. Atlatl projectile range, Sling armour penetration,
  and Grips attack speed complete the same authoritative chain, with `PIERCE`
  feedback and a two-row loadout summary.

## Open Questions
- How to host/secure LLM name validation (local vs remote)?
- Which tech stack upgrades (Vue 3 + Vite?) are feasible short-term?
- Permadeath mitigation currency or shrineshow is it earned?
- How deep should container recursion go (limits to avoid UI chaos)?
- Should towns be per-region or global?

## Milestones (Draft)
1. **MVP Movement & Inventory**: WASD polish, click cancel, 127 inventory skeleton.
2. **Core Stats & Affixes**: character sheets, brands/bonds, monster parity.
3. **UI Refresh**: new panes, chat overlay, responsive layout.
4. **Instance Prototype**: party lobby, one tileset instance, simple loot loop.
5. **Passive Tree Alpha**: partial flower, progression tied to drops/quests.
6. **Permadeath Loop**: death rules, cheat-death prototype, name validation.

This document evolves alongside implementation. Add sections or RFCs as systems mature.
