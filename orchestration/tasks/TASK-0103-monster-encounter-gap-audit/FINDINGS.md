# TASK-0103 — Monster, Pack, Rarity, and Encounter Gap Audit (FINDINGS)

- Worker lane: `ox-sw-h`
- Base SHA: `d2423873c577d299b3b39c56024d1d840993c72b`
- Audit head SHA (evidence produced on): `078640f6` (claim commit; see REPORT.md for final head)
- SPEC: `orchestration/tasks/TASK-0103-monster-encounter-gap-audit/SPEC.md` (BOUNDED-DESIGN, P1)
- Companion machine-readable evidence: `captures/encounter-matrix.json`
- Scope scanned: `native/include`, `native/src`, `native/client`, `native/tests`, `native/tools` (read-only context), `playtest/scenarios`

## Method

Full read of the two encounter models (`Simulation`, actor/unit-space;
`WorldSimulation`, tile-space/networked), the protocol session that bridges them,
the native client presentation path, and every test/scenario touching monsters.
All claims carry file:line citations against the audit head. No production code
was modified; this folder is the only changed path.

## Structural map (what exists today)

Two disjoint encounter engines coexist:

1. **Actor `Simulation`** (unit-space, D-114 world units). One monster per route
   via `spawn_enemy` (native/src/core.cpp:597-603); public deterministic spawn
   seam `spawn_monster` (native/include/verdigris/core.hpp:329,
   native/src/core.cpp:248-254). Monster stats derive from `player_stats()`
   through `enemy_stats(level)` — the frozen stat-symmetry invariant
   (native/src/core.cpp:68-99). Elites schedule Thrust/Sweep with a 3-tick
   telegraph contract `kTelegraphTicks` (native/include/verdigris/core.hpp:29-32;
   native/src/core.cpp:637-658), resolved through the shared action pipeline
   (native/src/core.cpp:611-628), cancelled on player death
   (native/src/core.cpp:769-776).
2. **Tile-space `WorldSimulation`** (what networked clients actually play).
   20-monster scatter per floor from a seeded xorshift LCG
   (native/src/core.cpp:1633-1716), theme level table
   (native/src/core.cpp:1644-1647), modulo role recipes
   (native/src/core.cpp:1673-1682), marsh rare/empowered decorations
   (native/src/core.cpp:1684-1690), boss at slot 19
   (native/src/core.cpp:1691-1705), boss-only ground-slam telegraph
   (native/src/core.cpp:1916-1943), loot via `drop_monster_loot`
   (native/src/core.cpp:3079-3111).

The bridge is `ProtocolSession`: monster wire objects
(native/src/networking.cpp:853-863), combat/telegraph envelopes
(native/src/networking.cpp:1990-2005), quest/relic hooks on death
(native/src/networking.cpp:2035-2106). The native client parses a subset
(native/client/remote_session.cpp:571-584, 651-684) and renders
(native/client/main.cpp:2074-2079, 1168-1240).

## Negative control (required by SPEC)

**Invariant without authoritative coverage:** the `"uncommon"` rarity tier has an
authored consumer branch — `drop_monster_loot` assigns it base gear chance 0.10
(native/src/core.cpp:3092-3095) — but **no producer anywhere**: the generator
only ever assigns `"rare"` (marsh first spawn, native/src/core.cpp:1684-1687),
`"elite"` (boss slot, native/src/core.cpp:1691-1695), or defaults to `"common"`
(native/include/verdigris/core.hpp:760). No test in `native/tests` and no
scenario in `playtest/scenarios` exercises an uncommon monster. A successor wave
that authors uncommon monsters would inherit an unverified drop constant backed
by dead code. Recorded in `captures/encounter-matrix.json → negative_control`.

## ENGINE gaps (content-neutral, ranked)

Ranked by risk to successor waves × blast radius. All are fixable without
roster/lore/balance authority.

1. **Rarity vocabulary mismatch corrupts client rendering.** The server emits
   `"common"` as the default rarity (native/include/verdigris/core.hpp:760) but
   the remote client classifies elite as `rarity != "normal"` 
   (native/client/remote_session.cpp:666-668), so *every* common monster routes
   into the elite/boss billboard at 1.85x scale
   (native/client/main.cpp:2076-2079). No test covers this boundary.
