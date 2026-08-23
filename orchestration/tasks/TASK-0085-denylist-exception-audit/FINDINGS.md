# TASK-0085 FINDINGS — Live denylist-exception evidence packet

- lane: ox-pc-bb · model: openrouter/stealth/ox-alpha
- evidence base: live worktree at HEAD `224a0b7c` (branch
  `worker/verdigris/pc/ox-pc-bb`); SPEC `base_commit d2423873` verified ancestor.
- scope discipline: EVIDENCE ONLY. This packet lists occurrences, contracts,
  and per-token disposition consequences. It does NOT choose a disposition,
  propose a canonical replacement name, set a compatibility window, or make a
  lore/item decision (SPEC stop conditions; OI-001 reserves those to the owner).

## 0. Governing context

- **D-116 (owner-ruled, `orchestration/DECISIONS.md:82-95`)**: the C++ native
  server speaks the EXISTING `{event,data}` WebSocket protocol so the current
  client connects unchanged; the existing playtest scenario suite is the PARITY
  BAR and regression suite ("unchanged-harness law"). Any token a scenario
  asserts is frozen for both servers until the harness changes.
- **OI-001 (`orchestration/owner-input/OI-001-denylist-dispositions.md`)**:
  decision WAITING_EVIDENCE on this task. Packet recommendation on record
  (preserve `legacyRelicId`, migrate `bronze-dagger`) is explicitly "not an
  owner ruling" (`orchestration/tasks/TASK-0121-owner-content-approval-matrix/captures/owner-gates.json:145`).
  Acceptance rubric there: no save loss, no unchanged-harness regression, no
  denied starter kit restored to canon, every surviving exception wire/data-only
  and documented.
- **Constitution tension (`docs/product/VERDIGRIS_CONSTITUTION.md`)**: generic
  starter kits are out of product identity (:16) and the Delaford firewall
  denies "bronze dagger starters [and] generic starting coins … by default"
  (:177-179), requiring intentional carry-over to be named in the legacy
  allowlist. Both exceptions under review are exactly such carry-over.
- **Denylist state at evidence time (`config/legacy-denylist.json:32-33`)**: the
  two tokens are documented as category-note exceptions dated 2026-08-20,
  flagged for owner review.

## 1. Exception A — wire key `legacyRelicId`

### 1.1 What it is

A JSON property stamped onto circulating relic ground items by
`drawCirculatingRelic` (`server/core/services/chronicles.js:312`):
`item.legacyRelicId = record.id`. It carries the House-Chronicle relic record id
so that a later pickup can claim the record via `claimCirculatingRelic`
(`server/core/services/chronicles.js:340-342`). The sibling `legacy` object
(`sourceScionId`, `sourceScionName`, chronicles.js:313-317) travels with it.

### 1.2 Live occurrence table (every occurrence from acceptance grep #1)

| # | Location | Role | Consumer contract |
|---|----------|------|-------------------|
| A1 | `server/core/services/chronicles.js:312` | WRITE: stamps key on drawn relic item | source of truth |
| A2 | `server/core/services/chronicles.js:341` | READ: claim guard (`if (!item?.legacyRelicId) return false`) | claim round-trip |
| A3 | `server/core/services/chronicles.js:342` | READ: `chroniclesRepository.claimRelic(item.legacyRelicId, player)` | marks relic claimed in House chronicle |
| A4 | `server/player/handlers/dev.js:202` | WIRE: dev/state scene snapshot includes `legacyRelicId: item.legacyRelicId \|\| null` in groundItems | harness `state().groundItems`; native mirrors this shape (A8) |
| A5 | `playtest/scenarios/chronicles.mjs:55` | HARNESS ASSERTION: identifies the ancestral ring by `item.id === 'gold-ring' && item.legacyRelicId` | **the D-116 parity bar**; also reads `ring.legacy.sourceScionName` at :60 |
| A6 | `tests/unit/instance-loot.spec.js:290,298` | UNIT TEST: fixture + assertion that a dead scion's relic enters the live loot stream carrying `legacyRelicId: 'relic-1'` | pins loot-stream passthrough (`server/core/combat/loot.js:305` inserts the stamped item unmodified into scene items) |
| A7 | `native/tools/check_legacy_denylist.py:221` | GATE SELF-TEST: variant `("legacyRelicId", "legacy relic id")` expected to be DENIED — see §3 gate findings | self-test contract, currently unsatisfiable |
| A8 | `native/src/networking.cpp:482` | NATIVE PARITY EMIT: `put(out, "legacyRelicId", ground.relic_record_id)` inside `ground_item_json` (+ `legacy` object :483-486) | D-116 requires native to emit the same key; feeds `dropped_items_json()` → login payload (`networking.cpp:666-671`) and ground-change events; backed by `GroundItem::relic_record_id` fields (`native/include/verdigris/core.hpp:705-715`) |

