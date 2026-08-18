# TASK-0047 N4 — survey notes (kimi-work, working file)

Base: 05c3f46 (contains N3 INTEGRATED). Branch: codex/TASK-0047-native-n4-kimiwork.
Harness is READ-ONLY. Attach set: loot, equipment-slots, depth-loot, overflow,
vesselforge, vesselforge-brand + regression quickstart, single-session, movement,
zones, combat, encounter-variety, boss-mechanic = 13/13.

## Harness wire surface (playtest/harness.mjs + 6 scenarios)

Inbound events the C++ server must handle:
- `dev:give {itemId, qty, seed?, itemLevel?}` — real pipeline, overflow 'drop'
  at feet bound to player; seed → deterministic vessel roll.
- `dev:drop {itemId, seed?, itemLevel?}` — place on floor at player x,y.
- `dev:state {requestId}` → `dev:state {state, requestId}` snapshot (shape below).
- `dev:teleport {x,y}` — floor onto tile, portal check (stairs => transition).
- `dev:setlevel {level}`, `dev:heal {}`, `dev:forcecritical {}` (exist N3).
- `item:equip {item:{id, uuid, slot, targetSlot?, miscData:{slot,targetSlot}}}`.
- `player:take:underfoot {}` — take ONE reachable ground item (same tile or
  manhattan-1). Any one of the reachable set may leave the floor.
- `player:context-menu:build {miscData, tile, player:{socket_id}}` →
  `game:context-menu:items {data:[entries]}`.
  * world tile variant: miscData.clickedOn {0:'main-canvas',1:'gameMap'},
    tile.world {x,y} — must include Take entries per ground item on tile:
    entry {uuid, id, label starts 'Take' (case-insens, tags stripped),
    action:{name:'take',...}} (plus other entries; >1 total ok).
  * inventory variant: clickedOn {0:'inventory-item',1:'inventorySlot'},
    miscData.slot = item.slot — vessel items must include entry with
    `action.actionId === 'player:vesselforge:add-brand'` and label matching
    /100 coins/i.
- `player:context-menu:action {data:{item, tile}, queueItem:{item:{uuid,id},
  tile, action, at, coordinates, queueable, world}, player:{socket_id}}` —
  executes Take (ground->inventory) and add-brand (100 coins, +1 brand).
- `player:inventory:commit {id, player:{socket_id}, action:'world-drop',
  item:{id,uuid,slot}}` — remove from backpack, drop at feet (production drop).

## dev:state snapshot fields the scenarios read (server/player/handlers/dev.js)

- uuid, x, y, level, socket? (p.player.socket_id from login block), lifecycle
  ('alive'), inventory[]: {id,uuid,qty,slot,size,itemLevel,stats,vessel}
- inventoryDetails[]: itemIdentity = {id,uuid,name,displayName,qty,slot,
  position,orientation,boundTo,affixes,vessel,stats,attributes,
  resourceBonuses,combatBonuses,size}
- groundItems[]: {id,uuid,name,displayName,boundTo,x,y,qty,itemLevel,stats,vessel}
- wear: {slot: item.id|null}; wearDetails: {slot: itemIdentity}
- combat: {attack,defense,blockChance,criticalChance,goodsFound,
  damageAgainstBeasts,respawnProtectionUntil}
- monsters[]: {uuid,name,x,y,level,rarity,tags[],modifiers,behaviour,hp,coins}
- sceneMetadata: {depth, layout, theme, stairsUp, stairsDown:{x,y}, spawnPoints}
- itemLevel(item) = item.vessel.item.ilvl ?? null

## Vesselforge deterministic generation (MIRROR EXACTLY)

RNG: mulberry32 (both dev.js seededRng and engine use the same algorithm):
  state = (state + 0x6D2B79F5) >>> 0
  v = state; v = imul(v ^ (v>>>15), v|1); v = v ^ (v + imul(v ^ (v>>>7), v|61))
  out = ((v ^ (v>>>14)) >>> 0) / 4294967296
