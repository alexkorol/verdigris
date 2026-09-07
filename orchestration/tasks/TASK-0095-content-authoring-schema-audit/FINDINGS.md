# FINDINGS — TASK-0095 native content and asset-authoring schema audit

Lane: `ox-pc-bd` · Branch: `worker/verdigris/pc/ox-pc-bd` · Base: `d2423873c577d299b3b39c56024d1d840993c72b`
Scope: read-only audit of `native/content`, `native/src`, `native/include`, and the historical
`server/` reference tree. No schema or content was implemented. Machine-readable registry:
`captures/content-surfaces.json`.

---

## 1. Where content is authored today

### Zone and layout

| Surface | Authored in | Form |
| --- | --- | --- |
| Adventure zone table (6 zones: id, display name, template, layout) | `native/src/core.cpp:1427-1438` (`adventure_zones()`), mirrored row-for-row from `server/core/party.js` `ADVENTURE_ZONES` | code-bound C++ |
| Template/layout acceptance sets | `core.cpp:1440-1449` (`is_zone_template`, `is_zone_layout`) — the layout enum is a separate hardcoded string check | code-bound C++ |
| Instance geometry per layout (warren ribs, clearings thickets, gauntlet corridor), 40×40 grid, fixed stairs/spawn anchors, protected spawn clearing | `core.cpp:1455-1489`, `1655-1699` (`generate_instance`) | code-bound N2 procedural stubs |
| Town collision (open 200×200 field, login spawn 38,115) | `core.cpp:1493-1501`, `1603-1626`; spawn default `core.hpp:977` | code-bound N2 stub; real tables live in `server/maps/surface.tmx` (~202 KB Tiled map + layers) |
| World-web roads (tin/salt/chalk/copper) + node-name terroir tables | `native/src/networking.cpp:719-741`; JS original `server/core/world-web.js` `ROADS` | code-bound; node identity is hash-derived per House |
| Versioned zone graph (exits, reachability, visual roles) | `native/content/schema.json` entities.zone + `native/content/seeds/zones.json` | JSON seam, schema_version 1, **not loaded by the C++ core yet** (`native/content/README.md:4-6`) |

Consumers to keep in view: zone ids resolve through `zone_id_for_instance()`
(`networking.cpp:706-715`) for quest targets and transition payloads; display names flow from
`zone_display_name()` (`core.cpp:1628-1653`) into HUD and scene transitions.

### Actor

- **Shared stat schema** (constitution invariant): `ActorStats` at `core.hpp:106-122`; one schema
  for players and monsters. Defaults are content-adjacent but struct layout is frozen by
  snapshots and recorded command streams.
- **Monster composition is inline code**: theme→level table (`core.cpp:1712-1715`: crypt 4,
  wilds 6, marsh 8, else 2), level formula (`+2/floor`, `+1 per 7 placed`, `core.cpp:1736-1738`),
  per-theme melee/ranged/buffer pack recipes (`core.cpp:1741-1758`), trash id `<theme>-lurker`
  and name `<Zone> Lurker` (`core.cpp:1730-1733`), coins formula (`core.cpp:1780-1781`), grove
  beast tags (`core.cpp:1777-1779`). Boss slot hardcodes five display names
  (`core.cpp:1759-1769`). This mirrors `server/core/map.js` `THEME_MONSTERS` and
  `server/core/data/monsters/index.js`.
- **Intentional extension seams**: `spawn_monster()` (`core.hpp:342`), `pending_wave()` pack
  materialization (`core.hpp:344-350`), `SeasonalMechanic` hooks (`core.hpp:352-354`,
  `seasonal.cpp`).

### Skill

- Action vocabulary `ActionType {Melee,Dash,Wait,Thrust,Sweep,WarCry}` with **frozen ordinals**
  (`core.hpp:25-27`); tuning lives in named constants: D-114 world-scale table
  (`core.hpp:61-73`), skill costs/bonuses (`core.hpp:81-90`), telegraph window
  (`core.hpp:29-32`), dash ticks (`core.hpp:76`).
