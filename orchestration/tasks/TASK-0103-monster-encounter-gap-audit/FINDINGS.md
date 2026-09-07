# TASK-0103 — Monster, pack, rarity, and encounter gap audit (FINDINGS)

- Lane: `ox-pc-bd` · Model: `openrouter/stealth/ox-alpha`
- Base commit: `d2423873c577d299b3b39c56024d1d840993c72b` · Audited head: `6d4effdb` (claim commit; audited paths unchanged from base)
- Machine-readable companion: [`captures/encounter-matrix.json`](captures/encounter-matrix.json)
  (`$contract: verdigris.audit.encounter-matrix`, version 1)
- Authority: `docs/product/VERDIGRIS_CONSTITUTION.md:87-92` freezes the shared
  actor/stat schema ("Players, monsters … share one microscopic actor/stat
  schema"; elite difficulty comes from "level, build, equipment, actions, and
  support rather than arbitrary billion-point Life"), and
  `VERDIGRIS_CONSTITUTION.md:113-116` names the missing world features this
  audit maps: "pack spawning, rarity, uniques, scarce equipment drops".
  This audit changes no code; every claim cites the tree at the audited head.
- Frozen invariants honored: monster/player stat symmetry (`enemy_stats()`
  derives from `player_stats()`, `native/src/core.cpp:80-99`) and deterministic
  seeds are treated as frozen. Scarcity/reward unknowns are preserved as
  owner-input gaps, never resolved here.

## Method

1. Ran the spec gate verbatim: `rg -n "monster|pack|spawn|rarity|unique|warden|boss|aggro|telegraph|elite|role"
   native/include native/src native/client native/tests playtest/scenarios`
   (exit 0, 1322 lines; verbatim transcript in REPORT.md).
2. Read end to end: `native/include/verdigris/core.hpp` (1017 lines), the
   encounter regions of `native/src/core.cpp` (make/spawn/pack/AI/loot/world
   generation), `native/src/networking.cpp` snapshot + event emission,
   `native/client/presentation_state.{hpp,cpp}`, telegraph/monster rendering in
   `native/client/main.cpp`, `native/tests/{core,networking,session}_tests.cpp`,
   and the encounter scenarios under `playtest/scenarios/`.
3. Cross-checked every engine claim against a test or recorded a gap. Nothing
   was patched; all paths outside this task folder were treated read-only.

## Orientation: two parallel encounter engines

The native workspace carries **two** monster models that do not share code:

| | D-114 world-unit `Simulation` | N-series tile-space `WorldSimulation` |
| --- | --- | --- |
| Type | `Actor` kind `Monster` (`core.hpp:23`, `:140-161`) | `WorldMonster` (`core.hpp:778-799`) |
| Packs | 3-warden anchored pack + pending-wave roster (`core.cpp:607-631`) | 20 scattered singles, one boss (`core.cpp:1466`, `:1701-1784`) |
| Telegraphs | tick-based, `kTelegraphTicks=3` (`core.hpp:29-32`) | wall-clock ms window (`core.cpp:1473-1474`) |
| Roles | none (elite boolean only, `core.hpp:148`) | `behaviour_type` melee/ranged/buffer (`core.cpp:1741-1751`) |
| Rarity | absent | string field (`core.hpp:789`) gating loot (`core.cpp:3160-3163`) |
| Boss | generic elite skill set | named bosses + ground-slam contract (`core.cpp:1759-1773`, `:1984-2011`) |

Every gap below is stated against this split. The constitution's single-actor-
schema invariant (`VERDIGRIS_CONSTITUTION.md:87-92`) is satisfied *within* each
engine but the feature sets are disjoint across them.

## Surface map

### 1. Spawning

- D-114: `spawn_enemy()` erases all monsters then spawns the entry warden at
  `kEnemySpawnX`; route `route:tin:2:0` gets a single level-2 elite, everything
  else a 3-member pack (`native/src/core.cpp:607-631`). The public deterministic
  content seam is `spawn_monster(position, level, elite)`
  (`core.hpp:340-342`, `core.cpp:254-258`). Spawn distance derives from the
  frozen world-scale table (`kEnemySpawnDistance = kMeleeRange*5`,
  `core.hpp:69`).
- Tile world: `generate_instance()` scatters exactly `kInstanceMonsterCount=20`
  monsters using a seeded xorshift over candidate walkable tiles, rejecting
  spawn-clearing/stair-adjacent tiles, capped at 4000 attempts
  (`core.cpp:1701-1729`). Instance seed is `fnv1a(theme+":"+layout, seed_)`
  (`core.cpp:1802`); spawn points are exported in metadata
  (`core.cpp:1699`, wire at `networking.cpp:939`).
- Suppression hooks exist for world-web node instances: cleared nodes spawn no
  monsters at all (`set_spawn_suppressed`, `core.hpp:877-879`; kill-all +
  warden-name override wiring at `networking.cpp:1534-1538`, `:2539-2540`).

### 2. Pack composition

- D-114 pack recipe is hardcoded: entry warden at the spawn anchor, one elite
  one melee range deeper on the approach line, one normal flanking off-axis;
  offsets reuse `kMeleeRange` so no new balance numbers are introduced
  (`core.cpp:616-630`). Unmaterialized pack mates wait in `pending_wave_`
  (`core.hpp:344-350`, `:407-412`) and **materialize together** one telegraph
  window after the entry warden falls (`materialize_wave`, `core.cpp:633-647`;
  re-arm on any in-instance kill, `core.cpp:835-842`). Extraction gating waits
  for both living wardens and owed roster entries (`core.cpp:843-855`).
- Tile world has no pack cohesion: 20 independent placements whose only group
  structure is the role rotation index `placed % 6` (`core.cpp:1744-1751`) and
  the boss occupying the last slot (`core.cpp:1759`).
- Gap: "pack" has no authoritative cross-engine definition — 3 anchored
  co-materializing wardens in one engine vs positional scatter in the other.

### 3. Roles

- Tile world roles are authored recipes mirroring `server/core/map.js`: crypt is
  melee-heavy (`role_index==4` ranged, `==5` buffer), marsh alternates
  ranged/melee with a buffer, dungeon/grove/wilds alternate melee/ranged with a
  buffer (`core.cpp:1741-1751`). Role reaches the wire as
  `behaviour.type` (`networking.cpp:930`).
- Engine gap: **the buffer/support role is a label without simulation**. No
  native code applies an aura, buff, or heal based on `behaviour_type=="buffer"`
  (searched: `buffer|aura` in `native/src/core.cpp`, `native/src/networking.cpp`
  — the only "aura" is a static wire decoration `aura:damage` attached to
  already-empowered monsters, `networking.cpp:933`). Empowerment is rolled at
  generation time only (`core.cpp:1752-1757`), never applied dynamically by a
  nearby buffer. The browser scenario asserting live aura pressure
  (`playtest/scenarios/encounter-variety.mjs:22-36`) is satisfied by the browser
  server, not by native code.
- D-114 roles reduce to the elite boolean; there is no ranged/caster/support
  action vocabulary in the D-114 enemy AI (elite Thrust/Sweep only,
  `core.cpp:681-703`).

### 4. Aggro

- D-114: every living monster unconditionally pursues the current Scion —
  facing quantizes toward the player (`core.cpp:674-677`) and attacks when in
  melee range (`core.cpp:705-718`). There is no threat table, no taunt, no
  leash/reset, and no multi-target aggro; the player is always the sole target
  (`enemy_turn`, `core.cpp:649-720`).
- Tile world: engagement is proximity-only. Bosses pre-engage within 2 tiles
  (`core.cpp:1892-1899`); pack members strike on their own 1200 ms cooldown
  within 1 tile regardless of player action (`core.cpp:1904-1927`); walking out
  to 4 tiles clears the active target (leash, `core.cpp:1933-1940`). Target
  selection prefers boss → empowered → nearest-with-aim-tiebreak
  (`start_player_attack`, `core.cpp:1833-1877`).
- A respawn ward prevents monsters damaging freshly respawned Scions
  (`networking.cpp:2096-2099`).
- Gap: no aggro surface a seasonal mechanic or boss script could hook (no
  events emitted for engage/disengage/leash).

### 5. Rarity

- Field: `WorldMonster::rarity`, default `"common"` (`core.hpp:789`).
- Producers: `"rare"` for marsh `placed==0` with `{"empowered"}` modifier
  (`core.cpp:1752-1755`); `"elite"` for the boss (`core.cpp:1763`); everything
  else stays `"common"`.
- Consumers: loot gate `0.05 / 0.1 / 0.2 / 0.5` for common/uncommon/rare/elite
  (`drop_monster_loot`, `core.cpp:3160-3163`); elite coin bounty ×3
  (`core.cpp:1781`); quest triggers key on `rarity=="elite"`
  (`networking.cpp:2111`, `:2155-2159`); browser scenarios filter
  `rarity === 'elite' | 'rare' | 'common'` (`playtest/scenarios/boss-mechanic.mjs:14`,
  `encounter-variety.mjs:10,19,24`).
- **Negative control (spec-required)**: the invariant *"every rarity value that
  can appear on a monster has an authoritative, tested drop-chance meaning"*
  has no authoritative coverage:
  1. The consumer table recognizes `"uncommon"` (`core.cpp:3161`) but **no
     producer ever emits it** — the tier is orphaned between the generator set
     {common, rare, elite} and the loot-gate set {common, uncommon, rare, elite}.
  2. `rarity` is an open `std::string` with no enum, no validation, and no
     fallback warning; an unrecognized value silently degrades to the 0.05
     base chance.
  3. No unit test exercises `drop_monster_loot`'s tier→chance table; the only
     loot tests cover a single kill path (`networking_tests.cpp:244-332`), and
     the rarity-keyed coin multiplier (`core.cpp:1781`) is likewise untested.
  Machine-readable form: `captures/encounter-matrix.json → negative_control`.

### 6. Equipment

- Shared pipeline supports attacker equipment structurally
  (`Actor::equipped_item_id`, `core.hpp:147`; use-recording
  `core.cpp:420-428`), but the damage resolver applies item bonuses **only for
  `ActorKind::Player`** (`core.cpp:410`). Monsters are spawned unequipped
  (`core.cpp:250-251`) and no spawner equips them.
- Tile-world monsters carry no equipment concept at all; their power is
  level-scaled constants (`life = kN3TrashLife + (level-2)*5`, `core.cpp:1739`).
- Gap against the constitution: "Human enemies are differently built/equipped
  actors" (`VERDIGRIS_CONSTITUTION.md:90-91`) — the schema admits it, the
  resolver forbids it. Elite difficulty therefore cannot come from equipment
  today.

### 7. Unique/boss seams

- Named bosses per theme: The Elder Oak (grove), The Pale Sovereign (crypt),
  Alpha of the Wilds (wilds), The Rotfather (marsh), Warden of the Deep
  (dungeon/Old Barrow default) (`core.cpp:1759-1773`); world-web node instances
  substitute the node's warden name (`set_boss_name_override`, `core.hpp:876-878`;
  `networking.cpp:1534`).