dev:give seed S: rng = mulberry32(S); createVesselBlock does
  forge.reseed(floor(rng() * 2**32))  (consumes ONE rng call)
then forge rand = mulberry32(reseeded).

generateItem({ilvl, formId, materialId?}) consumes rand in this order:
 1. genId(): 1 rand (floor(rnd*1e6)) [idCounter prefix irrelevant]
 2. (formId given => no form pick; materialId absent => materialPoolFor +
    pickWeighted: 1 rand; pool = formDef.materials filtered tier <=
    1+floor(ilvl/15), weight = dropWeight||10, IN PACK ORDER of formDef.materials)
 3. vessel = rint(mat.vessel[0], mat.vessel[1]) : 1 rand
 4. patienceMax = rint(mat.patience[0], mat.patience[1]) : 1 rand
 5. brand count n = pickWeighted(brandCountWeights [30,25,25,15,5] => 0..4): 1 rand
    n = min(n, vessel)
 6. per brand (rollBrand):
    a. pickWeighted(brandPool): 1 rand. Pool = brandMods (INSERTION ORDER:
       keen, heavy, swift_haft, bloodgroove, long_reach, warded, hale, spirited,
       emberkiss, riverblessed, emberward, surefoot, keen_eye, wealthy,
       beastbane, strongback) filtered kinds includes item.kind && modId unused.
       weight w = mod.weight||10, per tag in mod.tags: w *= mat.weights[tag]
       (if present), w *= formDef.weights[tag] (if present). Filter w>0.
       pickWeighted: roll = rnd*total; iterate subtracting w, first roll<=0 wins;
       fallback last entry.
    b. tier: gated = tiers with (minIlvl||0) <= ilvl; pickWeighted over gated
       with weights brandTierWeights [10,5,2] indexed by TIER INDEX: 1 rand.
    c. genId(): 1 rand.
    d. value = rint(tierDef.roll[0], roll[1]): 1 rand.
 7. maybeName: if brands+bonds+trophies >= 3: pick(nameTables.pre) + pick(post):
    2 rands (pre list 10 entries, post 8).

ilvl = min(80, floor(itemLevel)) (default 10).
Expected rolls (acceptance): vessel-ring seed 4 ilvl 40 -> wealthy brand value
10 ONLY combat modifier (goodsFound 10); vessel-khopesh seed 1670 ilvl 40 ->
keen_eye value 22 + beastbane value 13.

aggregate(): sums[modId] += value for flat/scalar brands + implicit stat.
deriveVesselCombat: modifiers = {blockChance: min75(sums.block),
 criticalChance: min75(sums.keen_eye), goodsFound: min100(sums.wealthy),
 damageAgainstBeasts: min100(sums.beastbane)} — combatBonuses on itemIdentity.
Weapon rating: min/max from dmg*statMult+flat, *physMult, dps, rating=max(1,
 round(dps/2)) into channel (khopesh=slash, handaxe=slash, spear=stab,
 macuahuitl=crush, atlatl/sling=range). ward rating = max(1,round(ward/8))
 all defense channels. (equip pipeline needs attack rating.)

tooltip lines: sections name/kind/vessel/stat/implicit/brand('✦ {label} (Tn)')
… — brand lines kept 'brand' section only when modId in ACTIVE_BRAND_MODS
(keen, heavy, swift_haft, warded, hale, spirited, emberkiss, strongback,
keen_eye, wealthy, beastbane) else dormant. Scenario asserts vessel.lines has
section 'brand' matching /Critical Chance/, /Item Find/, /Damage against
Beasts/. Labels: '+{v}% Critical Chance', '+{v}% Item Find',
'+{v}% Damage against Beasts'.

## Pack data needed (verdigris-pack.js)

