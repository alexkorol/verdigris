# Verdigris vision and goal map

This is the durable product checkpoint produced from the owner interview on
2026-08-23. It distinguishes the immediate **Owner Demo** from the complete
native game.

## Product identity

Verdigris is a native C++ Bronze Age / ancient-world action RPG. The House is a
lineage, clan, and economic unit; a Scion is one mortal playable character.
Combat, expeditions, items, House development, death, succession, and item
history form one connected loop.

The browser game and WIZARD modules are reference implementations and source
material. The shipped target is an optimized native client and authoritative
native simulation/server.

## Owner Demo purpose

The Owner Demo is an internal proof build for the sole developer. It must make
the C++ conversion feel real and provide a daily playable base for continued
orchestration. It is not a public founder tier or external tester program.

## First fifteen-minute journey

1. Choose one of four ancient-world faction families.
2. Name the House and first Scion; offer generated alternatives and safe-name
   validation with a deterministic fallback.
3. Choose a prior occupation that lightly nudges Strength, Dexterity, or
   Intelligence without creating a permanent class.
4. Begin immediately in a village-defense crisis.
5. Pick up a plain civilian or work tool and fight toward the village center.
6. Defeat the invader leader at the square, well, or communal building.
7. Gain the first level and open the geometric passive tree.
8. Enter the surviving non-combat town, meet NPCs, trade, and receive direction.
9. Choose between an immediate Scion equipment improvement and the House's
   first durable investment.
10. Enter a larger generated expedition, explore an optional branch, find
    clues about the wider disturbance, gain another level, and unlock a bounded
    spell-lattice choice.
11. Find trophies, reagents, coins, and sparse meaningful equipment; apply one
    or two basic Brands and show slow Bond progress.
12. Return safely, bank value, and visibly advance the House.

## Three unmistakable Verdigris moments

### First level

The prologue boss grants a skill point. The player opens the geometric passive
tree and makes a meaningful allocation. The tree must be an active WIZARD
module, not a placeholder pane.

### Ancient-world identity

Architecture, materials, weapons, clothing, monsters, language, music, and UI
must communicate Bronze Age / ancient fantasy immediately rather than generic
medieval fantasy.

### House or Scion

After the first clear, the player must decide whether to strengthen the current
mortal Scion or invest scarce value in the persistent House. This is the first
proof that Verdigris is not a conventional single-character ARPG.

## Combat identity

- Visceral, fun, and potentially fast; do not slow combat merely to sound
  tactical on paper.
- Ordinary pack clearing can be satisfyingly low-friction because much of the
  depth lives in build planning, equipment, skills, and the passive tree.
- Rares, uniques, bosses, PvP, and specialized encounters may demand layered
  counters and utility choices.
- Core action set: primary attack or spell, movement skill, and utility such as
  guard, slam, buff, debuff, or control.
- Equipped weapons must be visible.
- Swings, thrusts, slams, casts, projectiles, trails, hit reactions, and deaths
  must be readable and satisfying.
- Animated vector or articulated placeholder actors are acceptable initially.
  Static cutouts with invisible attacks are not.

## Items, Brands, Bonds, and circulation

- Equipment drops are sparse and worth inspecting. Avoid loot explosions and
  random junk.
- Ordinary rewards favor copper, silver, gold, trophies, materials, and
  concrete magical reagents.
- Brands are deliberate player-applied modifiers using scarce resources or
  House services.
- Bonds develop through equipping, carrying, using, surviving, and shared
  history. The first session should show only modest progress.
- Items have stable identities and retain ownership, use, loss, recovery, and
  transformation history.
- Unrecovered items may eventually enter a scalable shared circulation pool,
  attach to monsters or zones, and return as announced recovery opportunities.
- The term "storied item" remains provisional.

## Mortality and recovery

- The opening should be forgiving: failure restarts the defense without forcing
  the player through naming screens again.
- Long-term mortal and persistent-character modes remain an open design choice.
- Mortal Scions should have access to distinctive relic or legacy outcomes
  without turning death into optimal farming.
- A missing or fallen Scion may create a rescue or item-recovery expedition for
  a successor or reserve character.
- Even final death should contribute something bounded to House history or
  development.

## Town and campaign graph

- One persistent non-combat town contains commerce, House services, storage,
  NPC guidance, and expedition access.