- Authored boss contract: announce-once ground slam — telegraph event with
  radius 2 / 1000 ms window, resolves strictly at its authored window, dodged
  by leaving the marked tiles (`core.cpp:1984-2011`, constants `:1464-1474`);
  delivered as `monster:telegraph` envelope (`networking.cpp:2063-2066`).
- Boss targeting priority and adjacency pre-engage: `core.cpp:1841-1848`,
  `:1892-1899`.
- Legends records elite kills (`elite_kill` when `elite` or level ≥ player+2,
  `core.cpp:823-832`) and elite deaths feed relic circulation
  (`networking.cpp:2155-2159`).
- "Unique" as an item/affix class does not exist anywhere in native (searched:
  `unique|champion|affix` — zero production hits). The constitution's
  "uniques" requirement currently maps onto `rarity:"elite"` bosses only.
- Proof-of-Temper guarantee: the session can force the first elite gear drop
  while the slay-elite objective is current (`set_guaranteed_elite_gear`,
  `core.hpp:873-875`; wired per tick at `networking.cpp:2082`; consumed at
  `core.cpp:3165`).

### 8. Telegraphs

- D-114: `kTelegraphTicks = 3` is part of the simulation contract so "every
  presentation can render the same warning window" (`core.hpp:29-32`); elites
  schedule Thrust (forward cone, mid band) or Sweep (close range, resource
  gated) with an `AttackTelegraphed` event carrying the skill name and windup
  (`core.cpp:681-703`); windups are cancelled on either side's death
  (`core.cpp:655-663`, `:813-821`, `:476` test). Resolution goes through the
  same `resolve_actor_action` pipeline as player skills.
