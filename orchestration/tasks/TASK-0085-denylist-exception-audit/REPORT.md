# TASK-0085 REPORT — acceptance transcripts

- task: TASK-0085 · state: REVIEW_REQUESTED (see STATUS.md)
- lane: ox-pc-bb · model: openrouter/stealth/ox-alpha
- branch: `worker/verdigris/pc/ox-pc-bb` · claim commit `224a0b7c`
- SPEC base_commit `d2423873c577d299b3b39c56024d1d840993c72b` verified ancestor
  of routed HEAD `f9ff44db` via `git merge-base --is-ancestor` (exit 0).
- Deliverable: `FINDINGS.md` (same folder) — occurrence/contract/visibility
  tables and three-disposition evidence for both owner-pending exceptions.
- Resource capsule honored: read-only evidence gathering; no ports bound or
  probed; port 6500 untouched; no forbidden path modified.
- Transcript capture order note: all transcripts below were captured in one
  session pass AFTER `FINDINGS.md` was written and BEFORE this `REPORT.md`
  file existed. Both files are new/untracked at capture time; `git diff` does
  not list untracked files, which is why commands 4–5 print no file names.
  The post-transcript `git status --short` at §6 shows actual worker writes.

## 1. `rg -n -F 'legacyRelicId' --glob '!orchestration/tasks/TASK-0085-denylist-exception-audit/**' .`

