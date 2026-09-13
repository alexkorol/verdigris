# FINDINGS — TASK-0104: Itemization, extraction, and item-history gap audit

Lane `ox-pc-bb` · BOUNDED-DESIGN · read-only audit · capsule honored (no ports,
no writes outside this folder). Base `d2423873c577d299b3b39c56024d1d840993c72b`
(ancestor of claim head `fca5b7c5`). Every claim cites `file:line`. Machine twin:
`captures/item-lifecycle.json`.

Scope note: the native tree carries **two disjoint item universes** — (A) the
D-114 combat simulation (`Item`, `Scion`, `House`) and (B) the tile-space
browser-parity world (`GameItem`, `VesselItem`, `PlayerInventory`, `WearSet`,
`GroundItem`, N4/N5 waves). Both are mapped below; the split itself is finding
G-04.

---

## A. Core simulation item system

### A1. Model and stable identity
- `Item { id, name, attack_bonus, owner_id, use_count, equipped,
  relic_candidate, history }` — native/include/verdigris/core.hpp:124-133.
  Carried by `Scion::carried_items` (core.hpp:217); House holds
  `stored_items`, `relic_candidates`, `lost_trophies` (core.hpp:180-184).
- IDs minted by the seeded simulation RNG: `rng_.token("item")`
  (native/src/core.cpp:743), trophies `rng_.token("trophy")`
  (core.cpp:781). Because RNG state is snapshotted/restored
  (core.cpp:1229-1230), identities are reproducible and byte-stable across a
  save/load round trip (test: "restored RNG state reproduces generated item and
  trophy drop identities", native/tests/core_tests.cpp:816).
- Single-owner invariant enforced by construction: an item moves
  pool → ground → carried → stored exactly once per hop; resurfacing pops the
  oldest pool entry before pushing to the ground (core.cpp:755-764).
- GREEN: identity preserved across pickup/equip/use
  (core_tests.cpp:1557), death registration (core_tests.cpp:677, 720-722),
  extraction (core_tests.cpp:699-703).

### A2. Generation / drops
- `drop_reward()` runs on every monster death inside an active instance
  (core.cpp:833-834): one generated weapon ("Ember-edged axe",
  attack_bonus 4 + rng(0..3)) with first history line "forged by the
  expedition seed" + `ItemDropped` (core.cpp:741-749); one trophy +
  `TrophyDropped` (core.cpp:781-784).
- Ground registry authority: `instance_.ground_item_ids` /
  `ground_trophy_ids` (core.hpp:306-308) decide what is pickable;
  stale references after instance retirement are rejected by design
  (core.cpp:480-483; test `test_instance_lifecycle_rejects_stale_pickups`,
  core_tests.cpp:979-1027).
- GREEN: drop presence + determinism (core_tests.cpp:68-69, 1141-1144,
  1316-1335).

### A3. Pickup → carried
- `resolve_pickup` (core.cpp:470-511): registry check, `owner_id =
  scion_.id`, history += "picked up", `ItemPickedUp`; trophies likewise
  (`TrophyPickedUp`), with `resurfaced_trophy_ids_` bookkeeping
  (core.cpp:498-500).
- GREEN: covered throughout (helpers core_tests.cpp:68-84; journey-level
  assertions core_tests.cpp:695-697).

### A4. Equipment behavior change (equip/unequip/use)
- `resolve_equip`: single-seat semantics — unequips everything, marks one item,
  history += "equipped", sets `Actor::equipped_item_id`, emits `ItemEquipped`
  + `ItemHistoryUpdated("equip")` (core.cpp:513-524).
- Behavior reaches combat through `equipped_attack_bonus()`
  (core.cpp:927-932) fed into `resolve_damage(..., item_bonus)`
  (core.cpp:410, 423-428): the only equipment-driven stat delta in either
  universe.
- `resolve_unequip` clears flag + actor ref, emits `ItemHistoryUpdated
  ("unequip")` (core.cpp:528-538).
- Use: `Interact "use:<id>"` bumps `use_count`, appends "used at tick N"
  history + `ItemHistoryUpdated` (core.cpp:441-452); every resolved player
  attack also records equipped-item use (record_equipped_item_use,
  core.cpp:430-439, called at core.cpp:420).
- GREEN state coverage: equip marks flag/ref (core_tests.cpp:1550-1555),
  use increments (core_tests.cpp:1558), unequip clears both
  (core_tests.cpp:1560-1563).
- RED (event layer): see G-05 — the `ItemHistoryUpdated` emissions themselves
  are asserted by **no** automated test.

### A5. Extraction
- `resolve_extract` (core.cpp:901-920): gated on alive + active instance +
  `at_extraction()` (Manhattan distance ≤ `kExtractionRange`, core.cpp:922-925,
  derived from the D-114 table core.hpp:52,67-68). Carried items/trophies move
  to `house_.stored_items/stored_trophies`, emit `ItemExtracted/
  TrophyExtracted/HouseStoreChanged`; relic candidates additionally get a
  "relic_extracted" legend (core.cpp:907-910). Carried vectors cleared;
  instance retired.
- GREEN: `test_extraction` (core_tests.cpp:1398-1408); extraction closes the
  expedition (core_tests.cpp:1245, 1404).

### A6. Death recovery / relic candidacy (D-106)
- Player `handle_death` (core.cpp:809-899): **every** carried item becomes a
  relic candidate — copied into `house_.relic_candidates` with
  `relic_candidate = true` plus a distinguishing history line: "registered
  after Scion death" (was equipped) vs "lost at <route>, awaiting recovery"
  (pack item) (core.cpp:868-883). Trophies → `house_.lost_trophies`
  (core.cpp:884-888). Legends `relic_candidate`/`trophy_candidate`/
  `scion_death` recorded; `ScionLost` emitted; instance retired.
- Successor does not inherit: `create_successor` starts empty
  (core.cpp:939-951); the fresh-profile rule is also enforced wire-side on
  scion admission (`wear_.clear(); inventory_.clear()` + purse regrant,
  native/src/networking.cpp:2734-2738).
- GREEN: `test_d106_all_carried_value_is_recoverable`
  (core_tests.cpp:1441-1488, incl. ordered candidates + distinct history
  wording 1466-1473), `test_death_and_successor` (1410-1439),
  `test_first_expedition_wave_death_recovery_interaction` (1336-1396),
  `test_death_retires_floor_without_double_registering_relics`
  (1029-1048).

### A7. Relic resurfacing / recovery circulation
- Re-entry **only** through the seeded reward stream: `drop_reward()` rolls
  1-in-`kRelicResurfaceOneIn` to pop the oldest relic candidate onto the
  floor, appending "resurfaced on route <id>" history, emitting
  `RelicResurfaced` + legend (core.cpp:755-764). Lost trophies reuse the same
  cadence with `TrophyResurfaced` (core.cpp:770-779).
- Abandonment safety: `retire_instance()` folds surfaced relic-marked ground
  items back into `pending_relic_items_` exactly once; surfaced trophies
  likewise via `resurfaced_trophy_ids_` (core.cpp:563-605); pending pools
  reattach to the next entered instance (core.cpp:548-558). Ordinary floor
  drops die with the instance.
- Loss-after-recovery returns the item to the pool once, identity intact
  (GREEN: `test_relic_loss_again_returns_once`, core_tests.cpp:707-727);
  full round trip death→resurface→pickup→extract with legends
  (GREEN: core_tests.cpp:661-705); ordering/determinism
  (GREEN: core_tests.cpp:1490-1541); replay determinism
  (GREEN: core_tests.cpp:729-753).

### A8. Scars / significant-item history
- Core `Item.history` is an append-only string ledger written at forge
  (A2), pickup (A3), equip (A4), use (A4), death registration (A6), and
  resurface (A7). Legends add the bounded House-level record
  (`kLegendCapacity = 64`, eviction of oldest non-founding, core.hpp:193,
  core.cpp:286-293; GREEN cap test core_tests.cpp:1646-1665, stable-id replay
  1667-1691).
- Wire-visible? The core `Item.history` list is **not** serialized into any
  networking payload (see C); presentation sees only event-feed strings
  (main.cpp:1851-1855). See G-06.
- AMBER: no scar-producing transition exists in the core system either; the
  word "scar" appears in the parity tooltip/sear code only (B2/B3).

## B. Tile-space parity world (N4/N5)

### B1. Model and stable identity
- `GameItem` (catalogue id + `uuid`, grid slot/size, ratings, vessel block,
  `bound_to`) — core.hpp:604-625. `VesselItem` (form/material/ilvl/vessel
  capacity/scars/patience/brands/epithet) — core.hpp:468-482.
- **Identity is process-global and per-run**: `next_item_uuid()` is a
  monotonic counter formatted as a v4-shaped uuid
  (native/src/core.cpp:2679-2688); `VesselForge::gen_id` mixes a counter with
  forge-rng draws (core.cpp:2289-2293). Comment states the contract is
  uniqueness only (core.cpp:2678-2679).
- RED (constitution tension): constitution requires items to "have stable
  identities and … gain history through ownership, use, survival, loss, and
  rediscovery" (docs/product/VERDIGRIS_CONSTITUTION.md:55-57). Counter uuids
  restart every launch, so rediscovery across sessions cannot be correlated.
  See G-03.

### B2. Generation / drops
- Seeded `VesselForge` (mulberry32 parity, core.hpp:450-459, 538-571):
  material roll gated by ilvl tier (core.cpp:2369-2382), vessel/patience rolls
  (2385-2388), brand-count weights + per-brand rolls (2390-2398), epithet at
  ≥3 brands (2405-2415). Sear spends patience, rolls on a clone, excludes
  used mods (2419-2439).
- Catalogue → instance: `create_game_item` honors size rules
  (core.cpp:2759-2781), vessel-block stat replacement
  (core.cpp:2800-2834), bind-on-pickup for weapon/armor/jewelry
  (core.cpp:2839-2845).
- Drops: `drop_monster_loot` — coins always (goods-found boosted), gear roll
  by rarity (0.05/0.1/0.2/0.5) capped 0.75, guaranteed elite gear under Proof
  of Temper (core.cpp:3147-3180); per-floor treasure hoard with depth-scaled
  ilvl `min(80, 10+(depth-1)*10)` (core.cpp:3182-3211, 3061-3063); loot-tile
  spiral avoiding stairs/blocked tiles (core.cpp:3120-3145); floor retirement
  clears ground items (core.cpp:3226).
- GREEN: ground-truth rolls (core_tests.cpp:1807-1871), sear rules +
  exclusion (1873-1894), inventory first-fit/overflow/currency
  (1896-1942), wear seats/caps (1944-1986), loot math + depth scaling
  (1988-2026), depth chaining + treasure (2028-2066+), kill drops land
  (2007-2024).

### B3. Scars
- `VesselItem.scars` counts against vessel capacity in `sear`
  (core.cpp:2420-2422) and renders a "✕ N scarred slot(s)" tooltip line
  (core.cpp:2502-2506). Initialized to 0 in generation; **no function in the
  native tree ever increments it** (`rg "scars" native/src native/include`
  shows only reads/serialization).
- RED: the scar lifecycle edge does not exist. Constitution names "scars" as
  part of the Brands & Bonds direction (VERDIGRIS_CONSTITUTION.md:56-58,
  139-143). Formula/balance stays owner-owned; the missing piece is the
  neutral seam (a transition that can mark a slot), not the math. See G-02.

### B4. Inventory / equip / extract (wire session)
- `PlayerInventory` 12×7 spatial grid, first-fit, currency stack-merge,
  overflow reporting (core.hpp:640-670).
- `WearSet` physical seats, ring duplication, swap-on-full, capped totals
  (core.hpp:673-700); totals feed session combat mods (networking.cpp:841,
  3039-3046 clamps).
- Session extract drains backpack **and** worn seats into the House store,
  emitting refreshes (networking.cpp:1012-1042); JS has no player:extract
  (comment 1012-1015).
- Gold auto-pickup near the player without Take (networking.cpp:1645).
- GREEN: journey pickup→equip→extract end-to-end
  (native/tests/session_tests.cpp:287-392); equip surfaces
  `player:equippedAnItem` (session/networking flows above).

### B5. Death / relic circulation (N5)
- Relic circulation is **world state**, not session state
  (networking.cpp:817 comment): fallen-scion heirlooms re-enter floors via
  `add_relic_ground_item` carrying `relic_record_id` + source-scion
  provenance (core.cpp:3094-3107); wire exposes `chroniclesRelic` and legacy
  alias (networking.cpp:474-486); recovery flips crypt status
  (`mark_relic_recovered`, networking.cpp:1674-1692+) and the client raises a
  dedicated toast on lost→recovered transitions
  (native/client/main.cpp:2667-2681, 3778-3786).
- GREEN: D-106 gate-b scenario drives succession → elite kill → surfaced
  heirloom → Take → reconnect → same crypt status
  (session_tests.cpp:550-555 header, 1275+, 1415, 1860-1900).

### B6. Persistence (tile-space)
- None. `ProtocolSession` keeps `inventory_`/`wear_` in memory
  (networking.cpp:584-587 initial purse; kept across socket replacement,
  networking.cpp:604-641); the session map itself is in-memory
  (networking.cpp:2988). `WorldSimulation` ground items and the forge stream
  have no serialization seam at all (no snapshot friend; core.hpp:860-1015).
  The only durable serializer is core-sim `snapshot()/restore()`
  (core.cpp:1226-1375), whose grammar has no GameItem form.
- RED: process restart discards every carried/worn/floor GameItem. TASK-0097
  separately proved the atomic file adapter (persistence/adapter.hpp) has zero
  production callers. See G-01.

## C. Wire payloads (item-bearing)
| Envelope | Producer | Contents | Cited |
|---|---|---|---|
| login/state snapshot `inventory`,`inventoryDetails`,`wear`,`wearDetails` | buildStateSnapshot | full slot lists + identity projections | networking.cpp:967-969 |
| `player` payload `inventory.slots` | player_payload | id/uuid/name/slot per item | networking.cpp:656-661 |
| `core:refresh:inventory` | emit_inventory_refresh | full identity list | networking.cpp:986-989 |
| `player:equippedAnItem` | emit_equip_state | wear + wearDetails + totals | networking.cpp:1000-1009 |
| `item:change` | ground mutations | floor ground list | networking_tests.cpp:259-266 |
| `world:itemDropped` | drops | uuid/id/name/x/y (+relic fields) | networking.cpp:462-486 |
| `chroniclesRelic`/`legacyRelicId` | relic ground items | relicId/scionId/scionName provenance | networking.cpp:474-486 |

- Core-sim item facts (`Item.history`, `use_count`, `relic_candidate`) ride
  only inside generic `Event` echoes if a transport forwards them; there is no
  item-history projection on the wire. AMBER (G-06).

## D. Presentation (native client)
- Authoritative-mirror discipline: local model copies `carried_items`,
  ground list, and stored count straight from the simulation
  (native/client/local_session.cpp:195-204, 213); remote model built from
  server snapshots only ("no House, Scion, oath, or relic is ever invented",
  main.cpp:2565).
- Controls per constitution (WASD/mouse/X/Z/F, VERDIGRIS_CONSTITUTION.md:94-99):
  X nearest pickup (main.cpp:942), Z toggles loot name labels
  (main.cpp:2119, 3523-3524), F contextual extract with mode-aware hint
  (main.cpp:3609-3625, hint fn 3806), gear pane I
  (main.cpp:236, 2130, focus-guarded 3139), extraction marker render op
  (main.cpp:3281-3285), banked/extraction HUD summary (main.cpp:2273),
  event-feed strings for pickup/extracted (main.cpp:1851-1855).
- Relic beats: crypt lost→recovered toast (main.cpp:2667-2681) and roster
  "heirloom <status> (N to circulation)" lines (main.cpp:2964-2971).
- Scenario harness proves the pipeline layers (render list + pane + HUD,
  native/README.md:97-126). GREEN: journey scenario asserts pickup/equip/
  extract through real commands (session_tests.cpp:287-392).
- GAP: no presentation surface renders core `Item.history`/scars (nothing to
  render — see G-02/G-06).

## E. Test coverage matrix (lifecycle edges)
| Edge | Implementation | Tests | Verdict |
|---|---|---|---|
| generate/drop item+trophy | core.cpp:741-785 | core_tests.cpp:68-69,1141-1144 | GREEN |
| pickup (+stale rejection) | core.cpp:470-511 | core_tests.cpp:979-1027 | GREEN |
| equip behavior change | core.cpp:513-524,927-932,410 | core_tests.cpp:1550-1555 | GREEN |
| unequip | core.cpp:528-538 | core_tests.cpp:1560-1563 | GREEN |
| use history (state) | core.cpp:430-452 | core_tests.cpp:1557-1558 | GREEN |
| **use/equip/unequip history EVENT** | core.cpp:436,448,523,536 | **none** (`rg ItemHistoryUpdated native/tests` → 0 hits) | **RED — negative control NC-1** |
| extract to House | core.cpp:901-920 | core_tests.cpp:1398-1408 | GREEN |
| death → relic candidacy (all carried) | core.cpp:866-899 | core_tests.cpp:1441-1488 | GREEN |
| successor inherits nothing | core.cpp:939-951; networking.cpp:2734-2738 | session_tests.cpp:1860-1900 (fresh profile) | GREEN |
| relic resurface round trip | core.cpp:755-764 | core_tests.cpp:661-705 | GREEN |
| loss-again returns once | core.cpp:563-605 | core_tests.cpp:707-727 | GREEN |
| surfaced→pending at snapshot boundary | core.cpp:1265-1291 | core_tests.cpp:847-878 | GREEN |
| restore rebuilds equipped ref | core.cpp:1367-1372 | core_tests.cpp:755-776 | GREEN |
| **snapshot version rejection** | core.cpp:1299-1301 | positive only, core_tests.cpp:763 | RED (NC-2, inherited open from TASK-0097) |
| tile-space generation rolls | core.cpp:2358-2439 | core_tests.cpp:1807-1894 | GREEN |
| tile-space drops/loot math | core.cpp:3147-3211 | core_tests.cpp:1988-2066 | GREEN |
| tile-space pickup→inventory→equip | networking take/wear paths | session_tests.cpp:287-392 | GREEN |
| tile-space extract | networking.cpp:1012-1042 | session_tests.cpp:366-392 | GREEN |
| relic circulation + recovery | networking.cpp:1674-1692; core.cpp:3094-3107 | session_tests.cpp:550-555,1415,1860+ | GREEN |
| **scar production** | none exists | none | RED (G-02) |
| **cross-session stable uuid** | core.cpp:2679-2688 | none possible in-process | RED (G-03) |
| **tile-space durability** | absent | absent | RED (G-01) |
| history projection to wire/UI | absent | absent | AMBER (G-06) |

## Ranked content-neutral gaps

- **R1 / G-01 (red) — Tile-space item world has no durability seam.**
  `inventory_`/`wear_`/ground items exist only inside a running server
  process; restart silently destroys carried value while the chronicle keeps
  claiming relic continuity. Neutral fix path: extend the existing
  deterministic field grammar (core.cpp:961-964) or route sessions through the
  idle adapter (persistence/adapter.hpp). Owner decisions (format/storage
  location) not required to prove the seam.
- **R2 / G-02 (red) — Scar lifecycle edge missing.** Capacity accounting and
  UI copy assume scars exist (core.cpp:2420-2422, 2502-2506) but no transition
  produces one; Brands & Bonds attunement/awakening sections are likewise
  hard-absent (tooltip comment core.cpp:2457-2458). Neutral seam: a single
  deterministic command/event pair that flips a vessel slot, formula-free.
- **R3 / G-03 (red) — Identity is not stable across sessions tile-side.**
  Process-global counters (core.cpp:2679-2688, 2289-2293) violate the
  constitution's stable-identity clause (lines 55-57) for any history that
  outlives a process; core-sim ids are fine because RNG state persists.
  Neutral seam: seed the uuid stream from durable state.
- **R3 / G-04 (red→amber) — Two item universes never converge.** Extracted
  GameItems land in a different House store shape than core `stored_items`;
  histories recorded in either are invisible to the other. Any future
  significant-item ledger must pick one identity spine. (Design decision
  flagged, not made.)
- **R4 / G-05 (red, negative control NC-1) — `ItemHistoryUpdated` emissions
  untested.** The event that carries "used/equipped/unequipped" to listeners
  has zero automated assertions (`rg -n "ItemHistoryUpdated" native/tests` →
  no matches), even though the underlying state is tested. Smallest locking
  test L1: after equip→use→unequip, assert the three typed emissions
  (ids + text) on `sim.events()`.
- **R5 / G-06 (amber) — No history/scars projection reaches wire or HUD.**
  Tooltip lines exist only inside VesselBlock JSON (networking.cpp:311-341);
  core `Item.history` never serialized (C table). Blocks the constitution's
  memorable-item promise until G-04 resolves.
- **R6 / G-07 (amber, NC-2) — Snapshot schemaVersion rejection path
  (core.cpp:1299-1301) untested**; only the positive header check exists
  (core_tests.cpp:763). Still-open carry-over from TASK-0097's negative
  control; it gates every recovery load.
- **R7 / G-08 (amber) — Equip idempotence smears history.** Re-equipping the
  already-equipped item appends another "equipped" line with no guard
  (core.cpp:513-523); harmless today, but history is a permanent ledger —
  noise compounds. Untested either way.

## Negative control (spec-required)
**NC-1:** the history transition *equipped-item use → `ItemHistoryUpdated`
emission* (core.cpp:430-439) — and its equip/unequip siblings — has **no
automated test** anywhere under native/tests (grep evidence above). State
(`use_count == 1`) is asserted at core_tests.cpp:1558, so only the event
contract is dark. Proposed smallest test: L1 in G-05.
Secondary (recovery gate): NC-2 schemaVersion rejection, G-07.

## Frozen invariants honored
- D-106 recoverability: mapped, currently green (A6/A7, E rows).
- House ownership: extraction/admission boundaries cited (A5, B4, B6 note).
- Significant-item history: mapped (A8, B3, G-05/G-06).
- Denylist firewall: untouched; no production file modified by this task.
- No affix math, economy rates, drop rates, or item names proposed anywhere
  above; all flagged stops are seams, formulas remain owner-only.