- Tile world: the boss slam uses wall-clock `telegraph_until_ms` +
  `next_boss_telegraph_ms_` (`core.hpp:793`, `core.cpp:1984-2011`).
- Presentation renders both families: thrust cone and sweep arc draws with HUD
  avoidance (`native/client/main.cpp:1503-1591`), telegraph lifecycle in
  presentation state (`presentation_state.hpp:115`,
  `presentation_state.cpp:195-240`), and the render list records `Op::Telegraph`
  so scenarios catch suppressed draws (`render_list.hpp:22`).
- Gap: two incompatible timing models (ticks vs wall-clock ms) for the same
  gameplay concept; the wall-clock variant cannot be tick-replayed.

### 9. Rewards

- D-114 kill rewards: fixed-name generated axe (`attack_bonus = 4 + rng(0,3)`)
  plus trophy, drawn from the serialized core RNG (`drop_reward`,
  `core.cpp:741-785`); relic/trophy resurfacing at 1-in-4 odds from the same
  stream (`core.cpp:755-779`, constant `core.cpp:29`).
- Tile world: coins always (Wealthy-boosted), then the rarity-gated gear roll
  with pool-uniform selection, ilvl `min(80, level*2)`, and a forge reseed
  draw (`drop_monster_loot`, `core.cpp:3147-3180`); guaranteed per-floor
  treasure hoard (`scatter_floor_treasure`, `core.cpp:3182-3211`); coin bounty
  formula `10 + level*5` explicitly marked "an authored stand-in until the JS
  reward tables are ported" (`core.cpp:1774-1780`).
