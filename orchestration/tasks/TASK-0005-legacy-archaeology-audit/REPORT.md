# TASK-0005 — Legacy browser-game archaeology audit

Audit target: browser/server reference at `1a41393` (the requested
`9eadfbd` code paths are unchanged in this worktree; the later commits on this
branch are orchestration-only). This is an evidence report, not a KEEP/REMOVE
decision. The product authority used for comparison is
`docs/product/VERDIGRIS_CONSTITUTION.md`; the firewall is
`docs/rebuild/LEGACY_MATRIX.md` and `config/legacy-denylist.json`.

Line references below are line numbers from the audited checkout. Commit tags
are historical provenance, obtained with `git log --all -- <path>` (not claims
that a commit is itself a product decision).

## 1. System inventory

### Login / identity — `mixed`

The browser offers a cryptographically random, localStorage-backed guest ID and
sends it in a `player:login` envelope (`src/core/auth/guest-session.js:3-25`),
while account login is a username/password flow in `src/components/ui/Login.vue:185-205`.
The server verifies account credentials with scrypt against a SQLite
`login_accounts` table and returns `local:<uuid>` (`server/core/services/identity-registry.js:24-33,40-105`,
`server/player/authentication.js:58-70`). A browser account/guest can be
authenticated but held as `ws.pendingPlayer` until a House/Scion selection;
`player:chronicles:select` admits the selected Scion (`server/player/handlers/socket-events/index.js:281-332,345-380`).
The same area still has a development profile and Delaford-facing UI language
(`src/components/ui/Login.vue:37-49`, `src/components/ui/Login.vue:11-16`).

Provenance: `delaford-inherited` account/guest/socket login seams
(`596c6fc`, `f083177`, `fdd102`); `verdigris-era` local identity and the
Chronicles admission gate (`e5b4e38`, `51251a0`, `a246bcd`); therefore mixed.

### Movement — `mixed`

The client maps WASD/arrows and diagonal chords, repeats held movement, and
maps Space/Shift/Q/E/R/F plus number aliases to skills
(`src/core/config/controls.js:3-53`, `src/core/utilities/input-controller.js:54-200`).
The server is authoritative: it rejects out-of-bounds, blocked, or monster-
occupied destinations, rounds positions, emits movement metadata, auto-picks
up currency, and transitions portals (`server/core/entities/player/movement-handler.js:228-382`).
Pathfinding remains a queued server action with interruption and sequence
metadata (`server/core/entities/player/movement-handler.js:385-507`), while
client rendering interpolates and rejects stale server steps
(`src/core/utilities/movement-controller.js:71-99,185-240`).

Provenance: old tile/path movement and action queue (`596c6fc`, `32ed54a`),
later continuous/off-grid movement and animation (`4ec535b`, `353d76e`), so
mixed. The constitution's deliberate WASD/control reference is evidenced by
the current control table, but the browser still carries click/pathfinding
compatibility paths (`server/core/entities/player/movement-handler.js:385-507`).

### Combat — `mixed` (Verdigris-era combat depth on legacy runtime)

`server/core/combat/index.js` validates life, cooldown, mana, skill profiles,
movement/defensive/healing/area/projectile/melee behavior, applies damage and
critical/beast modifiers, awards XP and loot, and emits hit/effect events
(`server/core/combat/index.js:292-375,624-759`). Player attacks use a three-tile
melee arc or line-of-sight projectiles; auto-attack keeps a target only while
it remains alive, in-scene, and within `AUTO_ATTACK_RANGE`
(`server/core/combat/index.js:61-91,775-820`). Monster combat has archetype and
rarity-scaled damage, aggression/pursuit, windup, reach and projectile line of
sight (`server/core/entities/monster/combat-controller.js:43-92,95-178`).
The browser client also bump-attacks and predicts clear movement, with server
reset on a monster collision (`tests/unit/client-combat-movement.spec.js:30-73`).

Provenance: initial end-to-end combat/levelling (`04d38a5`), combat-depth and
HUD pass (`579c29e`), then continuous movement, boss/rarity and measured
balance (`4ec535b`, `1fe3cb9`, `9072518`, `80c7f84`); the runtime/action
dispatcher remains inherited. The measured balance contract is executable in
`tests/unit/instance-balance.spec.js:12-18,30-40`.