- These are republished read-only via `PresentationCatalog` / `presentation_catalog()`
  (`core.hpp:92-104, 333-335`) so clients never mirror values.
- No native skill registry exists yet: the session defaults `active_skill_id_ = "primary-attack"`
  (`networking.cpp:643`), combat events emit that literal (`core.cpp:1967`), and
  `player:skill:trigger` accepts any client string unchecked (`networking.cpp:2507`). The browser
  reference authors real definitions via `server/shared/skills/schema.js`
  (`createSkillDefinition`) + `server/shared/skills/index.js`.

### Item and trophy

- **Item catalogue**: `kItemCatalogue[]` ~24 rows (`core.cpp:2636-2674`) mirroring
  `server/core/data/items/*.js`; lookup `item_def()` (`core.cpp:2741-2746`). Consumers:
  `create_game_item`, footprint resolution, equip totals, networking payloads
  (`networking.cpp:1071,1122`), shop/bank screens.
- **Loot pool**: `gear_drop_pool()` — 13 vessel ids where **declaration order is load-bearing**
  because the roll selects a pool index (`core.cpp:2748-2757`; rolled at `core.cpp:3167,3198`).
- **Vesselforge pack**: materials/forms/brand-mods/name-word tables ported verbatim
  (`core.cpp:2101-2228`); the comment at `core.cpp:2049-2050` declares iteration order
  load-bearing for brand-pool parity with `server/core/items/vesselforge/verdigris-pack.js`.
  Rolls run on `Mulberry32` with exact JS semantics (`core.hpp:450-459`, `core.cpp:2036`).
- **Content-coupled mechanics**: backpack footprints are derived from item id/name *substrings*
  (`id_contains` needles at `core.cpp:2694-2737`) — adding a catalogue entry whose id lacks the
  expected substring silently changes its grid size. Equip seat expansion (`ring → ring/ring2`)
  behaves similarly (`core.hpp:673-700`).
- **Trophy**: the single warden trophy name `"Warden's ember"` is a code literal inside
  `drop_reward()` (`core.cpp:781-784`); ids come from the seeded token stream. Relic/trophy
  circulation mechanics carry stable provenance wire keys (`core.hpp:705-715`;
  `legacyRelicId` denylist exception documented in `config/legacy-denylist.json`).

### Quest

- The ordered chain of four quests (`aldwyns-charge`, `proof-of-temper`, `the-pale-crown`,
  `rot-in-the-reeds`) is a code table `kQuestChain` (`networking.cpp:691-705`), fired by trigger
  call sites (`networking.cpp:1140,1546,1722-1723,2106-2111,2197,2386-2389,2559`).
- Objectives string-match **content owned by other surfaces**: zone ids (`weir-crypt`,
  `marsh-of-reeds`) and boss display names (`The Pale Sovereign`, `The Rotfather`). Mirrors
  `server/shared/quests.js` + `server/core/services/quest-service.js`. First-goal machine state
  parallels `server/core/first-goal.js` (`networking.cpp:1378,1637-1666`).

### Presentation

- Curated simulation-side exports only: `PresentationCatalog`, telegraph contract,
  `ExpeditionPhase` telemetry (`core.hpp:92-104, 294-312, 333-335`).
- Visual-role/slot vocabulary (10 roles × 6 slots) exists as versioned enums in the content seam
  (`native/content/schema.json:16-28`) applied by both seed files — the only presentation-content
  vocabulary outside code.
- Assets live presentation-side (`native/client/assets/`, audio mixer tests as consumers),
  consistent with the constitution's headless-core rule; licensing/porting language stays an open
  owner decision.

## 2. Stable IDs (frozen contracts)