- Determinism hole (inherited from TASK-0100 §6): all tile-world reward rolls
  consume `world_random_state_` (`core.hpp:1013`, `core.cpp:3067-3080`), which
  is **not captured by any snapshot** — restore() cannot reproduce a loot
  sequence across a save boundary. Crit rolls share the stream
  (`core.cpp:1958`).

### 10. Deterministic generation

- Frozen inputs honored: D-114 spawns consume only `Simulation::Rng`
  (serialized in snapshots, `core.cpp` snapshot block; proven across
  save/load in `core_tests.cpp` replay tests `:271-289`, `:514-535`); tile
  layouts derive from `fnv1a(theme:layout, seed)` (`core.cpp:1802`).
- Guest identity seeding makes shared party worlds reproducible
  (`networking.cpp` FNV identity → session seed path; build-divergence comment
  `networking.hpp:243`).
- Gaps: the uncaptured `world_random_state_` stream (above); wall-clock
  deadlines (`next_attack_ms`, `telegraph_until_ms`, respawn timers) embedded
  in encounter state; monster uuids embed a per-session serial
  (`monster-<serial>-<placed>`, `core.cpp:1731`) so they are stable per floor
  lifetime but not across regeneration.

### 11. Network snapshots

- Live monster state reaches clients only through the periodic state snapshot:
  alive monsters emit uuid/id/name/x/y/level/**rarity**/tags/coins/
  behaviour.type/hp/modifiers/effects (`networking.cpp:925-935`); dead monsters
  vanish silently (no corpse/death-persistence field beyond the `death`
  combat event).
- Combat facts flow as discrete envelopes: `combat:event` hits/deaths and
  `monster:telegraph` warnings (`networking.cpp:2063-2066`).
- Gap: the snapshot omits facing and any pack/roster membership; see §13.

### 12. Presentation

- Local D-114 path: monsters map to `WorldActor{elite}` (`presentation_state.cpp:62-70`),
  expedition objective derives from living wardens (`presentation_state.cpp:118-126`),
  telegraphs rendered as cone/arc with visibility fade and HUD avoidance
  (`main.cpp:1503-1591`), elites get distinct color/scale/sprite
  (`main.cpp:3416-3439`), elite windup cancellation is mirrored client-side on
  Scion death (`main.cpp:1996`).
- Remote path: snapshot → `WorldView` mapping keeps only id/position/life/
  alive/elite — **rarity, behaviour, modifiers, and facing are dropped at the
  presentation boundary** (`presentation_state.cpp:138-154`; facing explicitly
  noted absent from the wire, `:145`), so remote clients cannot render rarity
  styling or modifier labels even though the wire carries them.
- New-monster detection feeds presentation FX (`detect_monster_spawns`,
  `presentation_state.hpp:123-134`).

### 13. Tests