2. **Telegraph geometry is dropped end-to-end.** The wire carries authored
   `x/y/radius/durationMs` for `monster:telegraph`
   (native/src/networking.cpp:1991-1994), but the client reads only attacker ids
   and duration (native/client/remote_session.cpp:571-584) and renders a
   directional cone anchored to the player's inverted facing, shape inferred from
   skillId substrings (native/client/presentation_state.cpp:168-178). The boss's
   circular ground slam is drawn as the wrong shape at the wrong origin.
3. **Roles are cosmetic.** `behaviour_type` melee/ranged/buffer
   (native/include/verdigris/core.hpp:759; recipes native/src/core.cpp:1673-1682)
   has zero mechanics in tile space: ranged strike only from Chebyshev≤1
   adjacency like melee (native/src/core.cpp:1841-1845), and the buffer "aura"
   is a static JSON decoration of the separate `empowered` bool
   (native/src/networking.cpp:861), not role-driven logic. Scenarios assert the
   fiction anyway (playtest/scenarios/encounter-variety.mjs:13-32).
4. **No monster locomotion or aggro model.** Monsters never move: world sim is a
   static scatter (native/src/core.cpp:1636-1716); actor sim updates facing only,
   navigation explicitly deferred (native/src/core.cpp:661-665). Engagement is
   pure proximity (adjacency auto-strike native/src/core.cpp:1836-1859; boss
   doorstep radius 2, native/src/core.cpp:1824-1834; disengage at range>4,
   native/src/core.cpp:1865-1872). No leash/home/threat/call-for-help exists, so
   any fight resets by walking four tiles away.
5. **Boss staging has no arena semantics and no wire identity.** The warden lands
   wherever LCG slot 19 falls — possibly mid-pack, unverified reachable
   (native/src/core.cpp:1650-1716). `dev:state` omits the `boss` flag entirely
   (native/src/networking.cpp:853-863); clients guess via telegraph skillId
   substrings (native/client/remote_session.cpp:575-577). Node dead-stays-dead
   lives in in-memory `cleared_nodes_` (native/src/networking.cpp:2062-2070), not
   simulation state.
6. **Elite telegraphs exist only in the wrong engine.** Thrust/Sweep telegraphs,
   cancel-on-death, and their 30-test suite live in the actor `Simulation`
   (native/tests/core_tests.cpp:369-508), but tile-space encounters — the ones
   players experience — can never telegraph except bosses
   (native/src/core.cpp:1836-1859 has no elite branch). Two presentation
   contracts, one rendered.
7. **Loot RNG stream is decoupled from the world seed.**
   `world_random_state_` initializes to the fixed constant
   `0x9e3779b97f4a7c15` (native/include/verdigris/core.hpp:984; stream
   native/src/core.cpp:2999-3007): replayable per command script, but two worlds
   with different seeds roll identical crit/loot streams. Instance placement is
   correctly seed-derived (fnv1a, native/src/core.cpp:1734,3154; LCG
   native/src/core.cpp:1637-1643).
8. **Debug print inside the swing path.** `fprintf(stderr,"[swing] …")`
   (native/src/core.cpp:1874) fires per combat tick; harmless to state
   determinism but pollutes log-diff based build-comparison workflows this repo
   relies on (build-divergence tooling).
9. **Snapshot/client parse asymmetry.** The server pays for
   level/tags/coins/behaviour/modifiers on every monster
   (native/src/networking.cpp:853-863) while the native client consumes only
   uuid/name/x/y/hp/rarity (native/client/remote_session.cpp:656-671); kill-time
   ground drops emit no envelope, so the client fabricates placeholder sparkles
   (native/client/remote_session.cpp:640-646).
10. **No generator invariant tests.** Exactly-one-boss, spawn-clearing purity,
    boss reachability, and producer/consumer rarity set equality are asserted
    nowhere directly (see matrix `tests` row).

## CONTENT gaps (owner-dependent; STOP boundary respected)

Recorded only as unknowns/scaffolding targets — no roster, lore, names beyond
citing existing stand-ins, and no balance numbers proposed:

- Monster roster identity: all trash are `"<Zone> Lurker"` stand-ins
  (native/src/core.cpp:1664-1665); five authored warden names exist
  (native/src/core.cpp:1696-1701). Final roster/names are owner-only.
- Balance constants are parity placeholders pending owner pass: trash life 30 /
  player strike 18 / boss life 120 / boss damage 12 / telegraph radius 2 & window
  1000 ms (native/src/core.cpp:1398-1406), pack damage 4+level·2
  (native/src/core.cpp:1846), coin bounty stand-in
  (native/src/core.cpp:1706-1713), treasure-hoard stub
  (native/src/core.cpp:3114-3118). Scarcity/reward tables intentionally NOT
  ported (comments cite N5 note).
- Whether unique-tier monsters, monster equipment/loadouts, ranged reach values,
  aura magnitudes, phase scripts, or additional boss mechanics exist — all
  undefined; the engine seams they need are listed under scaffolding.

## Scaffolding definitions for successor waves

Pure seams, no content authored:

- **S1 RarityTier enum** `{Common,Uncommon,Rare,Elite}` + total string mapping;
  `drop_monster_loot` switches exhaustively (compile-time break on new tiers).
  Fixes ENGINE #1/#negative-control together with the client mapping fix.
- **S2 Wire monster v2 fields**: optional `boss`, `empowered`, `home{x,y}`,
  `aggro_radius`, `telegraph{shape,x,y,radius,until_ms}` appended to the
  dev:state monster object; native client model mirrors the full parse.
- **S3 EncounterRecipe data table**: `(theme, layout, depth) → count, roles,
  group anchors, leader` consulted by `generate_instance`; replaces modulo
  recipes; draw order frozen so recorded seeds stay valid.
- **S4 Pursuit seam**: single `pursue()` hook in `advance_combat` before strike
  loops; integer-tile movement over the existing walkable grid keeps
  determinism provable.
- **S5 Unified elite telegraphs in WorldSimulation** reusing the
  `monster:telegraph` envelope + shape enum `{Cone,Circle}`; actor-sim pipeline
  becomes an implementation detail of the same contract.
- **S6 LootRollContext free function** `{monster, goods_found, rng_stream}` so
  allocation policy changes never touch combat code; derive the world RNG from
  `seed_` behind a named constant.

## Negative-test definitions for successor waves

Each must FAIL or be trivially green today, and pin a chosen rule tomorrow:

- **N1 Producer/consumer set equality** for rarity literals between
  `generate_instance` emissions and `drop_monster_loot` branches (fails today:
  `uncommon` unreachable). [negative-control test]
- **N2 Wire round-trip vocabulary**: snapshot a `rarity="common"` monster;
  parsed client model must report `elite == false` (fails today).
- **N3 Seed-coupled loot**: two worlds differing only in seed must diverge in
  loot output within K kills (fails today; defines S6's fix).
- **N4 Boss invariants**: exactly one boss per floor across all themes/layouts/
  depths, and BFS-reachable from spawn over walkable tiles (untested today).
- **N5 Geometry fidelity**: emitted `monster:telegraph x/y/radius` must equal
  rendered op center/radius within tolerance (fails today).
- **N6 Aggro silence**: player outside every `aggro_radius` receives zero hit
  events over N ticks once S4 lands (vacuous today).
- **N7 Dodge authority**: any future telegraphed elite resolves against the
  player tile at resolve tick (pattern already correct for boss slam,
  native/src/core.cpp:1929-1937; pin before reuse).
- **N8 Role mechanics gating**: buffer death removes its aura effect within one
  tick once role semantics land (blocked on owner design; harness defined).

## Commands run (audit evidence)

See REPORT.md for the verbatim acceptance commands with exit codes. Supporting
queries used during the audit: targeted `rg` passes for
`monster|spawn|rarity|boss|elite|aggro|telegraph|pack|warden|behaviour`,
`unique` (all 19 hits are `std::make_unique`/uniqueness-of-id comments — no
unique-tier concept exists), full-file reads of both encounter engines, wire
serialization, client parse/render paths, and the three native test binaries'
encounter suites plus browser scenarios `encounter-variety`, `boss-mechanic`,
`mortality`, `loot`, `gear-outcomes`.