materials: flint t1 sm1.0 v[2,3] p[2,3] dw30 w{blade1.4,blood1.2,ward0.6};
 bone t1 sm0.9 v[2,4] p[2,4] dw25 w{beast1.6,spirit1.3,ember0.5};
 hide t1 sm0.9 v[2,3] p[3,4] dw30 w{beast1.3,swift1.3,ward0.8};
 quilted t2 sm1.2 v[2,4] p[3,5] dw16 w{ward1.3,life1.3,blade0.6};
 copper t2 sm1.25 v[2,4] p[3,5] dw18 w{river1.3,fortune1.3};
 bronze t3 sm1.6 v[3,5] p[4,6] dw9 w{blade1.2,ward1.2,blunt1.2};
 obsidian t3 sm1.55 v[3,5] p[2,4] dw8 w{blade1.8,blood1.8,ward0.4};
 jade t4 sm1.7 v[4,5] p[4,6] dw4 w{spirit1.8,ward1.5,blood0.5};
 amber t4 sm1.65 v[4,5] p[4,6] dw4 w{ember1.7,fortune1.5,spirit1.3};
 bronzescale t4 sm1.9 v[4,5] p[4,6] dw3 w{ward1.6,life1.3};
 skymetal t5 sm2.3 v[4,6] p[5,7] dw1 w{blade1.4,spirit1.4,ember1.3};
 rivetmail t6 sm2.6 v[5,6] p[5,8] dw0.3 w{ward2.0,life1.5}
forms (id, kind, w,h, dmg/aps or armor, tags, weights, materials, implicit):
 handaxe weapon 1x2 dmg[7,13] aps1.3 tags[blade] w{blade1.3}
   mats[flint,copper,bronze,obsidian,skymetal] impl phys_pct15
 spear weapon 1x4 [10,22] 1.0 [reach] {reach1.5}
   [flint,bone,copper,bronze,obsidian,skymetal] impl none(stat null)
 macuahuitl weapon 2x3 [14,30] 0.85 [blade,blunt,blood] {blood1.6}
   [obsidian,flint,bone] impl bleed(stat null)
 atlatl weapon 1x3 [8,18] 1.1 [reach,swift] {swift1.4} [bone,copper,bronze]
 khopesh weapon 1x3 [11,20] 1.2 [blade] {blade1.2,fortune1.2}
   [flint,copper,bronze,skymetal] impl atk_speed10
 sling weapon 1x2 [5,16] 1.15 [reach,swift] {swift1.3} [hide,quilted]
 hideshield shield 2x3 armor45 [ward] {ward1.5} [hide,bronze,bronzescale,rivetmail]
   impl block4
 wrap body 2x3 armor60 [ward,life] {life1.2} [hide,quilted,bronzescale,rivetmail]
   impl life15
 crest helmet 2x2 armor25 [ward,spirit] {spirit1.2} [bone,hide,copper,bronze,jade]
   impl spirit10
 grips gloves 2x2 armor18 [blade,swift] {} [hide,quilted,bronzescale] impl atk_speed8
 sandals boots 2x2 armor14 [swift] {swift1.6} [hide,quilted] impl move10
 girdle belt 2x1 armor0 [life] {life1.3} [hide,quilted,copper] impl life12
 gorget amulet 1x1 armor0 [spirit,ward] {spirit1.4} [jade,amber,bone,copper] impl attrs8
 ring ring 1x1 armor0 [fortune] {fortune1.2} [bone,copper,jade,amber] impl life12