Native (authoritative, deterministic):

| Test | Citation | Encounter invariant asserted |
| --- | --- | --- |
| `test_movement_step_derivation_and_actor_symmetry` | `core_tests.cpp:243` | Player/monster stat symmetry |
| `defeat_enemy` helper | `core_tests.cpp:23-65` | Pack-aware kill loop: waits through convergence window, fresh-life duels, roster empty at end |
| `test_elite_thrust_telegraph_timing` | `core_tests.cpp:395-423` | Telegraph precedes damage by exactly `kTelegraphTicks`; pending state lifecycle |
| `test_elite_skill_cone_gating` | `core_tests.cpp:425-435` | Close-range Sweep selection over Thrust |
| `test_elite_skill_fizzles_when_resolution_gates_fail` | `core_tests.cpp:437-452` | Resource re-check at resolution; clean fizzle |
| `test_elite_sweep_uses_shared_pipeline` | `core_tests.cpp:454-473` | Shared damage/resource/cooldown math for monsters |
| `test_elite_telegraph_cancels_on_death` | `core_tests.cpp:476+` | Windup cancellation on death |
| `test_elite_skill_replay_is_deterministic` | `core_tests.cpp:514+` | Command-stream replay equality |
| `test_non_elite_melee_cadence_is_unchanged` | `core_tests.cpp:536+` | Plain-melee cadence frozen |
| `test_n3_combat_rules_and_wire_events` | `networking_tests.cpp:178+` | Tile-world hit pipeline + wire events |
| `test_gate_a_ground_login_and_kill_loot` | `networking_tests.cpp:244+` | Kill → loot over the real protocol |
| journey/render-list checks | `session_tests.cpp:370`, `:537` | `monster:telegraph` reaches client; `Op::Monster` recorded from remote model |

Browser-reference scenarios (protocol-compatible, gated by `npm run playtest`,
not part of native CI): `boss-mechanic.mjs:1-58` (named boss, ≥2-tile /
≥800 ms readable warning, dodge-by-position, committed impact),
`encounter-variety.mjs:1-46` (biome role mixes, buffer presence, rare modifier,
live aura pressure, 15 s TTK bound), plus `respawn.mjs`, `session-arc.mjs`,
`quest.mjs` covering mortality/objective arcs.

Untested engine surfaces: buffer behavior (nothing to test — see §3),
`drop_monster_loot` tier table, elite coin ×3, boss slam cadence limits
(repeat eligibility `core.cpp:2010`), pack-convergence timing under player
death, and any rarity value outside {common, rare, elite}.

## Ranked engine gaps (content-neutral; candidate packets)

1. **E1 — Unified encounter host.** Two divergent monster models split the
   required feature set (§Orientation); TASK-0110 needs an explicit decision
   (bridge layer vs one canonical model) before any wave builds on both.
   Citations: §Orientation table.
2. **E2 — Role execution layer.** `behaviour_type` is data without behavior;
   buffer/support has zero simulation. Smallest fix: a role→behavior binding
   resolved per tick in the owning engine, with a contract test per role value.
   Citations: §3.
3. **E3 — Monster locomotion.** D-114 monsters never move (only facing turns,
   `core.cpp:674-677`; position writes are player-only `:318-319` and the
   shared dash `:341-342`; the code itself defers locomotion, `:708-709`).
   Tile-world monsters are fully static. Packs can never converge spatially
   today — the pending wave teleports onto anchors instead.
4. **E4 — Monster equipment.** Damage resolver excludes non-player attackers
   from item bonuses (`core.cpp:410`); blocks the constitution's
   equipment-driven elite difficulty (`VERDIGRIS_CONSTITUTION.md:90-91`).
5. **E5 — Aggro/threat surface.** No threat, taunt, leash-event, or engage
   events; aggro rules are private loops inside two functions (§4), so
   seasonal/boss scripting cannot observe or alter targeting.
6. **E6 — Rarity authority.** Open-string vocabulary, orphaned `uncommon`
   tier, untested tier→loot mapping (negative control, §5).
7. **E7 — Encounter determinism holes.** Uncaptured `world_random_state_`
   loot/crit stream + wall-clock deadlines in encounter state (§9, §10);
   extends TASK-0100's replay gap into the reward domain.
8. **E8 — Snapshot fidelity at the presentation boundary.** Client discards
   rarity/behaviour/modifiers/facing that the wire already carries (§12),
   blocking readable packs/elites remotely.