### Inventory / items — `mixed`

The server catalogue is assembled from legacy weapons/armor/belts/jewelry,
general items, vessel bridges, and curated Verdigris bases
(`server/core/data/items/index.js:1-13`). Item instances receive UUIDs,
affixes, bound ownership, vessel data, combat projections and grid footprints
through `server/core/items/factory.js:69-138`; inventory hydration preserves
removed IDs as inert records and refreshes vessel data
(`server/core/utilities/common/player/inventory.js:13-50`). The grid is 12×7,
bounded to eight cells per item, with orientation and equipment-slot-derived
footprints (`server/shared/inventory-footprints.js:1-18,34-90,123-157`).
Currency is a special stack/balance and overflow can reject or drop items at
the player's feet (`server/core/utilities/common/player/inventory.js:97-227`).

The post-Delaford Vesselforge engine is plain JSON, seedable, and tracks vessel
slots, Brands, Bonds, Trophies, Scars, Patience and attunement
(`server/core/items/vesselforge/engine.js:1-20,68-116,157-208`). The Verdigris
pack is explicit data (materials, forms, weights and tiers), not an opaque
database (`server/core/items/vesselforge/verdigris-pack.js:16-64,66-113,115-158`).
However, the adapter projects vessel ratings into the old attack/defense schema
(`server/core/items/vesselforge/adapter.js:94-180`), and the legacy catalogue
still contains `bronze-dagger` (`server/core/data/items/weapons.js:91-118`).

Provenance: legacy catalogue/inventory (`596c6fc`, `74bff90`, `8f38108`),
Verdigris/WIZARD vessel and affix port (`1c228ce`, `f42229b`, `882dd81`), so
mixed. The intended stable-identity evidence is also documented in
`docs/inventory-spec.md:8-40` and `docs/chronicles-persistence.md:45-59`.

### World / map — `mixed`

`WorldManager` owns persistent towns plus transient instances/zones and stores
per-scene map layers, NPCs, items, respawns, monsters, metadata and players
(`server/core/world.js:43-70,74-101,118-154`). The old default remains
`town:delaford` (`server/core/world.js:1,84-94`), while `world-layout.js`
builds named town/zone/dungeon scenes and portal metadata
(`server/core/world-layout.js:10-17,1005-1018`). `server/core/map.js` imports
the 200×200 surface JSON, uses deterministic seeded generation, separate theme
and layout recipes, safe spawn rings, room/corridor carving, stairs and
generated monster/loot metadata (`server/core/map.js:1-19,30-90,789-836,846-878`).
The web layer adds four House-scoped roads, deterministic node names and Warden
gates (`server/core/world-web.js:1-10,14-63,118-133,174-224,241-277`).

Map inputs are both generated JS data and Tiled assets: `server/maps/layers/surface.json`,
`server/maps/surface.tmx`, `server/maps/layers/*.tsx`, and the TMX format note
(`server/maps/README.md:1-16`). Public scene payloads intentionally strip
`monsterDefinitions` while retaining map/NPC/monster/drop/metadata
(`server/core/world-transitions.js:8-35`).

Provenance: static Delaford/world/map runtime (`596c6fc`, `85565c7`), seeded
dungeons and biome packs (`4597017`, `42d4619`, `9db42ef`, `d343c72`), then
House-charted web (`17ba4ba`); mixed. The world-web behavior is explicitly
documented as a replacement for the static wilderness ring in
`docs/crossroads-world-web.md:1-5,90-121`, but the current default town ID
still says Delaford.

### NPCs / monsters — `mixed`