```text
.\config\legacy-denylist.json:32:    "legacyRelicId exception (2026-08-20)": "legacyRelicId is the LIVE Verdigris wire key for relic provenance (server/core/services/chronicles.js:312, asserted by playtest chronicles scenario). The unchanged-harness parity gate (D-116) requires native to emit it. Removed from the denylist as a documented exception - flagged for owner review in RUN_STATUS.",
.\orchestration\ONBOARDING-SOL-ORCHESTRATOR.md:164:  `legacyRelicId` wire key and `bronze-dagger`, both required by the
.\orchestration\owner-input\README.md:9:| OI-001 | WAITING_EVIDENCE (TASK-0085) | before denylist compatibility cleanup | `legacyRelicId` and `bronze-dagger` disposition |
.\playtest\scenarios\chronicles.mjs:55:      const relic = state.groundItems.find(item => item.id === 'gold-ring' && item.legacyRelicId);
.\server\player\handlers\dev.js:202:        legacyRelicId: item.legacyRelicId || null,
.\orchestration\owner-input\OI-001-denylist-dispositions.md:6:Decision required: choose separate dispositions for wire key `legacyRelicId`
.\orchestration\owner-input\OI-001-denylist-dispositions.md:9:Recommended choice: preserve `legacyRelicId` as a documented compatibility
.\server\core\services\chronicles.js:312:  item.legacyRelicId = record.id;
.\server\core\services\chronicles.js:341:  if (!item?.legacyRelicId) return false;
.\server\core\services\chronicles.js:342:  return chroniclesRepository.claimRelic(item.legacyRelicId, player);
.\native\tools\check_legacy_denylist.py:221:        ("legacyRelicId", "legacy relic id"),
.\tests\unit\instance-loot.spec.js:290:        legacyRelicId: 'relic-1',
.\tests\unit\instance-loot.spec.js:298:      legacyRelicId: 'relic-1',
.\native\src\networking.cpp:482:    put(out, "legacyRelicId", ground.relic_record_id);
.\orchestration\tasks\TASK-0005-legacy-archaeology-audit\REPORT.md:296:| Legacy schema names (`defence`, `legacyRelicId`, `legacyTile`, `LEGACY_MODE`) | `server/core/entities/player/fresh-scion-profile.js:4-9`; `server/core/services/chronicles.js:255-269`; `src/core/inventory/constants.js:3-7`; `src/core/rendering/renderer-mode.js:1-2` | The checker is identifier-substring based and has no schema/semantic rules. |
.\orchestration\tasks\TASK-0121-owner-content-approval-matrix\FINDINGS.md:43:| G-03 | naming, lore | `legacyRelicId` / `bronze-dagger` dispositions | WAITING_EVIDENCE (TASK-0085) | before denylist compat cleanup (not Gate B/C path) | occurrence/breakage table proving live consumers |
.\orchestration\tasks\TASK-0121-owner-content-approval-matrix\captures\owner-gates.json:139:      "title": "Legacy token dispositions: wire key legacyRelicId and item id bronze-dagger (OI-001)",
.\orchestration\tasks\TASK-0121-owner-content-approval-matrix\captures\owner-gates.json:145:      "packet_recommendation": "OI-001 recommends preserving legacyRelicId as a documented compatibility wire key until a versioned migration exists, and migrating bronze-dagger away from player-visible/canonical content while retaining only the minimum compatible read path proven necessary. Packet recommendation, not an owner ruling.",
.\orchestration\tasks\TASK-0120-release-verification-gap-audit\captures\gate-1-rg-release-surface.txt:325:orchestration\owner-input\README.md:9:| OI-001 | WAITING_EVIDENCE (TASK-0085) | before denylist compatibility cleanup | `legacyRelicId` and `bronze-dagger` disposition |
.\orchestration\tasks\TASK-0104-itemization-history-gap-audit\FINDINGS.md:237:| `chroniclesRelic`/`legacyRelicId` | relic ground items | relicId/scionId/scionName provenance | networking.cpp:474-486 |
.\orchestration\tasks\TASK-0095-content-authoring-schema-audit\FINDINGS.md:76:  `legacyRelicId` denylist exception documented in `config/legacy-denylist.json`).
.\orchestration\tasks\TASK-0095-content-authoring-schema-audit\captures\content-surfaces.json:255:      "form": "mechanics with stable wire keys (legacyRelicId documented denylist exception)",
.\orchestration\tasks\TASK-0133-save-migration-rollback-contract\captures\gate3-seam-inventory.txt:466:orchestration\tasks\TASK-0005-legacy-archaeology-audit\REPORT.md:296:| Legacy schema names (`defence`, `legacyRelicId`, `legacyTile`, `LEGACY_MODE`) | `server/core/entities/player/fresh-scion-profile.js:4-9`; `server/core/services/chronicles.js:255-269`; `src/core/inventory/constants.js:3-7`; `src/core/rendering/renderer-mode.js:1-2` | The checker is identifier-substring based and has no schema/semantic rules. |
EXIT=0
```

Exit code: **0**.

## 2. `rg -n -F 'bronze-dagger' --glob '!orchestration/tasks/TASK-0085-denylist-exception-audit/**' .`