- Provisional NPC roles:
  - Elder or steward: crisis, House projects, faction relationships, routes.
  - Weapons-and-tools trainer: arms, combat guidance, martial tasks.
  - Armor, ritual, or lore merchant: protection, magic goods, reagents, lore.
- Campaign progression is a graph of zones and acts.
- Most regions contain an optional side branch; some branches may branch again.
- Detours grant meaningful movement, equipment, skill, knowledge, route, or
  House rewards.
- Waypoints and limited portal travel should be convenient while fitting the
  setting and risk model.

## Instances and transitions

- The Owner Demo includes a town and multiple distinct combat zones.
- Zones persist for a defined lifetime so returning preserves state.
- Normal portal or gate use re-enters the existing instance.
- Control-clicking a portal or transition explicitly requests a fresh instance.
- Transitions are physical gate-like landmarks with hover highlights and clear
  destination labels.
- Generated zones guarantee entrance, main route, optional branch, objective,
  boss, exit, and valid connectivity.

## House development

The House is not literally a building. It controls people, relationships,
workshops, routes, stores, training, and infrastructure across settlements.

### First development fork

- **Stores and cultivation:** daily/offline coin, crops, food, and supplies.
- **Materials and workshop:** wood, ore, bars, item bases, crafting, and Brands.
- **Caravan and exchange:** trade with settlements and other Houses for gear,
  currency, reagents, and relationships.

### Real-time loop

- A bounded daily grant is claimed by the player.
- Copper, silver, and gold placed in the treasury pay ongoing upkeep.
- Farms, mines, workshops, and routes accumulate outputs while offline.
- Recommended safe behavior: empty funding or full storage pauses production
  without debt, deletion, or decay.

### Later House depth

Salvage and disenchanting, workers and wages, smelting, shipping, remote
expeditions, advanced crafting, recombination, rune work, spell-lattice support,
recovery intelligence, Legends, and currency exchange.

## Six auxiliary inventory windows

These are outer-tree milestones expected roughly around levels 20–40. Actual
access depends on node and link cost, with worthwhile intermediate passives
discouraging a direct beeline.

- Strength: **War-call**, plus tower-shield direction.
- Dexterity: **Quick Rig**, plus second weapon set and quiver directions.
- Intelligence: **Attendant** focus.
- Strength + Dexterity: **Spoils Roll**.
- Dexterity + Intelligence: **Preparation Case**.
- Intelligence + Strength: **Reliquary**.

All six require complete Verdigris mechanics, WIZARD laboratory demos, and
validated ancient-world item art.

## Owner Demo presentation requirements

- Animated actors and observable attacks.
- No circle or rectangle placeholders in normal play.
- Raster, image-derived WIZARD Framekit panes, frames, controls, and overlays.
- Faithful WIZARD Life and resource orbs.
- Main, pause, settings, character, House, inventory, and exit menus.
- Escape closes the top pane or opens pause; it never quits directly.
- Exit only through an explicit menu action or the window-close control.
- Diablo-style grid backpack, paper doll, equipment seats, tooltips, and real
  item art from WIZARD RPG Inventory.

## WIZARD modules required in active gameplay

- Geometric skill tree
- RPG Inventory, Brands, Bonds, trophies, reagents, and item history
- Cartographer map generation
- Spell lattice
- Splash and menu scene
- Chronicles House, Scion, mortality, succession, and recovery
- WIZARD resource orbs
- Image-derived Framekit

## Long-term full-game pillars

- Multiact campaign and repeatable chosen-goal endgame
- Solo, cooperative, and scalable authoritative online play
- Persistent Houses, mortal and reserve Scions, companions, and succession
- Rare meaningful items, Brands, Bonds, relic circulation, and Legends
- Deep geometric passive tree and graph-based spell laboratory
- Async House economy, trading, passive production, and currency exchange
- Factions, politics, settlements, routes, and ancient-world cultures
- Seasonal extension mechanics
- Native performance, packaging, persistence, replay, observability, and tools

## Open decisions

- Final four faction identities and respectful cultural boundaries
- Mortal versus persistent-character modes
- Rescue, missing, and final-death state transitions
- Item-recovery exclusivity and global circulation scaling
- Exact economy rates, coin exchange, sinks, storage, and inflation controls
- Portal fiction and risk model
- Companion timing and cooperative-play timing
- Final terminology for historical items

