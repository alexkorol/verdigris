# TASK-0104 — Itemization, extraction, and item-history gap audit (FINDINGS)

- Worker lane: `ox-sw-g` (read-only audit, BOUNDED-DESIGN; no production code modified)
- Base commit: `d2423873c577d299b3b39c56024d1d840993c72b`
- Head at time of audit: `d2423873` (+ docs-only commits in
  `orchestration/tasks/TASK-0104-itemization-history-gap-audit/` afterwards;
  see REPORT.md for exact SHAs)
- Spec note: `SPEC.md` did not exist at the base commit (the task folder was
  created by this lane). The task brief in the assignment is treated as
  binding; recorded as a deviation in REPORT.md.
- Companion machine-readable map: `captures/item-lifecycle.json`

## Method

Full reads of `native/include/verdigris/core.hpp`,
item/extraction/death/persistence regions of `native/src/core.cpp` and
`native/src/networking.cpp`, `native/persistence/adapter.hpp`, item-related
client code under `native/client/`, all four test files under `native/tests/`,
plus `docs/product/*` and `config/legacy-denylist.json`. Targeted greps for
stable-id, generation, equip, drop, pickup, extract, relic, scar, brand,
history vocabulary. Acceptance command output recorded in REPORT.md.

There are TWO coexisting item systems in `native/`:

1. **Core `Simulation` item model** (headless proof-of-loop): `Item`
   (native/include/verdigris/core.hpp:124), `Trophy` (:135), `House` pools
   (:172), `Scion.carried_items` (:210), seed-derived ids, durable history
   vectors, snapshot()/restore().
2. **Wire-facing N4/N5 `GameItem` pipeline** (browser parity):
   `GameItem` (core.hpp:575), `PlayerInventory` (:611), `WearSet` (:644),
   `GroundItem` (:676), `VesselForge`/`VesselItem` (:422-542),
   `WorldSimulation` ground/loot seams (:901-925), driven by
   `ProtocolSession` (native/src/networking.cpp).

The audit ranks gaps **within each system** and flags cross-system seams.
Owner-dependent content questions (affix math, rates, item names) are listed
separately and deliberately unresolved per frozen invariants / OD-002 /
OD-006.

## A. Content-neutral lifecycle gaps (implementation facts, RANKED)

### GAP-1 (P0, history) — Unequip emits a history event without a history line

`Simulation::resolve_unequip` (native/src/core.cpp:522-534) clears the
equipped flag and emits `EventType::ItemHistoryUpdated` with text `"unequip"`
(core.cpp:532) but **never appends to `item.history`**. Every other history
transition appends a durable line: use (core.cpp:431, 443), equip
(core.cpp:515), pickup (core.cpp:500), death registration
(core.cpp:809-811), resurfacing (core.cpp:713). Result: the event stream
claims a history update the durable record does not contain; a
snapshot/restore round-trip preserves the omission. No test asserts history
content across unequip (`test_item_identity_and_branch`,
native/tests/core_tests.cpp:1212-1237, asserts only flags/references at
:1228-1233).

### GAP-2 (P0, recovery / D-106 red risk) — Circulating relics are volatile across restarts

