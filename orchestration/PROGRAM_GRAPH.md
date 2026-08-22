# Verdigris whole-program dependency graph

Architect-owned surge map. `RUN_STATUS.md` is the current executable routing
truth; this file exposes the deeper graph and does not make DRAFT work
claimable. Product authority remains the constitution and owner-input gates.

## Terminal product gates — completion means every gate is proved

| Gate | Required proof |
|---|---|
| T1 Native parity | All D-122 axes: unchanged server/rules parity, real networked native Gates A/B/C, and presentation/feel rubric at the default path |
| T2 Owner-playable journey | Clean launch -> House/Scion -> concrete expedition choice -> combat/loot/equip -> extraction/progression -> death/successor/recovery -> quit/relaunch, with no dev grants or hidden local fallback |
| T3 Campaign and endgame | Owner-approved multi-act graph, meaningful optional branches, once-per-House campaign completion, and repeatable selectable endgame goals/areas/rewards |
| T4 Production systems | Combat, skills/magic seams, monsters/packs/uniques/bosses, itemization/history, passive progression, persistence, content tooling, networking, performance, packaging |
| T5 Presentation and experience | Final panes/typography/HUD/minimap, renderer, surface density, animation, VFX, sound, music, accessibility/options, onboarding, error/reconnect UX |
| T6 Owner-approved content | Art/assets, provenance, lore, naming, magic, balance, economy, campaign/encounter/item/skill content, music, season and travel rules explicitly approved |
| T7 Release hardening | Full regression, deterministic replay, lifecycle/long soak, clean-machine builds, asset/runtime manifests, save migration/recovery, upgrade/rollback, platform packaging and release gates |
| T8 Repeated owner verdicts | Owner play at each major slice and final release candidate, numbered friction/correction waves resolved, no quality-rubric zero, presentation captures at supported resolutions |

No task count, green unit suite, server parity, technical demo, or exhausted
queue proves completion. Terminal status requires T1-T8 current at the same
release revision/environment, with owner-only gates actually decided.

## Current critical path

```text
server/rules parity (DONE, 32/32)
  -> Gate A networked native guest journey (GREEN)
  -> TASK-0081 wire freeze
  -> TASK-0077 Gate B Chronicles client
  -> TASK-0086 Gate C audit + TASK-0096 campaign measurement
  -> resolve only evidenced Gate C missing fields
  -> TASK-0089 Gate C native journey
  -> all three D-122 axes can be called native product parity
```

Presentation runs beside that path:

```text
TASK-0115 browser panel inventory -> pane interfaces ----+
TASK-0077 Gate B client -> TASK-0078 surface density ----+-> TASK-0087 pane shell
TASK-0093 typography audit ------------------------------+       -> TASK-0090 progression panes
TASK-0114 renderer evaluation -> TASK-0088 ADR -> owner dependency ruling -> Stage-2 renderer
TASK-0094 asset provenance -> shippable asset manifest ---------------------> packaging
```

## Executable discovery to sequenced implementation

| Stream | READY evidence packet | Sequenced implementation | Later outcome |
|---|---|---|---|
| Wire/journeys | 0081, 0086, 0091 | 0077, 0089 | complete networked House/Scion/campaign journeys |
| Presentation | 0114, 0115, 0084, 0093, 0094 | 0078, 0087, 0088, 0090 | panels, typography, renderer, surface density, cohesive HUD |
| Animation/VFX | 0116 | 0122 | authored motion/effects, combat timing/readability, deterministic captures |
| Audio/music | 0117 | 0123 | device/bus/voice runtime, combat/UI cues, ambience, owner-approved music |
| Accessibility | 0118 | 0124 | rebinding, focus, scale/contrast/color/motion/audio options, persisted settings |
| Onboarding | 0119 | 0125 | legible first launch, goals, controls, rewards, death/recovery, reconnect |
| Regression/soak | 0080, 0082, 0083, 0098, 0099, 0100 | 0106 plus CI/soak releases | deterministic replay, dual-server parity, lifecycle and performance coverage |
| Content tooling | 0095, 0096 | 0113 | versioned zones, campaign graphs, assets, validators, migrations |
| Persistence | 0097 | 0107 | crash-safe, versioned House/Scion/item persistence and relaunch |
| Combat | 0101 | 0108 | deeper physical vocabulary, feedback, and D-115 feel waves |
| Skills | 0102 | 0109 | content-neutral action/slot/effect infrastructure, then owner-approved skills/magic |
| Monsters | 0103 | 0110 | packs, rarity, uniques, bosses, deterministic encounters |
| Itemization | 0085, 0104 | 0111 | stable identities, extraction, history, recovery, Brands/Bonds seam |
| Progression | 0105 | 0112 | authoritative passive tree, stats, allocation, migrations, panes |
| Packaging | 0092, 0094, 0099 | packaging/launcher releases after dependencies | owner-playable build, clean-machine launch, Windows/macOS delivery |
| Release/migration | 0120 | 0126, 0127 | clean-machine, soak, save migration/recovery, upgrade/rollback, release evidence |
| Owner content | 0121 | approved content waves by domain | art/lore/naming/balance/economy/content decisions without agent invention |