```text
.\config\legacy-denylist.json:33:    "bronze-dagger exception (2026-08-20)": "bronze-dagger is the LIVE starter-kit item id asserted by the town-amenities scenario (unchanged-harness law D-116). Removed as a documented exception pending owner review."
.\native\tools\check_legacy_denylist.py:212:        ("bronze-dagger", "bronze dagger"),
.\playtest\scenarios\town-amenities.mjs:17:    const starterDagger = state.inventory.find(item => item.id === 'bronze-dagger');
.\native\src\networking.cpp:1392:    {"bronze-sword", "bronze-dagger", "wooden-shield"},
.\native\src\networking.cpp:2209:  for (const auto& item : inventory_.items()) if (item.id != "coins" && item.id != "bronze-dagger") pending_relic_items_.push_back(item);
.\native\src\networking.cpp:2395:    int price = item_id == "bronze-sword" ? 15 : item_id == "bronze-dagger" ? 10 : item_id == "wooden-shield" ? 8 : -1;
.\native\src\networking.cpp:2626:      for (const auto& item:inventory_.items()) { if (item.id=="bronze-dagger") has_dagger=true; if (item.id=="coins") coins+=item.qty; }
.\native\src\networking.cpp:2627:      if (!has_dagger) { CreateItemOptions o; auto dagger=create_game_item("bronze-dagger",o); if (dagger) inventory_.add(std::move(*dagger)); }
.\native\src\core.cpp:2668:    {"bronze-dagger", "Bronze Dagger", "weapon", "right_hand", false, false, {4, 2, -1, 0}, {0, 1, 0, 0}, 0, 0, "", ""},
.\orchestration\owner-input\OI-001-denylist-dispositions.md:7:and item id `bronze-dagger` after the occurrence/breakage table is accepted.
.\orchestration\owner-input\OI-001-denylist-dispositions.md:10:wire key until a versioned migration exists; migrate `bronze-dagger` away from
.\orchestration\owner-input\README.md:9:| OI-001 | WAITING_EVIDENCE (TASK-0085) | before denylist compatibility cleanup | `legacyRelicId` and `bronze-dagger` disposition |
.\orchestration\ONBOARDING-SOL-ORCHESTRATOR.md:164:  `legacyRelicId` wire key and `bronze-dagger`, both required by the
.\orchestration\tasks\TASK-0005-legacy-archaeology-audit\REPORT.md:93:still contains `bronze-dagger` (`server/core/data/items/weapons.js:91-118`).
.\orchestration\tasks\TASK-0005-legacy-archaeology-audit\REPORT.md:291:| Hyphenated `bronze-dagger` | `server/core/entities/player/fresh-scion-profile.js:13-19`; template `server/core/data/helpers/player.json:39-49`; item definition `server/core/data/items/weapons.js:91-118` | Denylist has `bronze dagger` and `bronze_dagger`, not `bronze-dagger`; native data files are not scanned at all. |
.\orchestration\tasks\TASK-0005-legacy-archaeology-audit\REPORT.md:306:   Scions receive a `bronze-dagger` and 100 `coins`, and four starter skills
.\orchestration\tasks\TASK-0005-legacy-archaeology-audit\REPORT.md:310:   constitution denies bronze-dagger/generic starting coins and
.\tests\unit\chronicles-login.spec.js:276:    expect(admitted.inventory.map(item => item.id)).toEqual(['bronze-dagger', 'coins']);
.\tests\unit\chronicles-login.spec.js:422:    expect(successor.inventory.map(item => item.id)).toEqual(['bronze-dagger', 'coins']);
.\tests\unit\fresh-scion-profile.spec.js:19:    expect(first.inventory.map(item => item.id)).toEqual(['bronze-dagger', 'coins']);
.\tests\unit\fresh-scion-profile.spec.js:60:    expect(fresh.inventory.map(item => item.id)).toEqual(['bronze-dagger', 'coins']);
.\tests\unit\equipment-replacement.spec.js:155:        id: 'bronze-dagger',
.\tests\unit\equipment-replacement.spec.js:184:      id: 'bronze-dagger',
.\tests\unit\equipment-replacement.spec.js:234:        id: 'bronze-dagger',
.\tests\unit\loot-first-find.spec.js:180:    id: 'bronze-dagger',
.\tests\unit\inventory-system.spec.js:68:    expect(resolveItemSize({ id: 'bronze-dagger', slot: 'right_hand', type: 'weapon' })).toEqual({ width: 1, height: 2 });
.\tests\unit\inventory-store.spec.js:155:      id: 'bronze-dagger',
.\tests\unit\inventory-item-presentation.spec.js:33:    expect(resolveInventoryItemArtId({ id: 'bronze-dagger' })).toBe('dagger_bronze');
.\tests\unit\inventory-item-presentation.spec.js:34:    expect(resolveInventoryItemArt({ id: 'bronze-dagger' })).toMatch(/dagger_bronze/i);
.\tests\unit\stale-player-snapshot.spec.js:15:    expect(fresh.inventory.slots.map(item => item.id)).toEqual(['bronze-dagger', 'coins']);
.\orchestration\tasks\TASK-0121-owner-content-approval-matrix\FINDINGS.md:43:| G-03 | naming, lore | `legacyRelicId` / `bronze-dagger` dispositions | WAITING_EVIDENCE (TASK-0085) | before denylist compat cleanup (not Gate B/C path) | occurrence/breakage table proving live consumers |
.\orchestration\tasks\TASK-0121-owner-content-approval-matrix\captures\owner-gates.json:139:      "title": "Legacy token dispositions: wire key legacyRelicId and item id bronze-dagger (OI-001)",
.\orchestration\tasks\TASK-0121-owner-content-approval-matrix\captures\owner-gates.json:145:      "packet_recommendation": "OI-001 recommends preserving legacyRelicId as a documented compatibility wire key until a versioned migration exists, and migrating bronze-dagger away from player-visible/canonical content while retaining only the minimum compatible read path proven necessary. Packet recommendation, not an owner ruling.",
.\src\core\inventory\item-art.js:22:  'bronze-dagger': 'dagger_bronze',
.\orchestration\tasks\TASK-0120-release-verification-gap-audit\captures\gate-1-rg-release-surface.txt:305:orchestration\owner-input\OI-001-denylist-dispositions.md:10:wire key until a versioned migration exists; migrate `bronze-dagger` away from
.\orchestration\tasks\TASK-0120-release-verification-gap-audit\captures\gate-1-rg-release-surface.txt:325:orchestration\owner-input\README.md:9:| OI-001 | WAITING_EVIDENCE (TASK-0085) | before denylist compatibility cleanup | `legacyRelicId` and `bronze-dagger` disposition |
.\orchestration\tasks\TASK-0081-gate-b-wire-contract\captures\gate-b-wire-contract.json:110:      "responseNotes": "Side effects: pending_chronicles_=false (:2522), home pitch hash from active_house_id_ (:2525-2529), once-per-scion starter kit (bronze-dagger if absent, coins topped to 100) (:2537-2543), world reset to town at the House wagon pitch (:2544-2545), emit_login (:2546).",
.\orchestration\tasks\TASK-0047-native-protocol-n4\NOTES.md:229:gold-ring, hide-girdle, bronze-sword, bronze-dagger, bronze-pike +
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:344:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:383:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:810:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:849:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:1289:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:1328:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:1774:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:1813:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:2243:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:2282:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:2728:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:2767:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:3197:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:3236:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:3682:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:3721:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:4151:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:4190:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:4636:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:4675:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:5105:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:5144:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:5590:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:5629:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:6059:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:6098:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:6544:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:6583:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:7013:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:7052:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:7498:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:7537:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:7967:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:8006:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:8452:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:8491:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:8921:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:8960:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:9406:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:9445:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:9875:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:9914:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:10360:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:10399:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:10829:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:10868:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:11314:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:11353:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:11783:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:11822:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:12268:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:12307:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:12737:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:12776:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:13222:              "id": "bronze-dagger",
.\orchestration\tasks\TASK-0048-chronicles-silent-combat\captures\baseline-c3988-wire.json:13261:              "baseId": "bronze-dagger",
.\orchestration\tasks\TASK-0095-content-authoring-schema-audit\captures\content-surfaces.json:182:      "locking_risk": "channel rating numbers feed combat parity tests; bronze-dagger is a documented denylist exception kept for unchanged-harness law D-116"
.\orchestration\tasks\TASK-0133-save-migration-rollback-contract\fixtures\negative-cases.json:11:        "content": { "level": 3, "inventory": [{ "id": "bronze-dagger", "uuid": "00000000-0000-4000-8000-000000000001" }] },
.\orchestration\tasks\TASK-0133-save-migration-rollback-contract\fixtures\negative-cases.json:53:        "content": { "savedAt": 1, "level": 9, "questPoints": 5, "inventory": [{ "id": "bronze-dagger", "uuid": "u-2" }] },
.\orchestration\tasks\TASK-0133-save-migration-rollback-contract\captures\gate3-seam-inventory.txt:107:orchestration\owner-input\OI-001-denylist-dispositions.md:10:wire key until a versioned migration exists; migrate `bronze-dagger` away from
.\orchestration\tasks\TASK-0133-save-migration-rollback-contract\captures\gate3-seam-inventory.txt:463:orchestration\tasks\TASK-0005-legacy-archaeology-audit\REPORT.md:291:| Hyphenated `bronze-dagger` | `server/core/entities/player/fresh-scion-profile.js:13-19`; template `server/core/data/helpers/player.json:39-49`; item definition `server/core/data/items/weapons.js:91-118` | Denylist has `bronze dagger` and `bronze_dagger`, not `bronze-dagger`; native data files are not scanned at all. |
.\server\core\data\items\weapons.js:93:    id: 'bronze-dagger',
.\server\core\entities\player\fresh-scion-profile.js:14:  ItemFactory.createById('bronze-dagger', {
.\server\core\data\helpers\player.json:41:      "id": "bronze-dagger",
.\server\core\services\wagon-service.js:27:    items: ['bronze-sword', 'bronze-dagger', 'bronze-mace', 'wooden-shield', 'leather-body', 'bronze-helm'],
EXIT=0
```