- `ActionType`/`EventType` ordinals are append-only so recorded command streams and stored event
  ordinals keep historical codes (`core.hpp:25-27, 249-252`).
- Wire envelope `{event,data}` and snapshot field keys are frozen (`AGENTS.md`; serializer
  helpers in `core.cpp:1042+`).
- Content-seam identifier grammar `^[a-z][a-z0-9]*(-[a-z0-9]+)*$` (max 64) is public validator
  contract; error codes `E_*`/`W_*` are stable while messages may be reworded
  (`native/content/README.md:60-80`).
- Runtime identity formats: item uuids `00000000-0000-4000-8000-<serial>` (`core.cpp:2678-2686`),
  monster uuids `monster-<serial>-<placed>` (`core.cpp:1731`), vessel ids `vf<base36…>`
  (`core.cpp:2289-2293`), web node ids `<road>:<tier>:<index>` parsed at `networking.cpp:2386`.
- Zone identity = template+layout resolved against `adventure_zones()`; unknown themes fall back
  to `"dungeon"` on entry (`core.cpp:1789`) and `"old-barrow"` in quest mapping
  (`networking.cpp:714`) — fallbacks are themselves content-bearing decisions.

## 3. Deterministic seed boundaries

| Boundary | Location | Drives |
| --- | --- | --- |
| Simulation ctor seed | `core.hpp:316` | Rng streams: ids, drops, combat resolution; snapshot-canonical bytes |
| Instance seed `fnv1a(theme+":"+layout, seed)`; floors append `":floor-N"` | `core.cpp:1802, 3222` | xorshift LCG monster scatter seeded from `metadata_.seed` (`core.cpp:1705-1711`) — same world seed + theme + layout + depth reproduces population exactly |
| World random constant `0x9e3779b97f4a7c15` | `core.hpp:1013` | treasure scatter; forge reseeds before each gear roll (`core.cpp:3174-3175, 3201-3202`) |
| VesselForge `Mulberry32` persistent stream | `core.hpp:450-459` | all material/form/brand/count/name rolls; `sear()` advances it |
| Session RNG unseeded unless payload carries explicit numeric seed | `networking.cpp:1064-1067, 1095-1098` | dev grants reproducible without perturbing the session stream |
| Web chart FNV-1a over `house\|road\|tier\|index` | `networking.cpp:746-762` | deterministic per-House charts with no stored layout state (only cleared wardens persist) |
| Snapshot boundary retires instance state (D-109) | `core.hpp:430-433`; exclusion comment `core.cpp:1265` | floor content never survives save/load; recovery pools do |
| Validator determinism | `validate_content.py` sorted diagnostics; negative suite asserts byte-identical output | authoring-time determinism is independent of runtime seeding |

Key structural fact: **seeded generation consumes authored tables positionally** (pool indexes,
weighted iteration order, Mulberry32 draw sequences). Any authoring pipeline must treat element
order as semantic data, not formatting.

## 4. Validation gaps

1. **No cross-language enum lock** — `schema.json` zone_template/layout enums mirror the C++
   acceptance sets by convention only (`native/content/README.md:64-66` vs `core.cpp:1427-1449`).
   Nothing fails CI when they drift.
2. **No locking test for `gear_drop_pool` order** although rolls index it positionally
   (`core.cpp:2748-2757` vs `3167,3198`). Same exposure for the three pack tables despite the
   load-bearing-order comment (`core.cpp:2049-2050`).
3. **No referential integrity across surfaces** — quest objectives match boss *display names* and
   zone *ids* owned elsewhere; renaming either silently breaks two chain quests
   (`networking.cpp:701-703` vs `core.cpp:1759-1769,1427-1438`).
4. **Seam coverage is narrow** — the versioned seam models only zone+encounter. Actors/stats,
   items/catalogue, brands, skills, quests, roads, loot pools, trophies, and seasonal definitions
   have no versioned shape.