Wire propagation beyond A4/A8 (same key, implicit): after
`drawCirculatingRelic` stamps the item, `dropMonsterLoot` pushes it raw into
`scene.items` and pickup broadcasts sceneItems verbatim
(`server/core/items/pickup.js:47`); after pickup the same object becomes an
inventory slot and `refreshInventory` sends `player.inventory.slots` verbatim
(`pickup.js:16-22`). Nothing strips the key anywhere, so it rides multiple
event paths.

### 1.3 Visibility

**Wire-only.** Zero hits for `legacyRelicId` in `src/` (browser client) or any
renderer/component code; the browser client never reads the key. What players
see is derived data: the item display name `"… — Relic of <scion>"`
(chronicles.js:318-320) and the `legacy.sourceScionName` provenance string.
The key itself exists so the server can route the claim and so the harness can
recognize a circulating relic.

### 1.4 Disposition evidence (no choice made)

1. **Keep documented exception** — zero breakage today. Cost: the canonical term
   "legacy relic id" is absent from `identifiers`, so NO file under `native/`
   is policed for ANY spelling of this identifier anymore (not just networking.cpp),
   and the checker self-test stays red (§3). The scoped allowlist mechanism that
   would confine the exception to specific files exists and is unused
   (`check_legacy_denylist.py:103-127`, `allowlist: []`).
2. **Migrate compatibly** — requires a versioned protocol change touching all of:
   JS emit+claim (A1-A3), dev state shape (A4), native parity emit (A8),
   harness assertions (A5, twice), unit fixtures (A6). Under D-116 the harness
   may not change casually — the scenarios ARE the parity bar — so migration is
   a coordinated JS+native+harness wave. Persisted relic records themselves are
   keyed by `record.id` in the chronicle repository; only the wire property
   name would move, but old clients/harness break unless dual-read.
3. **Remove with named breakage** — removing the key without replacement breaks:
   - playtest `chronicles` scenario at 'ancestral ring drop'
     (chronicles.mjs:53-58 never resolves → timeout failure), i.e. a red parity
     bar for BOTH servers;
   - native/JS dual-run divergence the moment either side stops emitting;
   - the relic claim round-trip: `claimCirculatingRelic` returns false, so a
     picked-up heirloom never marks its record claimed (duplicate circulation /
     lost provenance);
   - unit spec `instance-loot.spec.js:276-301`.

## 2. Exception B — item id `bronze-dagger`

### 2.1 What it is