9. **E9 — Hygiene.** Stray `fprintf(stderr,"[swing]…")` debug print in the
   production swing path (`core.cpp:1942`).

## Ranked owner-content gaps (owner-only input; intentionally unresolved)

1. **O1 — Roster & names.** `<Zone> Lurker` naming template and the five named
   bosses are placeholders (`core.cpp:1733`, `:1765-1769`). Owner owns final
   names/lore.
2. **O2 — Balance tables.** Theme levels {dungeon 2, crypt 4, wilds 6, marsh 8}
   (`core.cpp:1712-1715`); trash/boss life 30/120, damages 5/+2/12, intervals
   350/1200/1500 ms (`core.cpp:1466-1474`, `:1913`, `:1973`, `:2019`); coin
   formula stand-in (`:1776-1780`). Owner-owned per spec.
3. **O3 — Scarcity & drop pools.** Gear chance tiers 0.05–0.5
   (`core.cpp:3160-3163`), `gear_drop_pool()` contents (`core.hpp:591-592`),
   ilvl curve `min(80, level*2)` (`core.cpp:3171`), Proof-of-Temper policy
   (`core.hpp:873-875`). Preserved as unknowns; not chosen here.
4. **O4 — Boss mechanics.** Exactly one authored mechanic exists (ground slam,
   §7); a unique/boss ability set is owner scope.
5. **O5 — Pack recipes.** Per-biome role rotations mirror the JS reference
   (`core.cpp:1741-1751`); final compositions are owner content.
6. **O6 — Reward identity.** Placeholder item/trophy names ("Ember-edged axe",
   "Warden's ember", `core.cpp:744`, `:781`).
7. **O7 — Feel targets.** Contact-readability window 0.5–0.8 s
   (`core.hpp:58-60`) and the browser-side 15 s TTK bound
   (`encounter-variety.mjs:37-45`) await owner ratification for native.

## Negative control (spec-required, restated)

**Invariant:** every monster rarity value that can be observed on the wire has
an authoritative, tested meaning in the reward system.
**Status: uncovered.** Producer set {common, rare, elite} ≠ consumer set
{common, uncommon, rare, elite}; `uncommon` (`core.cpp:3161`) is unreachable
from any generator; the vocabulary is unvalidated open strings; and
`drop_monster_loot`'s tier table has no direct test. Secondary pack-form: no
cross-engine definition of minimum viable pack composition/spacing exists
(§2), likewise uncovered.

## Successor scaffolding for TASK-0110 (contracts defined, nothing implemented)

- **S1 PackContract:** pack = ordered roster of anchored members with
  co-materialization semantics; property test: for any roster, members
  materialize together at `kill_tick + kTelegraphTicks` on fixed anchors, and
  extraction waits for roster ∪ living (exists partially via
  `core_tests.cpp:23-65`; generalize beyond the hardcoded trio).
- **S2 RarityAuthority:** enum-backed vocabulary + table-driven tier→drop-chance
  test; negative tests: emitting `uncommon` must select the 0.1 gate; an
  unknown rarity string must fail fast (or be rejected at generation), never
  silently fall back.
- **S3 RoleBinding:** every emitted `behaviour_type` must resolve to a
  registered behavior; negative test: `buffer` currently fails this contract —
  the first wave must either implement it or remove the label.
- **S4 BossContract:** generalize boss-mechanic assertions (announce-once,
  resolve-at-window, dodge-by-authoritative-tile, repeat eligibility) into the
  native scenario harness (`native/client/main.cpp` scenario runner pattern).
- **S5 EncounterSnapshotContract:** wire must carry what presentation needs
  (facing, rarity, modifiers, pack/roster id); negative test: current snapshot
  fails (§11–§12).
- Negative-test harness home: extend `native/tests/core_tests.cpp` +
  `networking_tests.cpp`; client-visible contracts via the scenario runner
  (`native/README.md` "Scenario harness (D-119)"). No monsters were authored
  in producing this audit; all scaffolding above is test-shaped, not
  content-shaped.

## Stop point

Per spec, this audit stops before any roster/lore/balance choice (O1–O7) and
before implementing E1–E9. Engine-level follow-up packets should be cut from
the ranked engine-gap list; S1–S5 are their acceptance seeds.
