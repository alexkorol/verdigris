# Verdigris checklist gap audit

Coordinator audit: 2026-08-17. This document translates the user-authored
checklist into evidence-backed implementation gaps. It does not check items in
the product checklist or invent decisions; “not evidenced” means that the
current repository does not yet prove the requirement at the required product
surface.

## Evidence baseline

- Browser reference gates: 122 unit files / 779 tests, production build, full
  playtest 31/31, and browser-critical smoke 1/1.
- Native gates: denylist, core, networking, and client shell pass; native N2
  attach regression passes quickstart, single-session, movement, and zones 4/4.
- Product boundaries: [Verdigris Constitution](../product/VERDIGRIS_CONSTITUTION.md),
  [feature checklist](../product/VERDIGRIS_FEATURE_CHECKLIST.md), and
  [open decisions](../product/OPEN_DECISIONS.md).
- Protocol parity is staged by
  [PARITY_ROADMAP.md](PARITY_ROADMAP.md): N1/N2 are the current native
  transport/world slice; N3–N7 remain future parity waves.

## Player

| Requirement | Current evidence | Audit result |
|---|---|---|
| WASD walking, mouse aiming, LMB/RMB, Q/E/R, Z, X, auto-pickup, F, I/Tab | Native D-007 client implementation and driven-input evidence; browser reference has live movement, context actions, inventory, and skill UI | Covered across the two surfaces, but not yet one unchanged-client/native-server parity surface |
| No context menu in the native scheme | Native client contract is direct-control based; browser still uses context-menu actions for existing UI flows | Covered natively; browser controls remain partial under TASK-0038 / QUESTION-0008 |
| Inventory, stats, passive tree | Browser inventory, stats, and geometric skill-tree panes are tested; native simulation owns item/stats/tree state | Browser presentation covered; native presentation parity not yet evidenced |
| Worn equipment reflected on the in-world character | Browser equipment/wear and item presentation are tested; native client has equipment state and billboard/capsule presentation | Browser covered; native visual reflection needs a D-115 play/capture check |
| Loose quest threads | Browser Aldwyn/quest chain is covered by `first-goal`, `quest`, and `session-arc`; native quest parity is N6 | Browser covered; native not yet evidenced |

## Houses and progression

| Requirement | Current evidence | Audit result |
|---|---|---|
| Meta-progression | House-owned routes, unlocks, renown/treasury browser paths, persistence, and native House state are tested | Covered in current slices |
| Persistent Legends records | Native bounded deterministic records, persistence, founding milestones, relic/trophy history, and seasonal extension are tested | Bounded record system covered; full simulation influence on future loot/spawn pools remains open (OD-010) |
| Passive income, async trading, currency exchange | Constitution/checklist intent only; economy rates, sinks, and authority are explicitly open (OD-007/OD-009) | Not evidenced; owner economy decision required |
| 6–30 hour campaign range | Multizone route graph exists, but no accepted playable measurement for shortest/optional route duration | Not evidenced; OD-011/playable campaign measurement required |

## UI

| Requirement | Current evidence | Audit result |
|---|---|---|
| Character+inventory and trade+inventory panes | Browser `PaneHost`, inventory, shop, bank, and trade flows are exercised | Browser covered; native equivalent not yet evidenced |
| Minimap small/overlay modes and transparency/zoom/side settings | Browser `WorldMinimap` and pane-aware layout exist; login payload exposes minimap metadata | Basic browser minimap covered; the complete two-mode/settings contract remains open (OD-013) |
| Consistent orbs, skill bar, and other UI language | WIZARD orb/quickbar seam verification is 10 files / 73 tests; browser HUD tests pass | Browser seam covered; final native visual coherence remains a D-115 judgment |

## Campaign

| Requirement | Current evidence | Audit result |
|---|---|---|
| Multizone graph across acts | Browser world-web and native route/zone graph prove named routes, gates, stairs, and House-owned unlocks; N2 proves six zone payloads | Current slice covered; full native N6 parity remains |
| Optional branches / league unlocks | Native bounded branch and seasonal extension hooks exist; browser campaign has named branches | Mechanical seam exists; branch density and league content are not yet product-complete |
| Once per House per season | Seasonal objective/reward extension exists, but reset/inheritance semantics are owner-only (OD-001) | Not evidenced as a final rule |

## Monsters and loot

| Requirement | Current evidence | Audit result |
|---|---|---|
| Shared stats/elements | Native actor symmetry and browser stat/combat integration tests | Covered in current slices |
| Packs, rarity, uniques | Browser encounter-variety, monster lifecycle, boss, and instance-balance gates; native elite/combat tests | Covered in slices; native protocol N3 is next |
| Scarce equipment, generous trophies/materials | Browser loot/encounter/depth-loot gates and native item/trophy/relic paths | Current browser/native slices covered; full native payload parity and crafting-material economy remain |

## World

| Requirement | Current evidence | Audit result |
|---|---|---|
| Respawn/permadeath | Browser soft respawn and opted-in hard/mortal Chronicles paths are tested; native death/successor/recovery is deterministic | Both lifecycle modes are evidenced; final product presentation still needs D-115/owner confirmation |
| Persistent Legends items/monsters in future pools | Legends/history and relic/trophy re-entry are covered; bounded future spawn/loot influence is not yet proven | Partial; OD-010 remains open |
| Fast travel or town portals | Browser scene metadata contains portals and N2 zones expose stairs/return; risk/cost/destination rules are open | Structural portal seams exist; product behavior is not finalized (OD-012) |

## Authorized next work

1. Fable accepts or revises TASK-0043 and TASK-0044; only accepted commits may
   be integrated.
2. QUESTION-0009 records the N3 authority-bridge choice and the request for a
   READY N3 task/spec. Native parity then proceeds through N3 combat, N4
   items, N5 Chronicles, and N6 world-web/quests, each with the unchanged
   scenario matrix.
3. QUESTION-0007 and QUESTION-0008 must be resolved before the first-loot and
   browser-controls presentation work can be implemented safely.
4. OD-001, OD-007/009, OD-010, OD-011, OD-012, and OD-013 remain owner/product
   decisions rather than implementation assumptions.

## N3 parity handoff (read-only surface audit)

The clean N2 `ProtocolSession` currently handles `player:login`,
`world:zone:enter`, `instance:enterSolo`, `player:move`, `dev:teleport`,
`dev:give`, and `dev:state`. It does not yet expose the browser combat
combat/skill flow (movement-triggered melee and `player:skill:trigger`),
authoritative combat updates, monster deaths, or loot pickup over the native
protocol. Those are the concrete
N3 surface gaps behind the `combat` and `encounter-variety` scenarios.

The handoff rule is unchanged: map the existing wire events into the
deterministic core, keep gameplay rules out of `networking.cpp`, and close the
N3 scenario matrix before moving to N4 item/inventory payloads. This is a gap
inventory, not authorization to modify the native branch before Fable issues an
N3 task/spec.

The exact event/core mapping and acceptance matrix are preserved in
[`N3_PARITY_IMPLEMENTATION_BRIEF.md`](N3_PARITY_IMPLEMENTATION_BRIEF.md).