NPCs are data-backed town entities with fixed spawn/range, action lists, random
movement and broadcast movement/animation metadata
(`server/core/npc.js:7-48,66-101`; `server/core/data/npcs.js:1-74`). Monsters
are richer actors with UUID/template identity, tags, level, scene, archetype,
rarity, health/damage scales, respawn/rewards, shared stats, movement, AI and
combat controllers (`server/core/monster.js:18-85,107-125`). Archetypes encode
STR/DEX/INT scaling and behavior windows (`server/core/monsters/archetypes.js:1-107`);
rarity encodes attribute/health/damage/speed/respawn multipliers
(`server/core/monsters/rarities.js:1-45`). The compact catalogue currently
contains Ashen Wolf, Hollow Guard and Ember Seer with level, tags, spawn,
experience and respawn data (`server/core/data/monsters/index.js:3-77`).

Provenance: town NPC class/data is inherited (`596c6fc`); shared actor stats,
archetypes, AI/ECS and biome/monster tuning are Verdigris-era (`4c18a26`,
`9fe92ed`, `6e284b4`, `9072518`); mixed. The constitution's shared actor
vocabulary is present in the monster/player stats path, but this is still
browser/server object code rather than the native headless model.

### Skills / professions — `mixed`

The active combat registry defines six data-shaped skills with animation,
quickbar, cooldown, mana cost and behavior descriptors
(`server/shared/skills/schema.js:17-58`, `server/shared/skills/index.js:4-86`).
Server execution resolves only registry profiles, applies resource gates and
cooldowns, and awards combat XP (`server/core/combat/index.js:631-684`,
`server/core/combat/experience.js:19-104`). The authored passive tree is a
271-node server-validated graph with point budgets, reachability, conduit and
class-order checks (`server/core/passives/verdigris-authority.js:23-110`),
backed by generated JSON-like source data (`src/core/passives/verdigris-authored-tree-data.js:1-13`).

The old profile still initializes `attack`, `defence`, `fishing`, and `cooking`
(`server/core/entities/player/fresh-scion-profile.js:4-9,35-40`; template
`server/core/data/helpers/player.json:8-24`). A legacy `Skill.extractResource`
path still turns a resource into inventory or a world drop
(`server/core/skills/index.js:42-87`), while the old item preset explicitly
retires ore/smithing and leaves pickaxes as legacy weapons
(`server/core/data/helpers/database.js:16-20,49-56`).

Provenance: combat skill schema/passive lattice are Verdigris/WIZARD-era
(`1c228ce`, `d4d1e53`, `a62956b`); resource skills and extraction are Delaford
(`596c6fc`, `a128c49`, `0b28ad5`); mixed. The matrix says the existing passive
lattice is reference-only, which is important because browser code currently
uses server authority for it.

### Crafting — `mixed`

There are two distinct systems. The legacy item/action lane retains old smithing
and anvil vocabulary in catalogue data (for example `server/core/data/items/weapons.js:35-61`
and `server/core/data/items/general.js:25-39`), but `presetActions` deliberately
returns no `anvil` action and labels the ore/smithing loop retired
(`server/core/data/helpers/database.js:16-20,49-56`). The active crafting seam
is Vesselforge: sear/efface/chisel spend Patience; firing can ascend, scar or
shatter; trophies and bonds evolve separately
(`server/core/items/vesselforge/engine.js:243-307,310-440`). One exposed browser
operation adds a random Brand at a 100-coin cost, only at the default town forge
(`server/core/context-menu/strategies/vesselforge-brand.js:1-27`,
`server/player/handlers/actions/index.js:984-1038`).

Provenance: inherited anvil/smithing (`8f38108`, `0b28ad5`); Vesselforge
ported/curated rules (`1c228ce`, `882dd81`); mixed. The constitution's House
crafting requirement is not represented by the forge operation itself; House
upgrade/crafting-base metadata exists in `server/core/repositories/chronicles-repository.js:14-24,208-223`.

### Persistence — `mixed`

Chronicle accounts, Houses, living/dead Scions, relics, House links and cleared
world nodes are SQLite tables (`server/core/repositories/chronicles-repository.js:68-160`).
Login profiles also live in SQLite through `IdentityRegistry`; guest/dev and
retired-token profiles use machine-local JSON snapshots, with world coordinates
normalized back to town and durable item identity preserved
(`server/core/repositories/guest-save-store.js:1-23,25-87,89-117`).
`PlayerPersistenceService` throttles saves, routes Scions to Chronicle
snapshots, local accounts to SQLite and guests to files
(`server/core/services/player-persistence.js:5-17,44-79`). Stale inventory
records are dropped or retained inert, and old passive trees are refunded/reset
by schema (`server/core/utilities/common/player/inventory.js:13-25`,
`server/core/passives/verdigris-authority.js:121-128`; regression evidence
`tests/unit/stale-player-snapshot.spec.js:21-55,57-92`).

