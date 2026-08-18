# TASK-0047 — Parity wave N4 report

## Executive summary

Implemented the N4 item/inventory/Vesselforge slice over the native C++ server
against the unchanged playtest harness. The native server now owns seeded
Vesselforge item generation and searing, the curated item catalogue exercised
by the scenarios, grid inventory with overflow, the 11-seat wear set with
ring→ring2 spillover, ground items with underfoot/menu Take, the 100-coin
Delaford brand service, monster loot, depth floor chaining with guaranteed
treasure, and the N4 `combat:hit` parity fields (base/beastbane/critical/
attackStyle).

Attach bar: **13/13 PASS** (6 item-family scenarios + the 7 N1–N3 regression
scenarios, unchanged harness). All four native gates PASS. One authentic
negative was produced and reverted. This is **REVIEW_REQUESTED / not
architect-accepted**. No acceptance is claimed here.

## Approach and reference parity

Rules live in `WorldSimulation`/`VesselForge`; protocol translation lives in
`ProtocolSession`, matching the N1–N3 layering. The implementation was shaped
against the read-only browser references:

- `server/core/vesselforge/engine.js` — mulberry32 stream, generation order,
  brand pools/tiers, sear clone semantics, tooltip sections, epithet naming.
- `server/core/vesselforge/packs/verdigris-1.js` — forms, materials, brand
  mods, name fragments.
- `server/core/data/items/{general,jewelry,belts,weapons,verdigris,vessels}.js`
  — catalogue rows and equip slots.
- `server/core/inventory/*` — grid admission, overflow, wear seats.
- `server/core/loot.js` — monster coin/gear drops and floor treasure.
- `server/core/registry.js` + `server/core/actions/*` — context-menu build and
  action dispatch envelopes.

Deterministic ground truths were pinned as C++ unit tests from the JS engine:
vessel-ring seed 4/ilvl 40 (Bone Ring, wealthy T1 10 + strongback T2 10),
vessel-khopesh seed 1670/ilvl 40 (Flint Khopesh, beastbane T1 13 + keen_eye
T2 22), bronze-pike seed 1/ilvl 20 ("Copper Whisper", heavy T1 3 + keen T2 16
+ keen_eye T1 8), plus the mulberry32 seed-42 reference stream.

## Changed files

Implementation (commits `16c159f9` + `ba0d9307`, branch
`codex/TASK-0047-native-n4-kimiwork`, merged program tip `8ea0887c`):

- `native/include/verdigris/core.hpp` — Mulberry32, VesselItem/Brand/Block,
  VesselForge, ItemDef/catalogue, GameItem, PlayerInventory, WearSet,
  GroundItem, PlayerCombatMods, WorldSimulation N4 hooks.
- `native/include/verdigris/networking.hpp` — session inventory/wear/RNG
  ownership.
- `native/src/core.cpp` — forge engine, pack tables, catalogue, loot,
  floor chaining, treasure scatter, combat hit pipeline.
- `native/src/networking.cpp` — dev:give/dev:drop/dev:forcecritical,
  item:equip, player:take:underfoot, context-menu build/action dispatch,
  player:inventory:commit world-drop, snapshot extensions.
- `native/tests/core_tests.cpp` — 7 N4 tests incl. the 3 JS ground truths.
- `native/tests/networking_tests.cpp` — the pre-existing dev-give assertion
  now checks the granted `garnet-amulet` by id (previously passed vacuously
  on exact-size-1 while the grant silently failed as an unknown id).

Evidence captures added with this report:

- `captures/build-runtests.log`
- `captures/attach-13.log`
- `captures/negative-vesselforge-brand.log`
- `captures/positive-vesselforge-brand.log`

## Public interfaces and wire behavior

- `dev:state` snapshot carries `level`, `inventory`, `inventoryDetails`,
  `wear`, `wearDetails`, `combat`, and `groundItems`; monsters carry `tags`
  and `coins`.