Exit code: **0**.

## 3. `rg -n 'legacyRelicId|bronze-dagger' config/legacy-denylist.json`

```text
32:    "legacyRelicId exception (2026-08-20)": "legacyRelicId is the LIVE Verdigris wire key for relic provenance (server/core/services/chronicles.js:312, asserted by playtest chronicles scenario). The unchanged-harness parity gate (D-116) requires native to emit it. Removed from the denylist as a documented exception - flagged for owner review in RUN_STATUS.",
33:    "bronze-dagger exception (2026-08-20)": "bronze-dagger is the LIVE starter-kit item id asserted by the town-amenities scenario (unchanged-harness law D-116). Removed as a documented exception pending owner review."
EXIT=0
```

Exit code: **0**.

## 4. `git diff --check`

```text
(no output)
EXIT=0
```

Exit code: **0** — no whitespace errors.

## 5. `git diff --name-only`

```text
(no output)
EXIT=0
```

Exit code: **0** — no tracked-file modifications. Worker deliverables
(`STATUS.md`, `FINDINGS.md`, `REPORT.md`) are new files confined to the owned
path `orchestration/tasks/TASK-0085-denylist-exception-audit/`; untracked files
are not listed by `git diff --name-only` (see capture-order note in header).

## 6. Supplemental evidence cited by FINDINGS §3 (gate mechanics)

Live checker runs at evidence time:

```text
> python native/tools/check_legacy_denylist.py --self-test
self-test: expected denied variant was missed: bronzeDagger
EXIT=1

> python native/tools/check_legacy_denylist.py
native legacy denylist: PASS
EXIT=0
```

Interpretation (evidence only): the scan passes because the canonical terms were
deleted from `identifiers` (commits `7ab99b65`, `f33cc15a`), while the checker's
self-test still pins those terms as expected-denied variants and therefore
exits 1. CI runs only the scan form (`native/tools/ci-native.ps1:27`).

Worker-write confinement immediately before the handoff commit:

```text
> git status --short
?? orchestration/tasks/TASK-0085-denylist-exception-audit/FINDINGS.md
```

(`STATUS.md` was committed at claim time `224a0b7c`; `REPORT.md` was being
written at capture moment.)

## 7. Handoff

- FINDINGS.md is the deliverable for OI-001 / TASK-0121 G-03 consumption.
- No disposition selected; no rename performed; no denylist/config/server/src/
  native/playtest/docs-product change made. Evidence boundary respected per
  SPEC stop conditions.