The browser cache and one-time import path remain in the Chronicles client/store
(`src/core/chronicles/houses.js:85-161`, `docs/chronicles-persistence.md:26-43`).
Note a documentation drift: `docs/chronicles-persistence.md:7-12` describes a
JSON file as the default, while the current authoritative Chronicle repository
defaults to SQLite at `server/core/repositories/chronicles-repository.js:8-10,68-76`.

Provenance: remote/API-era player repository (`596c6fc`, `74bff90`), then local
guest saves/accounts (`e5b4e38`, `cbbe409`, `abd676b`) and authoritative
Chronicles/relics (`51251a0`, `a246bcd`, `5f256da`); mixed.

### Networking envelope — `mixed`

The wire contract is JSON `{ event, data }`; server receive validates event and
object payload, applies rate/auth/identity gates, and dispatches the envelope
to handlers (`server/Delaford.js:454-518`). The server emitter serializes the
same envelope and adds optional `meta`/timestamps (`server/socket.js:130-170,173-221`).
The client sends exactly `{ event, data }`, queues only login and passive-tree
save while disconnected, and refuses gameplay replay (`src/core/utilities/socket.js:1-28,64-105,130-157`).
Handlers intentionally receive the wrapper and read the nested payload at
`data.data` (`server/player/handlers/socket-events/index.js:281-327`,
`server/player/handler.js:9-30`; client movement consumes `message.data` and
`message.meta` at `src/core/player/events/player.js:108-121`). Socket messages
are capped and heartbeated (`server/socket.js:10-16,35-77`).

Provenance: envelope/handler dispatch is inherited (`596c6fc`, `b77bb20`),
while identity binding, payload limits, heartbeats and authorization are later
hardening (`dbfed15`, `f104de1`, `63e1730`); mixed. Any native adapter must
translate this protocol rather than import the socket server into simulation.

## 2. Provenance ledger

| System | Tag | File-level evidence | Archaeology |
|---|---|---|---|
| Login/identity | mixed | local SQLite + Chronicles gate: `server/core/services/identity-registry.js:24-105`; pending admission: `server/player/handlers/socket-events/index.js:281-332` | `596c6fc`, `e5b4e38`, `51251a0`, `a246bcd` |
| Movement | mixed | continuous server movement: `server/core/entities/player/movement-handler.js:316-382`; legacy queued path: `:385-507` | `32ed54a`, `4ec535b`, `353d76e` |
| Combat | mixed | current authoritative skills: `server/core/combat/index.js:631-759` | `04d38a5`, `579c29e`, `1fe3cb9`, `9072518` |
| Inventory/items | mixed | legacy catalogue: `server/core/data/items/index.js:1-13`; Vesselforge: `server/core/items/vesselforge/engine.js:1-20` | `596c6fc`, `1c228ce`, `f42229b`, `882dd81` |
| World/map | mixed | Delaford default: `server/core/world.js:1,84-94`; House web: `server/core/world-web.js:1-10` | `85565c7`, `4597017`, `17ba4ba` |
| NPC/monsters | mixed | NPC: `server/core/npc.js:7-101`; actor/AI: `server/core/monster.js:18-85` | `596c6fc`, `4c18a26`, `9fe92ed` |
| Skills/professions | mixed | authored registry: `server/shared/skills/index.js:4-86`; retired gathering: `server/core/data/helpers/database.js:49-56` | `1c228ce`, `d4d1e53`, `a128c49`, `0b28ad5` |
| Crafting | mixed | Vesselforge: `server/core/items/vesselforge/engine.js:251-307`; old anvil path intentionally retired: `server/core/data/helpers/database.js:16-20` | `8f38108`, `0b28ad5`, `1c228ce`, `882dd81` |
| Persistence | mixed | SQLite Chronicle tables: `server/core/repositories/chronicles-repository.js:93-160`; guest JSON: `server/core/repositories/guest-save-store.js:73-117` | `596c6fc`, `e5b4e38`, `cbbe409`, `51251a0` |
| Networking | mixed | receive: `server/Delaford.js:454-518`; emit: `server/socket.js:130-170` | `596c6fc`, `dbfed15`, `f104de1`, `63e1730` |