- `combat:hit` carries `baseAmount`, `beastbaneAmount`, `beastbanePercent`,
  `beastbane`, `critical`, `attackStyle` alongside the N3 fields.
- `player:context-menu:build` answers world Take (newest first) / Walk here /
  Cancel, and the inventory Add-brand entry (town, vessel item, free brand
  slot, patience ≥ 1).
- `player:context-menu:action` dispatches on `queueItem.action.actionId`
  (`player:take`, `player:vesselforge:add-brand`).
- `player:inventory:commit` `world-drop` places the item at the player tile.
- Movement/teleport emits `party:scene:transition` on depth or scene change.

## Verification

1. `powershell -NoProfile -File native/build.ps1 -RunTests` — denylist, core
   tests (incl. 7 new N4 tests), networking tests, camera2d tests all PASS.
   Literal transcript: `captures/build-runtests.log`.
2. Rebuilt native server (`native/build/verdigris_server.exe 6512`, loopback),
   unchanged harness at merged tip `8ea0887c` — **13/13 PASS**: loot,
   equipment-slots, depth-loot, overflow, vesselforge, vesselforge-brand,
   quickstart, single-session, movement, zones, combat, encounter-variety,
   boss-mechanic. Literal transcript: `captures/attach-13.log`.
   `git diff 8ea0887c -- playtest/ server/ src/` is empty; no harness or
   read-only reference files were changed.
3. Authentic negative: the brand-service cost was temporarily changed from
   100 to 99 coins; `vesselforge-brand` failed on "adding a brand spends
   exactly 100 coins" (`captures/negative-vesselforge-brand.log`). The
   constant was restored, the server rebuilt, and the scenario passed again
   (`captures/positive-vesselforge-brand.log`).

Two real defects were found and fixed during attach bring-up:

- `garnet-amulet` was missing from the catalogue, breaking the
  `single-session` regression grant (added from
  `server/core/data/items/jewelry.js`).
- The add-brand handler held an inventory item pointer across
  `spend_coins`, which rebuilds the items vector; the seared brand was
  written through a dangling pointer and lost. The handler now re-resolves
  the pointer after spending. (Parity gotcha also worth noting: epithet
  draws must be two separate statements — MSVC's unspecified `operator+`
  operand order initially swapped the pre/post fragment draws.)

## Stubs, deviations, and follow-ups

- Floor treasure scatters from the flat `GEAR_DROP_POOL` (loot.js order), not
  the per-depth `gearPoolForDepth` tiers; the depth-loot scenario contract
  (guaranteed ilvl-raising treasure per floor) is met.
- Underfoot take resolves directly at chebyshev ≤ 1; there is no
  walk-and-take queueing.
- Monster coin bounty uses the authored stub formula `10 + level*5`
  (elite ×3); tagged beast/boss coin parity is a later wave.
- Swap onto a full grid spills the displaced piece bound at the feet instead
  of the JS abort; documented simplification, unreachable by the scenario
  set.
- Relic/trophy/first-find circulation and bond/awakening sections are N5+;
  no flow produces them, so their tooltip sections never render.
- No starter blade is granted (D-106 denylist: no item-injection path beyond
  the scenario-driven dev verbs).
- Unseeded `dev:give` draws from a per-session mulberry32 stream instead of
  `Math.random`; seeded draws reseed the forge per instance as in JS.
- The `bindOnPickup` catalogue flag is unmodelled; binding happens on
  admission (`boundTo`) per the overflow scenario contract.
- The weapon-size `idContains` token lists drop the `pickaxe`/`hammer` tokens
  and the bar/ore/coins branch (denylisted, unreachable in N4 flows).
- Ground items on floor transition retire with the scene like the JS scenes;
  town ground items stash/restore across instance dives.

## Risks and architect handoff

The architect should rebuild the branch and personally rerun the attach set
per the task spec, then decide ACCEPTED/REVISE. In particular, review the
dangling-pointer fix in the brand service, the garnet-amulet catalogue
addition (regression dependency), and the documented treasure/coin stubs
before acceptance.