5. **Migration tooling absent** — the versioning policy text exists (`README.md:82-87`) but no
   dual-version validator or migration harness.
6. Minor: `W_UNREACHABLE_ZONE` warns while `E_UNREACHABLE_ENCOUNTER` errors
   (`validate_content.py:638-679`) — asymmetry is undocumented; and the legacy denylist scanner
   does cover `native/content/**.json` today (`check_legacy_denylist.py` excludes only
   tests/tools), which should be preserved as content grows.

## 5. Negative control (required by spec)

**`gear_drop_pool()` element ORDER cannot be safely externalized without a locking test.**
The gear roll draws a pool *index* and reads positionally (`core.cpp:3167,3198` over
`core.cpp:2748-2757`). Moving the list to JSON preserves values but not declared-order intent:
any tooling that sorts keys, merges files, or rewrites arrays reshuffles every rolled drop with
zero type errors and zero unit failures. Before externalization this needs (a) a byte-order
locking test asserting contents *and* sequence, and (b) a parity fixture rolling drops across
rarities/depths before-and-after the data move. The same hazard applies to the three Vesselforge
pack tables.

## 6. Proposed versioned schema/tool pipeline (successor design — nothing implemented here)

Schema direction:

- Keep the single integer `schema_version`; append-only enum growth; breaking changes ship a
  dual-validator only during a migration task (existing policy, now with tooling).
- Next entities behind the existing envelope `{schema_version, kind, items}`: `actor-def`
  (stat deltas + behaviour/rarity/tags referencing the shared stat schema — no balance numbers
  until the owner pass), `monster-pack` (theme recipes replacing inline logic), `item-base`
  (catalogue rows with explicit size fields replacing substring inference), `brand-pool`
  (materials/forms/mods with an explicit `order` field so file order stops being semantic),
  `loot-pool` (ordered ids carrying dense unique `order` integers), `quest-chain`
  (objectives referencing zone/actor ids **by reference**, never display-name strings),
  `road-chart`, `skill-def` (mirroring `createSkillDefinition` shape).

Tool pipeline:

1. `validate_content.py` remains the dependency-free gate; new entity kinds reuse the same
   envelope and diagnostic discipline.
2. **Lockstep checker** (stdlib Python beside `check_legacy_denylist.py` in CI): extracts
   `adventure_zones`, `gear_drop_pool`, pack tables, and `kQuestChain` literals from the C++
   sources and asserts the committed seeds match byte-for-byte including order. This is the
   locking validator that makes future externalization safe rather than hopeful.
3. Referential-integrity pass extended beyond exits: quest refs → zone ids + actor ids;
   encounter.zone → zones (exists); item vessel_form/hints → pack forms.
4. Keep the legacy-vocabulary scan over all native JSON as content volume grows; add a
   display-name lint hook for future owner lore intake.
5. When the core first loads seeds, parsing stays OUTSIDE the headless simulation: the platform
   layer parses files and hands typed structs to the sim, preserving the no-I/O core rule and
   determinism.

Migration risks to record now:

- Positional RNG contracts (pool order, pack order, draw sequences) — every externalization
  requires before/after parity fixtures.
- Quest ↔ boss-name coupling — fix references before moving any one surface alone.
- JS/C++ double maintenance while the browser reference lives; parity gate D-116 forbids casual
  browser edits, so shared JSON is the eventual single source, adopted incrementally.
- Float weights must serialize losslessly (decimal strings or numerator/denominator pairs), or
  JSON round-trips will alter brand odds.

Owner-only vs mechanical split (stop line respected): names, lore, epithet word lists, boss and
trophy names, all balance/drop/coin numbers, and the existence set of zones/roads/quests remain
owner decisions. Everything proposed above is schema-neutral validation and tooling.

## 7. Evidence map

Full machine-readable registry with every cited consumer: `captures/content-surfaces.json`.
Acceptance transcripts: `REPORT.md`.