The strongest provenance boundary is not a directory boundary: commits such as
`1c228ce` and `17ba4ba` add Verdigris/WIZARD data and House-world behavior into
the same server runtime that still owns Delaford defaults. Treat individual
catalogue IDs, constants, and schemas as tagged evidence rather than assuming
that all of `server/` is legacy or all of `src/` is current.

## 3. Extractable data and formats

These are concrete data candidates; extraction does not imply native adoption.

| Data | Path / format | Useful fields or constants |
|---|---|---|
| Curated item bases | `server/core/data/items/verdigris.js:20-56,61-200` | JS object records: id/name/price/type/slot/size/artId/vesselForm/vesselMaterial/attack/defense. |
| Legacy catalogue | `server/core/data/items/{weapons,armor,belts,jewelry,general,vessels}.js`; index `server/core/data/items/index.js:1-13` | JS arrays; useful for provenance and rejected IDs, not starter defaults. |
| Vesselforge pack | `server/core/items/vesselforge/verdigris-pack.js:16-64,66-158,270-307` | Serializable JS pack: materials, forms, brandMods, themes, trophies, pigments, omens, settings; `derive()` is the only non-data helper. |
| Vesselforge item schema | `server/core/items/vesselforge/engine.js:157-208` | JSON item `{v,id,formId,materialId,kind,w,h,ilvl,vessel,scars,patience,brands,bonds,trophies,att,evolutions,fired,awakened}`; seedable Mulberry32 at `:13-20`. |
| Affix tables | `server/core/items/affix-data/{brands,bonds}.js`; engine `server/core/items/affix-engine.js:46-77,124-158` | JS ranges/tiers/weights and recursive stat blocks. |
| Skills/quickbar | `server/shared/skills/index.js:4-129` | Data-shaped definitions: behavior, cooldown, mana, animation, hotkey, icon, tags. |
| Passive tree | `src/core/passives/verdigris-authored-tree-data.js:1-13,58-100` plus server validator `server/core/passives/verdigris-authority.js:23-110` | Generated JSON-like graph, schemaVersion, seats, conduits, stat effects; server owns budget/reachability. |
| Actor stats | `server/shared/stats/index.js:1-18,151-240,356-475` | JS rules for attributes, health/mana, lifecycle and damage/heal transitions. |
| Monster catalogue | `server/core/data/monsters/index.js:3-77`; archetypes `server/core/monsters/archetypes.js:1-107`; rarities `server/core/monsters/rarities.js:1-45` | JS records for tags, level, archetype, rarity, spawn, behavior, rewards, respawn; shared STR/DEX/INT curves. |
| World-web graph | `server/core/world-web.js:14-63,118-224,241-277`; prose `docs/crossroads-world-web.md:90-121` | Deterministic string hash/PRNG; road pairs, node IDs, tiers, parent/child IDs, Warden names/status. |
| Generated instance recipes | `server/core/map.js:30-90,94-192,789-836`; theme monster tags `:211-267` | JS layout recipes, theme pools, seeded 200×200 background/foreground arrays and metadata. |
| Hand-authored surface | `server/maps/layers/surface.json`; Tiled source `server/maps/surface.tmx`, layers `server/maps/layers/*.tsx` | JSON/TMX/TSX tile maps; preserve layer dimensions, tile GIDs, object/portal metadata. Format context: `server/maps/README.md:1-16`. |
| Chronicle persistence | SQLite schema `server/core/repositories/chronicles-repository.js:93-160`; JSON compatibility store `server/core/services/chronicles-store.js:24-36` | Account/House/Scion/relic/world-progress rows; bounded JSON-compatible fallback and migration limits. |
| Guest/player saves | `server/core/repositories/guest-save-store.js:73-117` | Pretty JSON snapshots; durable item fields retain UUID, rolls, affixes, vessel, binding while world coordinates/locks are stripped. |
| Balance constants | `server/core/combat/index.js:20-27,303-324`; `server/shared/combat.js:1-31`; instance balance test `tests/unit/instance-balance.spec.js:12-40` | Ranges, cooldowns, animation durations, critical multiplier, respawn protection and measured fight expectations. |

