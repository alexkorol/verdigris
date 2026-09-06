# VG-SAVE-001 — Inventory of the real durable profile

| | |
|---|---|
| Base SHA | `e7b65360ad189e392718feb37fb828533bbb08b5` (`feat(native): make Crossroads portals direct and clickable`) |
| Date | 2026-09-06 |
| Author | kimi-work |
| Status | **DRAFT-for-review** — Tier C classification document. It supports an architect/owner ruling; it is not itself a ruling. |
| Branch | `kimiwork/VG-SAVE-001-profile-inventory` |
| Scope | READ-ONLY. No code, test, or CMake changes. Only this folder is new. |

## 0. Relationship to TASK-0097 (superseded audit)

TASK-0097 (`orchestration/tasks/TASK-0097-persistence-durability-audit/FINDINGS.md`,
verdict ACCEPTED in its `REVIEW.md`) audited an earlier head (claim
`c289156a`, program tip `0bee7f1e`). Its two headline P0 risks are **partially
superseded** at this base by commit `4fe7bdb8` ("persist accounts across server
restarts"):

- TASK-0097 R1 "No production save trigger" (FINDINGS §7): **superseded.** A
  production save trigger now exists — `ProtocolSession::checkpoint()` is called
  after every handled envelope and every tick (`native/src/networking.cpp:831`,
  `:848`) and writes through the atomic adapter (`:818`).
- TASK-0097 R2 "Session profile has no serialization seam" (FINDINGS §7):
  **superseded.** The seam now exists: `archive_account` / `archive_scion`
  (`networking.cpp:722-753`) over the versioned, checksummed
  `native/persistence/account_archive.hpp` format.
- Still-valid TASK-0097 findings absorbed here: the adapter fsync/power-loss
  gap R3 (`native/persistence/adapter.hpp:23-57`, unchanged), the fixed `.tmp`
  suffix R4 (`adapter.hpp:25`, now mitigated at process level by the
  save-directory lock, `native/src/server_main.cpp:21-42,66-75`), and the
  inconsistent version-validation policies R5 (§4 below). This document does
  not redo the audit's fault matrix; it classifies the *live profile fields*.

**Evidence discipline:** every classification cites `file:line` from this
checkout at `e7b65360`. Concepts that could not be found are marked **ABSENT**;
they are not inferred from the constitution or the execution pack.

## 1. The storage surfaces that exist today

There are exactly **three** byte-level durable surfaces, and they are all
funneled into one file per account identity:

1. **Account archive** (`<hex-fnv-of-identity>.vgs`): binary, magic
   `"Verdigris account"` + `version = 1` + FNV-1a checksum over the payload
   (`account_archive.hpp:148-164`). Written by `ProtocolSession::checkpoint()`
   (`networking.cpp:812-820`, dirty-checked against `saved_bytes_` at `:817`)
   via `persistence::write_atomic` (`adapter.hpp:23-57`); read in
   `attach_save` (`networking.cpp:784-810`) via `persistence::read`
   (`adapter.hpp:59-73`). Attached at login when the server has a save
   directory (`networking.cpp:5165-5173`; directory resolution and the
   single-writer lock at `server_main.cpp:56-75`).
2. **Core Simulation snapshot** (`verdigris::snapshot` / `restore`,
   `native/src/core.cpp:1229-1297` / `:1300-1378`): the small deterministic
   House/Scion/RNG format from TASK-0097, now **embedded as a byte blob inside
   the account archive** (`networking.cpp:742-750`).
3. **Chronicle JSON document** (`chronicle_`, `networking.hpp:282`): an
   in-memory JSON tree (houses, living roster, crypt, per-house progression
   sub-objects) that is itself archived as a JSON string inside surface 1
   (`networking.cpp:735`; `account_archive.hpp:57-60`), and is additionally
   the restore source for the `restore_*` family at Scion set-out/select time
   (`networking.cpp:4742-4746`, `:4897-4907`).

Explicitly **not** storage: `ProtocolSession::snapshot()` /
`state_payload()` (`networking.cpp:1547-1759`, `:1760`) is a wire projection
rebuilt per request (the `dev:state` 4 Hz snapshot). Its `state.xp` block
(`networking.cpp:1553-1563`) is another agent's reservation: treated read-only
here, and this document proposes no change to it.

The **negative control** from the pack holds at this head: saving only the
small local `Simulation` (surface 2) cannot pass, because inventory, wear,
bank, passives, XP, lifecycle, chronicle, and recovery state all live on
`ProtocolSession` (`networking.hpp:205-316`), outside `Simulation`'s durable
field set (`core.hpp:404-430`). The real profile surface is the account
archive; this inventory covers it field-by-field below.

## 2. Field-by-field inventory

Behavior legend: **disk** = inside the `.vgs` account archive; **rebuilt** =
reconstructed/reset at session start, reconnect, or per wire request;
**lost** = dies with the process and is not rebuilt. "Dual" means the same
concept is written both as a flat archived field and inside the archived
Chronicle JSON (see gap G1).

### 2.1 Account / House identity and Chronicle

| # | Field / concept | Lives today | Behavior today | Proposal | Justification |
|---|---|---|---|---|---|
| 1 | Account identity (`identity_`) | `networking.hpp:205`; archived as `owner` with mismatch hard-throw `networking.cpp:732-734`; filename derived from FNV of identity `networking.cpp:5165-5167` | disk | durable | It is the save key; tamper-evident by identity check. |
| 2 | `username_` (playtest display name) | `networking.hpp:207`; `networking.cpp:735` | disk | durable | Display continuity across restarts; harmless. |
| 3 | Chronicle document (`chronicle_`): houses, living roster incl. per-Scion `mortal` flag, crypt, relic ledger, `campaignQuests`, `firstInvestment`, `treasury`, `renown`, `clearedRoadNodes`, `endgameMasteries`, `vesselforgeTrophies` | `networking.hpp:282`; built `networking.cpp:4280-4352`; archived `networking.cpp:735`; version field `=3` written `networking.cpp:4284` and **never validated on load** (grep: no read-side check) | disk | durable | This *is* "the House remembers": roster, crypt, and House progress survive restart (proven by restart test, §5). Version policy gap noted as G2. |
| 4 | `chronicles_revision_` | `networking.hpp:283`; `networking.cpp:735` | disk | durable | Monotonic client-sync counter; must not regress across restart. |
| 5 | Active House/Scion pointers (`active_house_id_/name_`, `active_scion_id_/name_`) | `networking.hpp:287-290`; `networking.cpp:735-736` | disk | durable | Login resumes the same House/Scion context. |
| 6 | Hardcore / mortal oath (creation checkbox) | Chronicle roster flag written at scion creation `networking.cpp:4347`; set at set-out `:4753-4760` and select `:4878-4903`; mirrored into `mortal_oath_`/`lifecycle_mode_` (archived per-Scion, `:724-725`) and re-derived on reconnect `:882-887` | disk | durable | Constitution: "Scion is mortal" is the admission contract; restart test asserts creation-time durability pre-admission (`test-account-restart.mjs:91-94`). |
| 7 | House founding (`chronicles:house:found`) reset block | `networking.cpp:4687-4704` | n/a (mutation path) | — | Documented here because it is the point that zeroes House-scoped caches (`:4696-4701`). |

### 2.2 House economy, progression, and world knowledge

| # | Field / concept | Lives today | Behavior today | Proposal | Justification |
|---|---|---|---|---|---|
| 8 | `house_treasury_` (House ledger gold) | `networking.hpp:226`; flat `networking.cpp:736`; dual into Chronicle `:3002` | disk (dual) | durable | "House owns stores … durable advancement" (constitution:62-66). |
| 9 | `house_progression_` (first-investment choice, first-clear flag, reward claimed, gear tier, income) | struct `native/include/verdigris/house_progression.hpp:24-33`; flat `networking.cpp:736`; archive encoder `account_archive.hpp:138-140`; dual `:2991-3004` / restore `:3006-3030` | disk (dual) | durable | Owner-demo House investment must survive; restart test asserts choice (`test-account-restart.mjs:124,135`). |
| 10 | `daily_purse_claimed_` | `networking.hpp:229`; set once `:4768-4773`; archived `:737`; **no reset anywhere** (grep: only these two sites) | disk | durable (flag G4) | Durable is correct, but "daily" never date-resets → effectively once-ever per account. Needs a ruling, not a code change from this lane. |
| 11 | `home_pitch_index_` | `networking.hpp:230`; archived `:738`; deterministically recomputed from house id at set-out `:4763-4767` | disk | **reconstructible** | Pure function of `active_house_id_`; storing it is redundant but harmless. |
| 12 | `campaign_complete_` (House-wide, inherited by successors) | `networking.hpp:235`; flat `:738`; dual from Chronicle `:4730-4732`,`:4887-4889` | disk (dual) | durable | Constitution: campaign completed once per House (`VERDIGRIS_CONSTITUTION.md:76-78`). |
| 13 | `house_renown_` | `networking.hpp:236`; flat `:738`; dual `:3157` | disk (dual) | durable | House-level social progression. |
| 14 | `cleared_nodes_` (world-web warden kills; "dead stays dead") | `networking.hpp:249`; flat `:739`; dual `:3092-3100` / restore `:3102-3141` (validated against authored web) | disk (dual) | durable | Route/expedition knowledge is House property (constitution:41-43). |
| 15 | `endgame_maps_completed_` | `networking.hpp:260`; flat `:739`; dual `:3156` | disk (dual) | durable | House endgame board progress. |
| 16 | `endgame_masteries_` (4 families × 16 tiers board) | `networking.hpp:270`; flat `:739`; dual `:3155` / restore `:3161-3175` (key-validated) | disk (dual) | durable | First-Warden mastery is House memory. |
| 17 | `trophy_fragments_` (Vesselforge trophy fragment counts) | `networking.hpp:243`; flat `:738`; dual `:3177-3189` / restore `:3191-3203` | disk (dual) | durable | "Complete and incomplete hunted trophies belong to the House" (`networking.hpp:241-242`). |
| 18 | `kitted_scions_` (starter-kit granted set) | `networking.hpp:271`; archived `:739`; gates re-grant `:4776-4782` | disk | durable | Prevents re-minting the starter purse across restarts. |
| 19 | `house_store_` (protocol House bank / extraction drain) | `networking.hpp:306-308`; archived `:741`; projected as `houseStoredItems` `networking.cpp:1757` | disk | durable | Extraction and stairs-up drain here; losing it on restart would void the core loop's "extract value to the House". |
| 20 | `bank_` (personal bank, `GameItem` vector) | `networking.hpp:240`; archived `:738`; item encoder `account_archive.hpp:129-133`; projected `:1609` | disk | durable | Restart test asserts deposit + equality (`test-account-restart.mjs:120-125,135`). |

### 2.3 Scion (per-Scion blob; `archive_scion`, `networking.cpp:722-729`)

| # | Field / concept | Lives today | Behavior today | Proposal | Justification |
|---|---|---|---|---|---|
| 21 | `inventory_` (the bag: 12×7 `PlayerInventory`) | `networking.hpp:304`; class `core.hpp:768-798`; archived `:723`; full `GameItem` shape incl. uuid, vessel block, expedition-map block `account_archive.hpp:129-133` | disk | durable | Restart test deep-equals `inventoryDetails` across processes (`test-account-restart.mjs:73,130,135`). |
| 22 | `wear_` (equipped seats, `WearSet`) | `networking.hpp:305`; class `core.hpp:801-829`; archived `:723` | disk | durable | Equipment is Scion build; test deep-equals `wearDetails` (`:131,135,141`). |
| 23 | `combat_xp_` | `networking.hpp:278`; archived `:723`; projected by the reserved `state.xp` block `networking.cpp:1553-1563` | disk | durable | XP is the Scion's earned progression; test asserts xp equality across restart (`test-account-restart.mjs:73,135`). No change to the wire block proposed (another agent's reservation). |
| 24 | Passive tree (`passive_tree_`, `passive_tree_saved_`) | `networking.hpp:223-224`; archived `:723-724`; intake handler validates nothing (`networking.cpp:2105-2114`); `schemaVersion=2` written in projection `:2095` and never checked on load | disk | durable (flag G2) | Allocated build must persist (test `:113-114,123,135`); version-policy gap carried from TASK-0097 R5. |
| 25 | `quest_points_` / `tree_quest_points_` (passive budget) | `networking.hpp:218-221`; archived `:724`; also re-derived from Chronicle at select `:3072-3075` | disk (dual-ish) | durable | Budget is earned; dual derivation noted under G1. |
| 26 | Lifecycle (`lifecycle_`, `lifecycle_deaths_`, `lifecycle_mode_`) | `networking.hpp:210-212,285`; archived `:724-725`; load revives to `alive` unless `permadead` `:797-803`; reconnect re-derives hard/soft `:873-887` | disk, with deliberate revive-on-load | durable (permadead) / reconstructible (soft-death) | "A final death remains final" (`networking.cpp:794-796`); test asserts permadead survives restart and re-admission (`test-account-restart.mjs:152-158`). Soft respawn state is intentionally reset. |
| 27 | Mortal oath (`mortal_oath_`) | `networking.hpp:286`; archived `:725`; re-derived from Chronicle roster `:882-887` | disk + reconstructible | durable | Oath is the admission contract (see #6). |
| 28 | First-goal machine (`first_goal_stage_`, started/completed ms) | `networking.hpp:215-217`; archived `:725-726`; preserved across reconnect `:891`; reset at select `:4906` | disk | durable | Onboarding checkpoint survives restarts by design. |
| 29 | `best_depth_` | `networking.hpp:292`; archived `:726`; updated `:4685` | disk | durable | Scion expedition history (constitution:38-40). |
| 30 | Actor stats + alive flag (level, attributes, life/resource pools, attack/defense…) | `networking.cpp:727-728`; `ActorStats` encoder `account_archive.hpp:134-137`; struct `core.hpp:106-122`; life/resource refilled on load `networking.cpp:801-802` | disk, pools topped on load | durable (stats) / reconstructible (current life, resource) | Level/attributes are the Scion; current wounds are session state (fresh login arrives healthy, `:910-912`). |
| 31 | Reserve-Scion store (`scion_saves_`) | `networking.hpp:119`; archived `:741`; written `remember_scion` `:755-760`; read `resume_scion` `:762-782` (fresh-Scion reset `:765-775`) | disk | durable | Reserve Scions "wait in reserve" (constitution:38-41); test asserts reserve switch preserves inventory/wear (`test-account-restart.mjs:128-131`). |

### 2.4 Recovery / relic state (D-106/D-109 boundary)

| # | Field / concept | Lives today | Behavior today | Proposal | Justification |
|---|---|---|---|---|---|
| 32 | `pending_relic_items_` / `pending_relic_count_` (fallen Scion's committed gear) | `networking.hpp:293-294`; captured at final death `networking.cpp:4171-4182`; archived `:740` | disk | durable | "Carried value is never destroyed — capture it into circulation" (`:4171`); constitution:55-60. |
| 33 | Relic provenance (`relic_source_scion_name_`, `relic_source_scion_id_`) | `networking.hpp:295-296`; archived `:740-741` | disk | durable | Recovery needs to name whose heirloom it is. |
| 34 | Relic circulation pool (world-state ledger, process-global static) | `networking.cpp:1359-1361`; per-owner slice archived by `archive_account_relics` `:1362-1380`, called `:752` | disk (per-owner file) | durable (flag G3) | Each account's own slice is checkpointed; **other** accounts' relics re-enter the pool only when their owner logs in (§4 G3). Test proves same-account recovery after restart (`test-account-restart.mjs:159-162`). |
| 35 | Crypt relic status (`queued`/`recovered`/`recoveredAt`) | inside Chronicle JSON `:4846-4855`, `:3311-3340` | disk (via #3) | durable | Rides the Chronicle document. |
| 36 | Core pending relic pools (`pending_relic_items_`, `pending_relic_trophies_`, resurfaced-trophy requeue) | `core.hpp:421-426`; snapshot absorbs surfaced ground candidates `core.cpp:1268-1294`; restore `:1352-1353`; embedded via `networking.cpp:742-750` | disk (inside core blob) | durable | D-106/D-109 semantics locked by core tests (TASK-0097 FINDINGS §0, §3). |

### 2.5 Core Simulation slice (embedded blob, `core.cpp:1229-1297` / `:1300-1378`)

| # | Field / concept | Lives today | Behavior today | Proposal | Justification |
|---|---|---|---|---|---|
| 37 | Core RNG (`rng.state`, `rng.serial`), `tick`, `nextLegendOrdinal` | `core.cpp:1232-1235`; restored `:1310-1313` | disk | durable | Restarts must not reroll streams (ADR-002 consequence; TASK-0097 §2.1). |
| 38 | Core House (`house.id/name`, routes, unlocked/cleared routes, specializations, stored trophies/items, relic candidates, lost trophies, seasonal rewards, legends, campaignComplete) | struct `core.hpp:175-194`; write `core.cpp:1237-1260`; read `:1315-1343` | disk — but see G5 | durable (flag G5) | Format-durable; however the production session treats core `house` as const (`networking.hpp:306-308`) and dispatches only `Command::enter` into the sim (`networking.cpp:4390`), so in real saves this House track is near-default/stale while the Chronicle track (#3, #8-#17) carries the live House. Architect ruling needed on which House is canonical. |
| 39 | Core Scion + `fallenScions` | struct `core.hpp:213-222`; write `core.cpp:1262-1266`; read `:1345-1351` | disk — same caveat as #38 | durable (flag G5) | Same dual-track concern; the live Scion profile is #21-#31. |
| 40 | Session RNG (`session_rng_`) and forge RNG (`world_->forge().rand()`) | `networking.hpp:309`; forge `core.hpp:1170-1172`; archived `networking.cpp:743-744`; restored `:748-749` | disk | durable | Deterministic vessel/tablet rolls continue across restart; test proves saved UUIDs never re-issue (`test-account-restart.mjs:142-144`). |

### 2.6 Transient by design (lost on restart, rebuilt or reset)

| # | Field / concept | Lives today | Behavior today | Proposal | Justification |
|---|---|---|---|---|---|
| 41 | Position / scene / world (`world_`: scene id, tile position, facing, monsters, telegraphs, combo/cooldown state) | `networking.hpp:313`; `core.hpp:1054-1056`; load resets `networking.cpp:794-796`,`:805`; reconnect `:913` | rebuilt (town spawn) | transient | "A resumed character arrives in town" (`networking.cpp:794-796`); test asserts `sceneType == 'town'` (`test-account-restart.mjs:136`). |
| 42 | Ground items / trophies on floors (`world_->ground_items()`) | `core.hpp:1166-1169` (per-scene lists retire with the floor) | lost | transient | Matches "unextracted value can be lost" (constitution:52-54); only relic candidates are rescued (#32/#36). |
| 43 | Respawn timers (`respawn_at_ms_`, `respawn_protection_until_ms_`, `prepare_final_death_`) | `networking.hpp:212-213,291`; not archived; zeroed `:797-803`,`:888-890` | rebuilt | transient | Soft-death ward is session-scoped by design. |
| 44 | UI/session panes (`shop_open_`, `bank_open_`, `shop_npc_id_`), pending chronicles flag | `networking.hpp:244-246,284`; reset `:892-899` | rebuilt | transient | Presentation state. |
| 45 | Combat clocks (`combat_clock_ms_`, `resource_regen_at_ms_`, war-cry fields, `active_skill_id_`) | `networking.hpp:272-276`; reset `:894-898` | rebuilt | transient | In-flight combat state dies with the scene. |
| 46 | World-web *position* (`current_node_*`, `current_child_*`, `node_warden_dead_on_entry_`) | `networking.hpp:250-255`; cleared `:900-901`; not archived | rebuilt | transient | Current expedition location; cleared set (#14) is the durable part. |
| 47 | Active endgame roll (`endgame_active_`, `endgame_completed_`, `endgame_map_tier_`, `goods_found`, name/family/objective_key/modifiers) | `networking.hpp:258-266`; copied from the consumed tablet `networking.cpp:2713-2726`; cleared on reconnect `:902-909`; **not in `archive_account`** | **lost across restart** | transient-by-current-design — **flag G6** | The tablet item is destroyed on open (`:2713-2715`) and the copied roll is neither archived nor recoverable; a restart mid-expedition loses both. Possibly intended ("one expedition", `:2744-2745`), but unverifiable from code alone. |
| 48 | `last_cleared_floor_key_`, `last_instance_theme_`, `last_instance_layout_` | `networking.hpp:228,237-238`; written `:2359-2360`,`:2652-2653`,`:2739-2740`,`:2817-2818`,`:4396-4397`; not archived | lost | transient | In-session dedup/naming helpers; guards like `first_clear_completed` make restart re-award impossible (#9). |
| 49 | `quick_start_` | `networking.hpp:208`; constructor arg `networking.cpp:702-705` | rebuilt per login envelope | transient | Login-mode flag, not profile data. |
| 50 | Party registry (`parties_`, `party_by_uuid_`), connections, `sessions_` map | `networking.hpp:345-362`; no archive path (grep: absent from `archive_account`) | lost | transient | Multiplayer lobby state is process-scoped; sessions are rebuilt from disk at login. |

### 2.7 Wire-only projections (never storage)

All `state.*` keys emitted by `ProtocolSession::snapshot()`
(`networking.cpp:1547-1759`) — identity/position (`:1549`), lifecycle
(`:1550-1551`), theme (`:1552`), **xp** (`:1553-1563`, reserved block,
read-only here), endgame projection (`:1564-1603`), chronicles summary
(`:1605`), bestDepth (`:1606`), quests (`:1607`), questPoints (`:1608`), bank
(`:1609`), passiveTree (`:1610`), houseInvestment (`:1611-1624`), attributes
(`:1625-1636`, a *stub approximation*: +2/attr per node until the geometric
tree engine is ported), static NPC table (`:1637-1656`), chroniclesRecord
(`:1657-1663`), lifecycleDetails (`:1664`), hp/resource (`:1665-1669`),
cooldown/cadence/bond timers (`:1670-1689`), monsters (`:1690-1710`),
sceneMetadata (`:1711-1739`), level/inventory/wear/wornItems
(`:1741-1753`), combat totals (`:1754`), ground/dropped items
(`:1755-1756`), houseStoredItems (`:1757`), groundTrophies (always empty,
`:1758`). Classification for every one of these: **reconstructible** — each is
derived per request from durable fields (#1-#40) or transient state (#41-#50);
none is itself a persistence seam.

## 3. Acceptance coverage checklist

| Acceptance concept | Status | Evidence |
|---|---|---|
| **House** | ACCOUNTED FOR | Chronicle document (#3) + treasury (#8) + investment (#9) + campaign/renown/world-web/endgame/trophy House tracks (#12-#17) + House store (#19) — all in the `.vgs` archive (`networking.cpp:735-741`). Core-format House block also embedded but near-dormant in production (#38, G5). |
| **Scion** | ACCOUNTED FOR | Per-Scion archive blob (#21-#31: `archive_scion`, `networking.cpp:722-729`) + reserve store `scion_saves_` (#31) + roster/crypt in Chronicle (#3, #6). |
| **wear** | ACCOUNTED FOR | `wear_` slots archived `networking.cpp:723`; encoder `account_archive.hpp:129-133`; restart test asserts (`test-account-restart.mjs:131,135,141`). |
| **bag** | ACCOUNTED FOR | `inventory_` archived `networking.cpp:723`; `PlayerInventory` `core.hpp:768-798`; restart test asserts (`test-account-restart.mjs:130,135`). |
| **bank** | ACCOUNTED FOR | `bank_` archived `networking.cpp:738`; deposit path tested (`test-account-restart.mjs:120-125,135`). House store separately covered (#19). |
| **passives** | ACCOUNTED FOR | `passive_tree_`/`passive_tree_saved_` archived `networking.cpp:723-724`; budget fields `:724`; restart test asserts (`test-account-restart.mjs:113-114,123,135`). Unvalidated `schemaVersion=2` flagged (G2). |
| **XP** | ACCOUNTED FOR | `combat_xp_` archived `networking.cpp:723`; projection at the reserved `state.xp` block `networking.cpp:1553-1563`; restart test asserts xp equality (`test-account-restart.mjs:73,135`). |
| **recovery** | ACCOUNTED FOR | Pending relic items/count/provenance (#32, #33: `networking.cpp:740-741`), circulation pool slice (#34: `:752,1362-1380`), crypt relic status (#35), core pending pools (#36: `core.cpp:1268-1294`). Restart test proves exact-UUID relic recovery (`test-account-restart.mjs:159-162`). Cross-account caveat at G3. |

Nothing in the acceptance line is ABSENT. Concepts the constitution names but
that have **no field anywhere at this head** (honest ABSENT flags, not
inferred): a Season/league layer (constitution:44-45 — no season field in any
archive or struct), asynchronous trading / currency exchange
(constitution:63-66 — no fields), mercenaries (constitution:85-90 — no
fields), and the Chronicle-side Season inheritance rule (constitution:45,
explicitly unresolved). These are future-work absences, not inventory gaps in
existing systems.

## 4. Gap list — durable per constitution, currently not / unverifiable

- **G1 — Dual-track truth (flat archive fields vs Chronicle JSON).** Treasury,
  campaign state, renown, cleared road nodes, endgame masteries, trophy
  fragments, and quest progress are each stored twice: as flat archive fields
  (`networking.cpp:736-739`) *and* inside the archived Chronicle document via
  `persist_*` (`:2991-3203`). They stay consistent only because both are
  checkpointed in the same atomic write and `restore_*` re-reads the Chronicle
  at set-out/select (`:4742-4746`,`:4897-4907`). Any future writer that
  updates one track without the other silently forks House memory. Needs an
  architect ruling on the single source of truth. (TASK-0097 could not see
  this; it post-dates the audit.)
- **G2 — Unvalidated inner version seams (TASK-0097 R5 persists).** The
  account envelope hard-rejects wrong magic/version/checksum
  (`account_archive.hpp:161`), but the Chronicle `version: 3`
  (`networking.cpp:4284`) is never checked on load, and
  `handle_skilltree_save` accepts any passive-tree blob without checking its
  `schemaVersion` (`:2105-2114`). A stale or foreign inner document loads
  silently.
- **G3 — Cross-account relic availability is process-state-dependent.** The
  circulation pool is a process-global static (`networking.cpp:1359-1361`);
  each account archives only its own slice (`:1366-1371`). After a restart, an
  account's owed relics re-enter circulation only when that account logs in
  (`:752`,`:1372-1379`). A relic owed to an account that never returns is safe
  on that account's disk file but permanently out of circulation for others.
  Whether that matches the "relics surface for any survivor" intent
  (`:1356-1358`) needs a ruling.
- **G4 — `daily_purse_claimed_` never resets.** Set once at first set-out of
  the process-lifetime (`networking.cpp:4768-4773`), archived (`:737`), and no
  date/day rollover exists (grep: two sites total). The "daily" road purse is
  currently once-ever per account across restarts. Durable storage is correct;
  the reset policy is missing/unverifiable.
- **G5 — Two House models; the core-format House track is near-dormant in
  production.** The embedded core snapshot serializes `house.*` routes,
  trophies, legends, etc. (`core.cpp:1237-1260`), but the session layer treats
  core `Simulation::house` as const (`networking.hpp:306-308`) and dispatches
  only `Command::enter` into the simulation (`networking.cpp:4390`). In real
  saves this track is expected to hold constructor defaults ("House Verdigris",
  `networking.cpp:705`) while the Chronicle track carries the live House.
  Classification of #38/#39 as "durable" is format-true but production-reach
  is indirect — flagged as the inventory's least-certain rows.
- **G6 — Consumed charted tablet + active endgame roll are lost on restart.**
  `open_expedition_map` destroys the tablet item (`networking.cpp:2713-2715`)
  and copies its roll into session fields (`:2719-2726`) that are neither
  archived nor in the reconnect-surviving set (`:902-909`). A crash
  mid-expedition forfeits the tablet with no recovery path — an unrecoverable
  loss not tied to a death decision (contrast constitution:52-54). Intent
  ("opens for one expedition", `:2744-2745`) is plausible but unverifiable
  from code.
- **G7 — Adapter power-loss window and `.tmp` collision (carried from
  TASK-0097 R3/R4, still true at this head).** No fsync/FlushFileBuffers of
  temp data before rename (`adapter.hpp:27-47`); fixed `.tmp` suffix
  (`adapter.hpp:25`). The process-level save-directory lock
  (`server_main.cpp:21-42,66-75`) and the per-session mutex make concurrent
  same-file writers unreachable in the current server, but the structural gap
  is unchanged.
- **G8 — Checkpoint cost / growth is unbounded by policy.** Every envelope and
  every 20 Hz tick runs a full account serialization and dirty-compare
  (`networking.cpp:812-838`,`:840-855`); the only size guard is a 32 MB
  load-time cap (`:789-790`). Chronicle growth (legends/crypt/history) has no
  pruning rule at this layer. Not a correctness gap today; a durability-adjacent
  risk for the architect.

## 5. What the restart test proves — and does not prove

`native/tools/test-account-restart.mjs` boots real server processes against a
temp `--save-dir`, kills the child with SIGKILL (no graceful final save,
`:65-69`,`:132`), and re-logs the same identity in a fresh process.

**Proves survives a true process restart** (the negative-control mapping —
these assertions are exactly what a Simulation-only save could not satisfy):

- Pre-admission Hardcore/soft creation checkbox is durable (`:83-94`).
- Gold purse, full item rolls (`inventoryDetails`), equipped wear
  (`wearDetails`), passive tree, quest chain + points, **xp**, Chronicle
  record, bank contents, House investment, endgame board — all `deepEqual`
  across a new process (checkpoint tuple `:73-75`; assertions `:123-135`).
- Reserve-Scion switch preserves inventory and equipment in-session
  (`:128-131`).
- Resume lands in town (`:136`); post-restart instance round trip keeps gold
  and equipment (`:137-141`).
- New-process loot never reuses a saved UUID (RNG/identity namespace
  continuity, `:142-144`).
- Mortal final death is durable: `permadead` across restart, selecting the
  fallen cannot resurrect, re-admission cannot strip the oath (`:145-158`).
- Successor recovers the exact circulating relic (same UUID) after restart
  (`:159-162`).
- Single-writer lock refuses a second process on the same save dir (`:163-166`).
- Failed storage stops mutations and preserves the last good checkpoint
  (`:167-180`); a corrupted save is rejected at login and the damaged file is
  preserved, never silently overwritten (`:182-192`).

**Does not prove:**

- Anything about fields absent from the `dev:state` wire projection used as
  the assertion oracle: `scion_saves_` internals across restart (only the
  in-process switch is checked), core-track House/legends (#38/#39),
  `session_rng_`/forge streams beyond UUID non-reuse, `daily_purse_claimed_`,
  `home_pitch_index_`, `first_goal_*`, `best_depth_`, `lifecycle_deaths_`,
  `kitted_scions_`.
- Inner version seams: stale Chronicle `version` or passive-tree
  `schemaVersion` documents are never rejected, and no test tries them (G2).
- Power loss between temp write and rename (TASK-0097 F-12; structural, G7).
- Cross-account relic circulation after restart (only same-account recovery is
  exercised, G3).
- Mid-expedition tablet loss semantics (G6 is untested either way).
- The consumed-tablet/mid-instance crash boundary in general: the test always
  returns to town before killing the process.

## 6. Areas where evidence was too indirect to classify confidently

1. **Core-track House/Scion/legends (#38, #39)** — format-durable, but
   production mutation paths into the core `Simulation` are minimal (one
   `dispatch(Command::enter)` site, `networking.cpp:4390`; const House,
   `networking.hpp:306-308`). Whether real saves ever carry non-default core
   House data could not be established without running the server, which this
   read-only lane does not do. → G5.
2. **G3's product intent** — the code comment says relic circulation is world
   state surfacing "for any survivor" (`networking.cpp:1356-1358`), but the
   storage reality is per-owner files plus a process-global pool. The
   classification (durable per-owner, availability process-dependent) is
   solid; whether that satisfies the design is a ruling, not evidence.
3. **G6's intent** — "opens for one expedition" (`networking.cpp:2744-2745`)
   hints the loss may be accepted design, but no doc or test at this head
   states the restart-mid-expedition contract.
4. **`daily_purse_claimed_` (G4)** — the field name says "daily"; nothing in
   code or docs at this head defines the rollover. Classified durable with the
   reset policy flagged as missing.

## 7. Summary classification counts

- **durable (on disk in `.vgs`):** rows 1-6, 8-10, 12-40 (incl. dual-track and
  embedded-core rows), i.e. the entire House/Scion/wear/bag/bank/passives/XP/
  recovery surface.
- **reconstructible:** row 11 (home pitch), current life/resource pools
  (row 30 partial), and every wire projection (§2.7).
- **transient-lost (by design):** rows 41-46, 48-50.
- **transient-lost (needs ruling):** row 47 (G6).
- **ABSENT at this head:** Season layer, async trading/currency exchange,
  mercenaries (§3 tail).