brandMods (id, label, shape, tags, kinds, weight, tiers[roll,minIlvl?]):
 keen '+{v}% increased Physical Damage' scalar [blade] [weapon] 12
   [8-14],[15-22@20],[23-32@50]
 heavy '+{v} flat Damage' flat [blunt,blade] [weapon] 12 [2-4],[5-8@25],[9-14@55]
 swift_haft '+{v}% Attack Speed' scalar [swift] [weapon] 9 [5-8],[9-13@25],[14-18@55]
 bloodgroove bleed scalar [blood] [weapon] 7 [10-15],[16-25@30]
 long_reach scalar [reach] [weapon] 7 [6-10],[11-16@30]
 warded '+{v}% increased Armour' scalar [ward] [shield,body,helmet,gloves,boots] 12
   [10-18],[19-28@20],[29-40@50]
 hale '+{v} to Maximum Health' flat [life] [weapon,shield,body,helmet,gloves,boots,
   belt,amulet,ring] 12 [10-20],[21-35@25],[36-55@55]
 spirited '+{v} to Maximum Mana' flat [spirit] [helmet,amulet,ring,weapon] 10
   [8-15],[16-26@25],[27-40@55]
 emberkiss 'Adds {v} Fire Damage' flat [ember] [weapon,amulet] 6 [3-7],[8-14@30],[15-22@60]
 riverblessed '+{v}% to Cold Resistance' scalar [river,ward]
   [shield,body,helmet,belt,amulet,ring] 8 [8-15],[16-25@25]
 emberward '+{v}% to Fire Resistance' scalar [ember,ward]
   [shield,body,helmet,belt,amulet,ring] 8 [8-15],[16-25@25]
 surefoot '+{v}% Movement Speed' scalar [swift] [boots] 9 [5-9],[10-15@30]
 keen_eye '+{v}% Critical Chance' scalar [blade,swift] [weapon,ring] 7
   [8-14],[15-22@35]
 wealthy '+{v}% Item Find' scalar [fortune] [weapon,shield,body,helmet,gloves,
   boots,belt,amulet,ring] 8 [6-12],[13-20@30]
 beastbane '+{v}% Damage against Beasts' scalar [beast] [weapon] 7
   [10-18],[19-30@30]
 strongback '+{v} to All Attributes' flat [life,spirit] [amulet,ring,belt] 6
   [3-6],[7-11@35]
brandTierWeights [10,5,2]; brandCountWeights [30,25,25,15,5]; maxVessel 6.
nameTables pre [Grim,Sable,Ashen,Reed,Ember,Frost,Dusk,Copper,Thorn,Vesper]
 post [Whisper,Ward,Bite,Song,Pledge,Shard,Gloam,Tithe]

## Catalogue vessel items (server/core/data/items/vessels.js)

vessel-<form> ids, slot: handaxe/spear/macuahuitl/atlatl/khopesh/sling
right_hand (spear+macuahuitl twoHanded), shield left_hand, wrap armor, crest
head, grips gloves, sandals feet, gorget necklace, ring ring. vesselforge:
{formId} only (NO materialId — rolled). Legacy items ring/gold-ring/
hide-girdle/bronze-sword/bronze-pike/coins live in
server/core/data/items/{jewelry,belts,weapons,general,verdigris}.js — READ NEXT.

## Still to survey (pointers)

- server/core/items/factory.js: createById (vessel attach, uuid, size from
  form w/h?), toWorldInstance, stack rules (coins stackable qty).
- server/core/utilities/common/player/inventory.js: grid 12x7=84, footprints,
  overflow 'drop' spill-at-feet, boundTo on admission.
- equip path: item:equip handler + server/core/utilities/wear.js — ring seats
  (ring, ring2), belt->wear.belt key name, equip->combat pipeline
  (player.combat.attack/defense/criticalChance/goodsFound/damageAgainstBeasts).
- server/core/combat/loot.js: dropMonsterLoot — coins always drop;
  Wealthy qty = floor(coins*(1+goodsFound/100)); guaranteed floor treasure
  (id != coins, itemLevel, vessel.item.ilvl) — depth-loot: floor5 ilvl >=
  floor1 + 30; find ilvl formula (likely depth-scaled).
- server/core/combat/index.js: combat:hit payload {targetId, amount,
  baseAmount, critical, beastbane, beastbaneAmount}; crit =
  max(base+1, round(base*1.5)); beastbaneAmount = round(base*1.13) when
  target tags include 'beast' and player has damageAgainstBeasts.
- server/player/handlers/party.js transitionFloor + '· Floor N' scene naming;
  stairsDown semantics in N2 WorldSimulation (already has stairs_down +
  check_stair_transition — but only depth 1 / return; N4 needs depth+1 chain).