## 4. Denylist gaps

The current denylist is intentionally small and only contains exact
case-folded substrings (`config/legacy-denylist.json:1-19`). More importantly,
the checker scans only native C/C++ extensions, not native JSON/JS/YAML/data
files (`native/tools/check_legacy_denylist.py:10-14,24-36`). The following
legacy identifiers are present in the browser code and would bypass the
checker if copied into native production in the shown spelling:

| Gap | Evidence in current code | Why the current checker misses it |
|---|---|---|
| Hyphenated `bronze-dagger` | `server/core/entities/player/fresh-scion-profile.js:13-19`; template `server/core/data/helpers/player.json:39-49`; item definition `server/core/data/items/weapons.js:91-118` | Denylist has `bronze dagger` and `bronze_dagger`, not `bronze-dagger`; native data files are not scanned at all. |
| Camel/Pascal starter variants (`startingCoins`, `StartingCoins`, `starting-coins`) | Legacy starter contract is represented as item ID/quantity in `server/core/entities/player/fresh-scion-profile.js:13-19` and template `:39-49` | Only `starting coins` and `starting_coins` are listed (`config/legacy-denylist.json:4-8`). |
| `bronze-pickaxe`, `pickaxe`, `bronze-bar`, `ore` | `tests/unit/fresh-scion-profile.spec.js:39-40`; size heuristics `server/shared/inventory-footprints.js:73-90,137-143`; retired-loop note `server/core/data/helpers/database.js:49-56` | None are denylist identifiers; `mining` alone is not guaranteed to catch compound IDs such as `bronze-pickaxe`. |
| `anvil`, `smith`, `hammer`, `woodcutting`, `crafting` | Legacy action/catalogue data `server/core/data/items/weapons.js:35-61`, `server/core/data/items/general.js:25-39`; UI/test skill vocabulary `tests/unit/character-sheet.spec.js:130-136`; client sheet `src/core/character-sheet.js:60-80` | Denylist has `smithing` but not these distinct identifiers. |
| Delaford-derived identifiers without the literal word (`town:old-wood`, `zone:fenmire`, `dungeon:barrow-depths`, `delafordGuest`) | `server/core/world-layout.js:10-17`; guest namespace `server/player/playtest-guest.js:1-35` | `Delaford` is caught only when the literal substring is present; the old zone names and compound IDs are absent from `config/legacy-denylist.json:3-13`. |
| Legacy schema names (`defence`, `legacyRelicId`, `legacyTile`, `LEGACY_MODE`) | `server/core/entities/player/fresh-scion-profile.js:4-9`; `server/core/services/chronicles.js:255-269`; `src/core/inventory/constants.js:3-7`; `src/core/rendering/renderer-mode.js:1-2` | The checker is identifier-substring based and has no schema/semantic rules. |
| Denied content in native non-C++ data | Checker extension allow-list `native/tools/check_legacy_denylist.py:10-14`; current item/profile data are JSON/JS (`server/core/data/helpers/player.json:39-49`, `server/core/data/items/weapons.js:91-118`) | A native `.json`, `.toml`, `.yaml`, `.txt`, or `.js` content file can carry any denied string without being visited. |

These are audit gaps, not a request to broaden the denylist in this task. The
architect should decide whether to add normalized-token rules and a content-file
scan when native data formats are finalized.

## 5. Surprises / risks against the constitution and matrix