## Deeper dependency graph

1. **Native client journey completion**: Gate B and Gate C precede any claim
   of product parity. Reconnect, persistence fault coverage, and protocol
   coverage sentinels then harden those journeys.
2. **Native presentation parity**: density, pane shell, typography, and
   progression panes finish the current side-by-side deltas. The renderer ADR
   can change the backend only after deterministic offscreen capture and
   cross-platform dependency approval are preserved.
3. **World and surface density**: accepted content schema and campaign graph
   validators precede multi-act/optional-branch authoring. Asset provenance
   precedes any production-art promotion.
4. **Combat depth**: the combat audit selects content-neutral mechanics;
   scaffolded math and tests precede implementation; every feel wave ends in a
   played executable and D-114 coherence table.
5. **Skill systems and magic**: slot/action/effect infrastructure may proceed
   without content. Production Arcane Lattice integration waits on owner magic
   direction and uses the inspected WIZARD reference, never a generic default.
6. **Monsters and encounters**: deterministic pack/rarity infrastructure
   precedes owner-authored rosters, uniques, and bosses. Shared actor/stat and
   damage authority cannot fork.
7. **Itemization**: stable item identity, extraction/recovery, and history
   precede Brands/Bonds formula work. House crafting/economy services depend on
   owner decisions about roles, rates, sinks, trade, and currency exchange.
8. **Passive progression**: audit the current approximation, freeze versioned
   topology/persistence interfaces, obtain owner-approved source data, then
   build allocation, migration, and UI. Never canonize the approximation.
9. **Campaign/endgame**: schema + measured graph + owner content decisions
   precede multi-act expansion. Endgame area/goal selection consumes the same
   validated graph and content pipeline. Fast travel waits on its risk model.
10. **Legends**: item/monster stable identities + persistence durability +
    bounded replayable records precede loot/spawn-pool influence. No unbounded
    background simulation.
11. **Networking completeness**: unchanged dual-server matrix, malformed-input
    boundaries, reconnect/session replacement, Gate B/C coverage, lifecycle
    soak, and failure artifacts form the release ladder.
12. **Determinism and performance**: versioned replay precedes aggressive
    optimization; benchmark provenance and hardware tiers precede budgets;
    dense encounters and presentation must remain replayable and capturable.
13. **Packaging and launch**: content/asset manifests, persistence locations,
    renderer runtime dependencies, clean shutdown, performance gates, and
    clean-machine inventory precede installer/signing/notarization. Those
    irreversible or account-level actions remain owner-only.
14. **Owner-visible polish**: every implementation wave names the visible
    delta, captures both supported resolutions, runs default-path gates, and
    receives an architect play verdict. Technical parity is not a stopping
    point; the graph continues through combat, content, progression, campaign,
    packaging, and regression depth toward a full ARPG.
15. **Animation and VFX**: authoritative simulation events and coherent timing
    contracts precede interpolation, authored motion, particles, auras, camera
    response, and production effect breadth. Captures and play verdicts prove
    readability rather than event emission alone.
16. **Sound and music**: event-driven device/bus/voice infrastructure precedes
    owner-approved sounds, ambience, and score. No-device tests, settings,
    packaging, spatial priority, and clean shutdown are release requirements.
17. **Accessibility and onboarding**: audit precedes settings/pane/input work;
    then first-session journeys prove controls, goals, rewards, failures,
    death/recovery, and relaunch without lore invention or color-only state.
18. **Release convergence**: clean-machine packaging and save migration matrices
    consume persistence, assets, performance, networking, replay, and platform
    decisions. A release candidate loops through repeated owner play and
    correction until T1-T8 are simultaneously proven.

## Owner gates with executable fallback

| Gate | Blocks | Work that continues |
|---|---|---|
| denylist disposition | final compat cleanup | TASK-0085 evidence, all unrelated native work |
| renderer dependency | Stage-2 backend implementation | TASK-0114/0088 evidence, render-list contracts, panels |
| asset/font policy | production asset promotion and packaging | 0093/0094 inventories, procedural presentation |
| magic direction | production skill content | 0102 audit and content-neutral infrastructure |
| economy/crafting | House services and rates | stable item lifecycle, stores, schema seams |
| passive-tree source | authoritative 271-node engine/content | 0105 audit, topology/persistence scaffold design |
| campaign/fast-travel choices | authored acts, branch density, travel risk | 0095/0096 schemas and measurements |
| season inheritance | season reset release | bounded versioned record seam and non-seasonal systems |
| sound/music direction | authored audio/music content | 0117 audit, backend-neutral runtime/test design |
| art/lore/naming/balance/content approvals | production content breadth and final release | 0121 matrix, content-neutral systems/tools/tests |

No owner gate stalls unrelated work. Exact batched packets live under
`orchestration/owner-input/`.