- context-menu strategies: registry.js (Take entry build for ground items),
  vesselforge-brand.js (add-brand entry + action handler, cost 100 coins,
  brand roll via forge.sear equivalent? check), actions/index.js handler for
  'player:vesselforge:add-brand'.
- N3 networking.cpp current handlers to extend: dev:give exists? (saw string)
  check what it does today; dev:state current shape (needs inventory/wear/
  combat/groundItems extensions).
EOF marker — append below as survey continues.

## GROUND TRUTH (from JS engine, captures/forge-truth.mjs)

- vessel-ring seed4 ilvl40: Bone Ring, brands [wealthy T1 v10, strongback T2
  v10], modifiers {goodsFound:10}, epithet none, vessel 4, patience 2.
- vessel-khopesh seed1670 ilvl40: Flint Khopesh, [beastbane T1 v13, keen_eye
  T2 v22], modifiers {criticalChance:22, damageAgainstBeasts:13}, slash
  rating 10, vessel 2, patience 3.
- bronze-pike seed1 ilvl20: "Copper Whisper" (3 brands => epithet), Bronze
  Spear, [heavy T1 v3, keen T2 v16, keen_eye T1 v8], stab rating 17,
  modifiers {criticalChance:8}, vessel 4, patience 4.

## IMPLEMENTATION PLAN (approved by self-review)

core.hpp/cpp N4 section: Mulberry32; pack tables (materials/forms/brandMods,
insertion order); VesselForge (reseed/generate_item/sear/tooltip(honest
dormant marking)/derive_combat/make_block); ItemDef catalogue (coins, ring,
gold-ring, hide-girdle, bronze-sword, bronze-dagger, bronze-pike +
13 vessel-* rows); resolve_item_size port; GameItem instance; PlayerInventory
(12x7 first-fit, coins merge, overflow spill, spend_coins); WearSet (ring
seats, swap, totals with 75/75/100/100 caps); WorldSimulation += GroundItem
list (town stash across instance hops; floor lists retire like JS scenes),
monster tags+coins, transition_floor(depth±1) with '· Floor N' naming,
guaranteed floor treasure (coins + GEAR_DROP_POOL item at
min(80,10+(depth-1)*10) spiral-placed on open tiles), drop_loot (coins
floor(c*(1+goods/100)) + rarity gear chance*goods cap .75, ilvl
min(80,level*2), spiral safe tile), take/underfoot rules; combat mods struct
into advance_combat (crit max(bb+1,round(bb*1.5)), beastbane round(base*1.13)
on 'beast' tag, force_critical consumed).
networking.cpp: replace legacy inventory_ with PlayerInventory/WearSet/
VesselForge on ProtocolSession; handlers dev:give (seed->mulberry->reseed),
dev:drop, item:equip (seat resolve+swap), player:take:underfoot,
player:context-menu:build (world Take newest-first + Walk here + Cancel;
inventory add-brand in town when vessel+patience+free slot), 
player:context-menu:action (take by uuid within manhattan<=1; add-brand sear
-100 coins), player:inventory:commit world-drop, dev:forcecritical; snapshot
full N4 shape; combat:hit extended (baseAmount, beastbaneAmount,
beastbanePercent, beastbane, critical, attackStyle); core:refresh:inventory
emits; game:context-menu:items {data:[...]}.
Unit tests: mulberry32 sequence vs JS; 3 ground-truth rolls exact; brandPool
exclusion of used mods; inventory first-fit+overflow counts; coins merge full
backpack; ring seats ring/ring2/swap; wear caps; take/underfoot bounds; depth
transition naming; treasure ilvl formula; loot Wealthy floor math.
Stubs to document: treasure uses GEAR_DROP_POOL (not gearPoolForDepth tiers),
no walk-and-take queueing (manhattan<=1 direct), monster coin formula authored
(10+level*5 elite x3), floor ground items retire like JS scenes, swap-failure
spills bound at feet, no relic/quest circulation in drops (N5).
