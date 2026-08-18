# Delaford checklist coverage in Verdigris

This matrix maps the preserved historical checklist to current evidence. It
does not override the Verdigris constitution or turn every historical item into
a product requirement.

| Historical item | Current Verdigris status | Evidence / boundary |
|---|---|---|
| Player walking / pathfinding | Covered | Native movement feel and browser movement gates; current full playtest movement passes. |
| Player context-menu / actions | Partial | Existing pane/context actions are covered; TASK‑0038's LMB/RMB and rebinding seam remains blocked pending ownership decision. |
| Health and stats | Covered | Native shared stats/combat model and browser stat/combat integration tests. |
| Inventory / character wear | Covered | Inventory, equipment, persistence, and current playtest gates. |
| First quest | Covered | `first-goal`, `quest`, and full playtest campaign checks. |
| Inventory / quests / chat / wear UI | Covered in browser reference | Focused UI tests and full playtest; presentation remains browser-track evidence, not native-core authority. |
| NPC trading / banking / walking | Covered | Economy, town amenities, and full playtest gates. |
| NPC dialog interaction | Covered for current campaign path | Aldwyn Talk interactions and quest progression are exercised; general dialog authoring is not a separate native system. |
| Monster battle / looting / spawning | Covered in browser and native slices | Combat, loot, encounter, monster lifecycle, and N2 zone population gates. |
| Networking: players / NPCs / monsters / items | Covered at current slice scope | Party/session regression, browser world web, and committed N2 native protocol gates. |
| Networking: player trading | Covered as browser economy path | Trade/shop/bank playtest evidence; not yet a native multiplayer economy protocol. |
| Respawn | Covered | Death/recoverability and respawn gates. |
| Player versus Player | Deferred | No current Verdigris acceptance requirement; requires an explicit product/authority decision before implementation. |
| Resource skills: mining, smithing, fishing, cooking | Deferred | No authoritative current implementation/test seam; do not infer these from the historical checklist. |

The historical source remains
[`DELAFORD_README_FEATURES_CHECKLIST.md`](../archive/DELAFORD_README_FEATURES_CHECKLIST.md).
Current product authority is
[`VERDIGRIS_FEATURE_CHECKLIST.md`](../product/VERDIGRIS_FEATURE_CHECKLIST.md)
and the constitution.