The wire-side relic circulation pool is a process-local static
(`circulation_pool()`, native/src/networking.cpp:748-750). Final-death
commits (networking.cpp:2133-2143) and elite-kill resurfacing
(networking.cpp:2083-2108) mutate it, but nothing persists it: server
restart silently destroys every circulating heirloom, and with it the
fallen Scion's crypt promise (`crypt[].relic.status="lost"`,
networking.cpp:2169-2176) becomes unrecoverable. D-106 ("no item is
destroyed by Scion death") therefore holds within a process lifetime only.
The legacy core's snapshot()/restore() DOES persist its recovery pools
(core.cpp:1182-1183, 1200-1223, 1263-1264, 1281-1282) but is never wired
into the running server (native/src/server_main.cpp contains no
snapshot/persistence call).

### GAP-3 (P1, identity) — Wire item uuids are process-scoped, not seed-stable

`GameItem.uuid` comes from `next_item_uuid()` over the process-global serial
`g_item_uuid_serial` (native/src/core.cpp:2608-2618, comment at :2608-2609:
"uniqueness is the only contract"). Two runs of the same world seed mint
different uuids, and uuids are not persisted anywhere. By contrast the core
`Item.id` is derived from the seeded RNG stream (`rng_.token`,
core.hpp:346; used at core.cpp:698, 736, 875-877) and replay-identical
(test_relic_resurface_replay_is_deterministic,
native/tests/core_tests.cpp:703-727). The constitution's "items have stable
identities" (docs/product/VERDIGRIS_CONSTITUTION.md:55) is satisfied by the
core model but only weakly by the wire model.

### GAP-4 (P1, storage) — Two House stores that never converge

Extraction drains backpack + wear into `house_store_`
(`finish_extraction`, native/src/networking.cpp:939-972), projected as
`houseStoredItems` (networking.cpp:910, 961-962). The countinghouse bank UI
operates on a **separate** `bank_` vector (`bank_items_json`
networking.cpp:1217-1227; withdraw/deposit menu actions
networking.cpp:1714-1727, 1908-1944). Extracted items never appear in the
bank screen; banked items never appear in `houseStoredItems`. No bridge,
migration, or shared authority exists. Structurally two parallel
"durable House storage" pools with different projections.

### GAP-5 (P1, extraction risk) — Wire extraction has no position/risk gate

Core extraction requires standing at the extraction point
(`at_extraction`, native/src/core.cpp:854-857, gated in `resolve_extract`
core.cpp:835). The wire path `handle_extract`
(native/src/networking.cpp:2116-2127) requires only `world_->in_instance()`;
stairs-up return and depth-return auto-bank via `finish_extraction`
(networking.cpp:2316, 2317, 2124). Any point in an instance extracts
everything risk-free. Diverges from the core risk model and from the
constitution's "push farther or return safely" decision weight
(docs/product/VERDIGRIS_CONSTITUTION.md:23-28, 52-53).

### GAP-6 (P2, death model seam) — Soft death keeps 100% of carried value on the wire

Soft-lifecycle death (`dev:kill` soft branch, native/src/networking.cpp:2390-2397)
sets `awaiting-respawn`; respawn restores life and position
(maybe_respawn, networking.cpp:2208-2223) while `inventory_`/`wear_` are
untouched. Only hard/final death circulates items
(handle_final_death, networking.cpp:2128-2143). In the core model every
death moves ALL carried value to recovery pools
(handle_death, core.cpp:803-823). Two different death-loss regimes coexist;
only the core regime is test-covered for item consequences.

### GAP-7 (P2, history visibility) — Core item history never reaches the wire

`item_identity_json` (native/src/networking.cpp:354-390) projects id, uuid,
boundTo, `affixes:{brand:null,bond:null}` (:371), vessel block, stats — but
**no history, use count, or relic provenance fields**. Core
`Item.history`/`use_count` (core.hpp:129-132) and the
`ItemHistoryUpdated`/`RelicResurfaced` events have no consumer in the
ProtocolSession pipeline (events stay inside the headless Simulation; the
client learns only via state refreshes). Significant-item history is
invisible to every native client today.

### GAP-8 (P2, admission consistency) — Binding applied inconsistently across acquisition paths

`dev:give` binds non-stackables to the grantee (native/src/networking.cpp:1008);
shop buy (networking.cpp:2357), wagon outfit buy (networking.cpp:2326-2328),
and kill/treasure loot (`drop_monster_loot`, `scatter_floor_treasure`,
native/src/core.cpp:3079-3143) mint **unbound** gear; `create_game_item`
applies binding only when `options.bind_to` is set
(native/src/core.cpp:2771-2777). Whether purchased/dropped gear should bind
is unspecified; today it silently differs per surface.

### GAP-9 (P2, trophy history) — Trophies carry no per-object history

`Trophy` is `{id, name}` (native/include/verdigris/core.hpp:135-138).
Trophy transitions exist only as events (`TrophyDropped/PickedUp/Extracted/
Resurfaced`, core.hpp:230-248) and legend records
(record_legend calls at core.cpp:732-733, 818-819). Loss/recovery order is
preserved by pool ordering (lost_trophies FIFO, core.cpp:725-734, 816-820)
but no durable per-trophy line sequence exists to satisfy "gain history"
symmetrically with items.

### GAP-10 (P2, documented deviation kept visible) — Equip swap onto a full grid spills instead of aborting

JS aborts the equip; native spills the displaced piece at the feet
(native/src/networking.cpp:1059-1064, comment names the documented N4
simplification). Recorded here because it is a lifecycle edge (displaced-item
destination) that only holds by convention.

## B. Owner-dependent content gaps (RANKED separately, not implementation bugs)

1. **Scar producer missing**: `VesselItem.scars` initializes to 0 and is
   never incremented anywhere (read at sear capacity core.cpp:2353 and
   tooltip core.cpp:2434-2436). Death-scar transformation
   (constitution :57-59; OD-002) has no rule set yet — deliberately not
   invented here.
2. **Bonds absent**: `affixes.bond` is always null on the wire
   (networking.cpp:371); no bond mechanic exists.
3. **"Notable gear" heuristic**: final-death circulation excludes exactly
   `coins` and `bronze-dagger` (networking.cpp:2133-2137, comment cites the
   chronicles scenario contract). The significant-item definition is an
   owner ruling pending.
4. **Relic recovery cadence**: core uses a fixed 1-in-4 per reward stream
   (`kRelicResurfaceOneIn`, core.cpp:29; rolls at :710, :725); wire side
   surfaces one heirloom per elite kill (networking.cpp:2083-2108). OD-006
   owns tuning; numbers cited, not judged.
5. **Legends do not yet influence loot/spawn pools** (OD-010); legends are
   recorded only (core.cpp:267-293, 717-718, 732-733, 813-819).
6. **House crafting layer** beyond the town brand service
   (player:vesselforge:add-brand, networking.cpp:1955-1974, cost 100 coins)
   and purchase paths (shop :2343-2365, wagon :2321-2333) is absent —
   economy pass pending (constitution :62-66).
7. **Denylist firewall status**: live uses of `bronze-dagger`
   (core.cpp:2600; networking.cpp:2137) and the `legacyRelicId` wire key
   (networking.cpp:415-421) are covered by the documented exceptions in
   config/legacy-denylist.json ("bronze-dagger exception (2026-08-20)",
   "legacyRelicId exception (2026-08-20)"), both flagged for owner review.
   No other denylisted identifier appears in new native production item
   code paths scanned.

## C. Red risks (frozen invariants)

| Invariant | Status | Evidence |
|---|---|---|
| D-106 recoverability | GREEN in-process, RED across restart (GAP-2) | networking.cpp:748-750, 2133-2143; core.cpp:803-823 |
| Single House ownership of relics | GREEN (pools move exclusively; erase-before-push; dedup on re-entry/snapshot) | core.cpp:710-712, 558-595, 1200-1223; tests core_tests.cpp:681-701, 821-852 |
| Significant-item history | GREEN in core (durable ordered history lines), RED on wire (GAP-1 event/history split, GAP-7 no wire projection) | core.cpp:426-534, 803-815; networking.cpp:354-390 |
| Denylist firewall | GREEN with two documented exceptions under owner review | config/legacy-denylist.json; networking.cpp:415-421, 2137 |
| Successor starts empty (constitution :60, D-004) | GREEN both systems | core.cpp:871-883; core_tests.cpp:1102-1105, 1161-1163 |

Boundary note (not a violation): uncollected FLOOR value at death/retirement
is abandoned by design ("floor value belongs to the instance",
core.cpp:558-569) while ALL CARRIED value survives — pinned by
test_death_retires_floor_without_double_registering_relics
(core_tests.cpp:1003-1022) and test_instance_lifecycle_rejects_stale_pickups
(:953-1001).

## D. Negative control (required): uncovered history/recovery transition

**Transition**: item history across UNEQUIP (GAP-1).
`resolve_unequip` (native/src/core.cpp:522-534) emits
`EventType::ItemHistoryUpdated` ("unequip") without appending any
`item.history` line, and no automated test exercises history content around
unequip — `test_item_identity_and_branch` (core_tests.cpp:1212-1237) asserts
only the cleared flag/actor reference (:1228-1233), and the persistence
round-trip tests never carry an unequipped-after-equipped history delta.
Today the suite would stay green even if the history vector were truncated
on unequip, or if the event were deleted. This is the audit's designated
negative control; also recorded in captures/item-lifecycle.json
(`negative_control`). Secondary uncovered recovery transitions (not chosen
as the control): restart survival of `circulation_pool()` (GAP-2) and the
crypt `relic.status lost->recovered` flip via `mark_relic_recovered`
(networking.cpp:1602-1631) — neither has a native automated test.

## E. Smallest proposed locking tests (proposal only — not implemented)

1. **Unequip history lock (locks GAP-1)** — in `core_tests.cpp`: pickup,
   equip, unequip, then assert EITHER `carried_items.front().history.back()`
   equals an unequip-class line OR no `ItemHistoryUpdated("unequip")` event
   was emitted. Red today; forces the owner to pick one contract.
2. **Circulation restart-survival lock (locks GAP-2)** — once the pool has a
   persistence seam: commit two heirlooms via final death, snapshot/restore
   (or server restart equivalent), assert both still surface oldest-first.
   Until the seam exists, the minimal honest variant documents current
   behavior: a fresh session must NOT see a prior session's unrecovered
   relic (pins the volatility instead of hiding it).
3. **Store-convergence lock (locks GAP-4)** — after `player:extract`, request
   the bank screen and assert the extracted uuid appears in `payload.items`.
   Red today.
4. **Identity-stability lock (locks GAP-3)** — two `WorldSimulation`s with
   equal seeds mint equal uuid sequences (or the owner accepts process-scope
   uniqueness explicitly and the test pins the documented contract).