The hyphenated catalogue id of the tier-1 starter weapon, canonically defined in
`server/core/data/items/weapons.js:93-120` (player-visible name **"Bronze
Dagger"**, examine text, price **9**, stab/slash stats). It is granted once per
fresh Scion with 100 coins (`server/core/entities/player/fresh-scion-profile.js:13-19`)
and duplicated in the player template (`server/core/data/helpers/player.json:41`).

### 2.2 Live occurrence table (every occurrence from acceptance grep #2)

Server / data / client:

| # | Location | Role |
|---|----------|------|
| B1 | `server/core/data/items/weapons.js:93` | CANONICAL DEFINITION: id, name "Bronze Dagger", examine, price 9, stats, actions |
| B2 | `server/core/entities/player/fresh-scion-profile.js:14` | STARTER KIT GRANT: `ItemFactory.createById('bronze-dagger', { bindTo: ownerId, includeAffixes: false })` |
| B3 | `server/core/data/helpers/player.json:41` | DATA TEMPLATE: starter inventory entry |
| B4 | `server/core/services/wagon-service.js:27` | HOUSE WAGON tier-1 "Road kit" stock list; stock price derived from B1 (`wagon-service.js:150-154` → 9 gold) |
| B5 | `src/core/inventory/item-art.js:22` | BROWSER REFERENCE art bridge `'bronze-dagger': 'dagger_bronze'` (client presentation seam) |

Native (C++ reconstitution):

| # | Location | Role |
|---|----------|------|
| B6 | `native/src/core.cpp:2668` | NATIVE ITEM TABLE: `{"bronze-dagger", "Bronze Dagger", "weapon", "right_hand", …}` — comment cites town-amenities starter kit |
| B7 | `native/src/networking.cpp:1392` | wagon tier-1 stock display list |
| B8 | `native/src/networking.cpp:2395` | hardcoded wagon buy price: `bronze-dagger ? 10` (**divergence note**: JS derives 9 from weapons.js:96 via wagon-service.js:154; native hardcodes 10. Not pinned by any harness assertion — recorded as evidence only) |
| B9 | `native/src/networking.cpp:2626-2627` | once-per-scion starter grant: detect dagger by id, else `create_game_item("bronze-dagger")` |
| B10 | `native/src/networking.cpp:2209` | final-death relic circulation skip-list: starter `bronze-dagger` (with coins) excluded as non-notable ("only earned gear circulates") |

Harness:

| # | Location | Role |
|---|----------|------|
| B11 | `playtest/scenarios/town-amenities.mjs:17-19` | HARNESS ASSERTIONS: starter inventory MUST contain `id === 'bronze-dagger'` with authored 1x2 footprint. Same scenario simultaneously asserts ABSENCE of denied `hammer`/`bronze-bar`/pickaxe (:14-16) — the scenario encodes the Delaford firewall while excepting the dagger |

Unit tests (fixture/assertion role each):

| # | Location | Role |
|---|----------|------|
| B12 | `tests/unit/fresh-scion-profile.spec.js:19,60` | fresh profile = `['bronze-dagger','coins']`; resume path preserves it |
| B13 | `tests/unit/chronicles-login.spec.js:276,422` | admitted scion and mortality successor both start `['bronze-dagger','coins']` |
| B14 | `tests/unit/stale-player-snapshot.spec.js:15` | template-driven Player keeps `['bronze-dagger','coins']` baseline (stale-save tolerance law) |
| B15 | `tests/unit/equipment-replacement.spec.js:155,184,234` | equip/unequip replacement flows use bronze-dagger (1x2) as the equipped-item fixture |
| B16 | `tests/unit/inventory-system.spec.js:68` | size contract: bronze-dagger resolves 1x2 |
| B17 | `tests/unit/inventory-store.spec.js:155` | inventory store add/positioning fixture |
| B18 | `tests/unit/inventory-item-presentation.spec.js:33-34` | art mapping: id → `dagger_bronze` name/url |
| B19 | `tests/unit/loot-first-find.spec.js:180` | first-find comparison fixture (local const named `bronzeDagger` — camelCase local variable, not the id string) |

Gate:

| # | Location | Role |
|---|----------|------|
| B20 | `native/tools/check_legacy_denylist.py:212` | self-test variant `("bronze-dagger", "bronze dagger")` expected DENIED — see §3 |

### 2.3 Visibility

**Both.** The id string `bronze-dagger` is wire/data-only (inventory payloads,
save snapshots, templates, shop stock ids). But the item is fully
player-visible through its canonical definition: inventory/examine/shop show
"Bronze Dagger" (B1/B6), and the starter grant puts it in every new Scion's
hands (B2/B9). This is why OI-001's rubric ("every surviving exception is
wire/data-only") cannot be satisfied by documentation alone for this token —
the *name* is canon, not just the key.

### 2.4 Disposition evidence (no choice made)

1. **Keep documented exception** — zero immediate breakage. Cost: canonical
   content permanently carries constitution-denied Delaford starter vocabulary
   (constitution :16, :177-179); spelling remains unpoliced across all of
   `native/` (§3); divergence risk grows (B8 already shows JS/native price drift).
2. **Migrate compatibly** — a rename touches BOTH servers plus data and history:
   new canonical id (owner-only naming decision), starter grant + template +
   wagon stock + shop pricing + native item table (B1-B10), art map (B5), and 8
   unit-test files (B12-B19) plus harness B11 (harness change = D-116 event).
   Old persisted material contains the old id: saved Scion snapshots, House
   chronicle relic records (relic payloads historically serialize full items
   with `"id"/"baseId": "bronze-dagger"` — e.g. TASK-0048 baseline capture
   `baseline-c3988-wire.json`, ~60 occurrences), so a compatible READ path for
   the old id must survive one migration window (OI-001 rubric: no save loss).
3. **Remove with named breakage** — every new-Scion flow loses its starter
   weapon and these fail concretely:
   - playtest `town-amenities`: 'new scion receives a proper starter dagger'
     and the 1x2 footprint assert (town-amenities.mjs:17-19);
   - native starter grant silently creates nothing (`create_game_item`
     returns null for unknown id → Scion spawns unarmed) while JS still grants;
     dual-run diverges;
   - specs B12-B14 fail immediately (starter contract = `['bronze-dagger','coins']`);
   - wagon tier-1 stock loses its melee option (B4/B7) and B8's price branch dies.

## 3. Cross-cutting gate-mechanics findings (live-verified)

These bear directly on how the owner ruling can be expressed:

1. **Exceptions were implemented by DELETION, not by the scoped allowlist.**
   Initial reconstitution `6dae611b` listed canonical terms `bronze dagger`,
   `bronze_dagger`, AND `legacy relic id` in `identifiers`. Hotfix `7ab99b65`
   removed `legacy relic id` (documented note; struct fields renamed); N6 wave 2
   `f33cc15a` removed `bronze dagger` + `bronze_dagger` (diff verified in git
   history). Result: NO denylist term matches any spelling of either token, so
   the scan exempts them EVERYWHERE under `native/` — not just the files that
   need the exception. The purpose-built per-path/per-identifier allowlist
   (`check_legacy_denylist.py:103-127,235-249`) sits empty (`allowlist: []`).
2. **The checker's own self-test is currently RED.** Live run (evidence-time):
   `python native/tools/check_legacy_denylist.py --self-test` exits 1 at the
   first variant ("self-test: expected denied variant was missed: bronzeDagger");
   the variants table (:209-224) still pins that `bronze-dagger`,
   `legacyRelicId`, etc. resolve to denied terms that no longer exist in
   `identifiers`. The `legacyRelicId` variant has been unsatisfiable since
   `7ab99b65`.
3. **CI stays green because it runs the scan only.**
   `.github/workflows/native.yml` → `native/tools/ci-native.ps1:27` invokes
   `python tools/check_legacy_denylist.py` without `--self-test`; the full scan
   exits 0 (live run). So the self-test/scan contradiction is invisible to CI.
4. **Owner-ruling surface**: whichever disposition the owner picks can be
   encoded precisely — keep = restore nothing but document + (optionally)
   convert notes to scoped allowlist entries; migrate = versioned wave per §1.4/
   §2.4; remove = the named breakage lists above. TASK-0121 gate G-03 and
   RUN_STATUS P2 (`orchestration/RUN_STATUS.md:1113`) are waiting on exactly
   this packet.

## 4. Documentation-only references (not live consumers)

For completeness, the acceptance greps also hit historical/orchestration text
(no runtime effect): `orchestration/ONBOARDING-SOL-ORCHESTRATOR.md:164`;
`orchestration/owner-input/README.md:9` and `OI-001-denylist-dispositions.md:6-10`;
TASK-0005 REPORT (:93,:291,:296,:306,:310); TASK-0095 FINDINGS (:76) +
captures (:182,:255); TASK-0104 FINDINGS (:237); TASK-0121 FINDINGS (:43) +
captures (:139,:145); TASK-0081 capture (:110); TASK-0047 NOTES (:229);
TASK-0048 baseline capture (~60 lines); TASK-0120/TASK-0133 captures and
fixtures (gate transcripts, negative-case save fixtures using the id);
TASK-0133 `fixtures/negative-cases.json:11,53`. These are prior evidence about
the exceptions, not contracts requiring them.

## 5. Evidence boundary

Per SPEC stop conditions: no canonical replacement proposed, no compatibility
window set, no denylist/config/server/src/native/playtest/docs-product file was
modified by this task. All worker writes are confined to
`orchestration/tasks/TASK-0085-denylist-exception-audit/**`.