1. **The fresh-Scion contract still violates the explicit firewall.** New
   Scions receive a `bronze-dagger` and 100 `coins`, and four starter skills
   include fishing/cooking (`server/core/entities/player/fresh-scion-profile.js:4-19,35-40`).
   The same assumptions are asserted by stale-player tests
   (`tests/unit/stale-player-snapshot.spec.js:9-19`) even though the
   constitution denies bronze-dagger/generic starting coins and
   fishing/cooking/mining/smithing (`docs/product/VERDIGRIS_CONSTITUTION.md:175-179`)
   and the matrix classifies them REMOVE (`docs/rebuild/LEGACY_MATRIX.md` rows
   Existing fishing/cooking/mining/smithing and Bronze dagger/generic starting coins).
   This is the highest-risk accidental port path.

2. **“Legacy” is an implementation seam, not a reliable age marker.** The
   Vesselforge adapter deliberately translates modern vessel calculations into
   “legacy combat ratings” (`server/core/items/vesselforge/adapter.js:94-99`),
   while the current Vesselforge pack is the constitution-aligned stable identity
   direction (`server/core/items/vesselforge/engine.js:1-10`; constitution
   `docs/product/VERDIGRIS_CONSTITUTION.md:54-60,136-144`). A mechanical rename
   or whole-directory port would either discard useful data or re-import old
   rules.

3. **The matrix says passive lattice reference-only, but browser server code
   makes it authoritative.** `server/core/passives/verdigris-authority.js:23-110`
   validates and computes authoritative attributes from the generated
   `src/core/passives/verdigris-authored-tree-data.js:1-13`. Native should extract
   the data only after an explicit specialization decision, consistent with
   `docs/rebuild/LEGACY_MATRIX.md`'s passive-lattice row.

4. **World-web prose and runtime names drift.** The world-web document says the
   Crossroads replaces the static Delaford wilderness ring
   (`docs/crossroads-world-web.md:1-5,90-121`), but runtime still initializes
   `town:delaford`/`Delaford` (`server/core/world.js:1,84-94`) and has old zone
   constants (`server/core/world-layout.js:10-17`). This is a naming/data drift
   risk for native route extraction, not evidence that all old map data should be
   ported.

5. **There are multiple persistence authorities and a stale persistence doc.**
   Identity and Chronicle repositories use SQLite (`server/core/services/identity-registry.js:5,24-33`;
   `server/core/repositories/chronicles-repository.js:5,93-160`), guests use
   JSON (`server/core/repositories/guest-save-store.js:89-117`), and a separate
   bounded JSON `chronicles-store` remains for compatibility
   (`server/core/services/chronicles-store.js:1-36`). The public persistence doc
   still names `server/data/chronicles-store.json` as default
   (`docs/chronicles-persistence.md:7-12`). Native extraction must select one
   authority and document migration rather than infer it from the browser.

6. **A server-to-client source-boundary violation is already present.** The
   server passive authority imports implementation/data from `src/` directly
   (`server/core/passives/verdigris-authority.js:1-8`). This is acceptable as
   historical browser glue but conflicts with the native separation invariant;
   native simulation must not reproduce this dependency direction.

7. **The network contract is easy to accidentally break.** Server handlers
   consume the nested `data.data` payload (`server/Delaford.js:122-163`,
   `server/player/handlers/socket-events/index.js:281-327`) while outgoing
   payloads are `{event,data}` with optional top-level `meta`
   (`server/socket.js:155-170,185-191`). A native client should adapt at the
   boundary and retain server-authoritative identity checks, not copy handler
   assumptions into simulation.

8. **Dead/retired gathering code remains reachable as vocabulary and tests.**
   The old extraction class still emits mined resources (`server/core/skills/index.js:42-87`),
   old item heuristics still recognize ore/pickaxes (`server/shared/inventory-footprints.js:73-90,137-143`),
   and tests retain fishing/cooking/mining/smithing snapshot shapes
   (`tests/unit/stale-player-snapshot.spec.js:21-41`). This can make a green
   legacy test suite reward behavior that the constitution explicitly denies.

## Read-only scope verification

Immediately before this report was created, `git status --short` produced no
output. After creating this report (before commit), the only expected change was:

```text
?? orchestration/tasks/TASK-0005-legacy-archaeology-audit/REPORT.md
```

No other path was edited. The final verification after committing is recorded
in the worker handoff: `git status --short` is clean, and
`git show --stat --oneline HEAD` contains only this report path.
