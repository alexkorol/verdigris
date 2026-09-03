# Native reconstitution handoff

## 2026-09-03 - complete WIZARD Framekit raster runtime

- Froze WIZARD Framekit 2.0's complete 99-image, 7.9 MB flagship `game/assets`
  pack plus its generated layout at source commit
  `00be6a0fe4ecfbd0b7862249012308268f92f0d7`. The native manifest owns an
  aggregate content hash, exact count/bytes, layout hash, and the dimensions of
  eleven production-critical runtime images.
- The binding Windows build now verifies that complete pack before running any
  test. A corrupt foundational slice, missing flagship raster, aggregate drift,
  bad layout, or missing/wrong-sized runtime component fails closed.
- Native panes replace the tiny procedural Framekit panel with the authored
  marble-and-brass crop while retaining true nine-slice geometry. The live HUD
  draws WIZARD's asymmetric statue orb chrome, four skill medallions, XP rail,
  winged objective backing, status control, and hover frame; every call keeps
  the prior vector treatment as an explicit decoder fallback.
- Scenario evidence is fail-visible rather than inferred from a load attempt:
  `character-inventory-diptych` requires all 11 decodes and proves the panel,
  both orbs, rail, controls, and four medallions actually drew at 960x600 and
  1366x768; `hud-information` separately proves the raster tooltip path.
- The exact binding native gate passes the asset verifier, every unit/session
  suite, the mortal succession and heirloom-recovery journey, and all 29 client
  scenarios. Inspected evidence is isolated under
  `native/build/goal-captures-framekit-final-2/`; the 3440x1440 frame budget is
  10.4 ms average against the 40 ms ceiling.

## 2026-09-03 - charted-tablet recharting

- Added a town-only, sealed-House rechart action to the consumable endgame
  loop. Spending 50 carried gold replaces a tablet's two rolled risk/reward
  clauses with a materially different pair while preserving its UUID, family,
  tier, layout, and 64-objective Wayfinder mastery seal.
- The server validates campaign completion, town state, exact item identity,
  and carried funds before mutation. Invalid, underfunded, out-of-town, and
  pathological no-change rolls fail without spending gold or consuming the
  tablet.
- The Framekit gear footer exposes a gold `V rechart 50g` keyboard-and-pointer
  action; Enter remains the distinct destructive action that breaks the selected one-use tablet. Inventory
  refresh immediately redraws its two clauses, goods bonus, and unchanged
  mastery preview.
- Networking coverage proves the gold delta, different clause set, and stable
  identity/tier/mastery contract. The `endgame-tablet-ui` production scenario
  proves the new control remains discoverable and fitted at 1366x768.
- The exact binding native gate passes every unit/session suite, the mortal
  succession and heirloom-recovery journey, and all 29 client scenarios.
  Inspected evidence is isolated under
  `native/build/goal-captures-tablet-rechart-final/`; the 3440x1440 frame
  budget remains 11.2 ms average against the 40 ms ceiling.

## 2026-09-03 - authored Warden disciplines

- Replaced the one-size-fits-all Warden ground slam with four deterministic,
  server-authored combat families shared by the campaign roads and their
  corresponding endgame tablets: Tin/Barrow Stonefall, Salt/Reeds Tidal Mark,
  Chalk/Crown Grave Ring, and Copper/Thorns Ember Crucible.
- Telegraph events now carry exact action identity, circle/ring shape, damage
  channel, outer radius, safe-eye radius, sampled center, and windup. The same
  stored geometry resolves the hit; River and Ember attacks use real
  resistances, and each discipline has its own cooldown and tier-scaled damage.
- The remote presentation seam no longer collapses boss actions into a thrust.
  Production rendering distinguishes the four hazards by color and geometry,
  including a genuinely hollow Grave Ring, while retaining Framekit minimap
  and bottom-HUD exclusion.
- Core tests cover sampled-target dodging, elemental mitigation, cooldowns,
  and Grave Ring's safe eye. Campaign protocol tests cover all four road
  contracts, presentation tests preserve the annulus, and the new
  `warden-disciplines` scenario captures all four warnings in one 1366x768
  production frame.
- The exact binding native gate passes every unit/session suite, the mortal
  succession and heirloom-recovery journey, and all 29 client scenarios.
  Inspected evidence is isolated under
  `native/build/goal-captures-warden-disciplines-final/`; the 3440x1440 frame
  budget remains 9.8 ms average against the 40 ms ceiling.

## 2026-09-03 - complete 23-point native campaign

- Expanded the authoritative campaign from twelve to all twenty-three quest
  points reserved by the passive tree. Acts IV-VI add the Crownless Marches,
  War of Claimants, and Verdigris Crown across real tier-three through
  tier-five road instances rather than synthetic point awards.
- Eleven new commissions carry fixed titles, givers, objectives, deeds,
  rewards, twelve canonical story holdings, and Wardens. The six-part finale
  first seals the Sepulchral Sanctum and returns alive, then breaks the
  Verdigris Usurper at the Empty Throne and requires a final living return
  before the House is marked complete.
- Road rites now require their exact authored tier as well as road identity;
  an inherited, deeper House holding cannot satisfy a shallower Scion quest.
  Parent-gated node clears, true instance depth, House road persistence, Scion
  checkpoints, and successor inheritance remain authoritative.
- Endgame unlock moved to the twenty-third return. Existing Houses carrying a
  legacy House-level campaign seal retain Wayfinder access and inherit the
  complete 23-point budget, while an in-progress twelve-point Scion resumes at
  Act IV. The Framekit journal now has a dedicated Act IV regression capture
  and keeps its bounded nine-deed rail for the longer Chronicle.
- The exact binding native build, complete unit/session suite, mortal
  succession and heirloom-recovery Gate B, and all 28 client scenarios pass.
  Inspected Act IV and Act VI evidence lives under
  `native/build/goal-captures-campaign-23-final/`; the 960x600 finale proves
  the modal journal suppresses top-HUD overpaint, and the 3440x1440 frame
  budget is 9.7 ms average against the 40 ms ceiling.

## 2026-09-03 - Framekit House and Scion front door

- Replaced the Chronicles text stack with a responsive two-pane Framekit
  lineage surface. The House ledger now presents the permanent lineage,
  living and remembered counts, completed charted expeditions, living Scion
  cards, the latest crypt record, and the House's 64-objective Wayfinder
  mastery progress without inventing client-side authority.
- The admission rail turns the existing server-authored House founding,
  Scion creation, set-out/succession, and mortal-oath commands into selectable
  cards. Arrow keys, Enter, legacy direct shortcuts, pointer hover, and click
  all resolve through the same deterministic action list and exact hit bounds;
  large lineages page around the active selection instead of clipping their
  later Scions, creation command, or oath control.
- Kept the author-written naming ceremony on the real Framekit path and
  preserved the complete Gate-B journey: found House, name a classless Scion,
  take the mortal oath, die, name a successor, recover the heirloom, and
  reconnect into the same lineage.
- Added `chronicles-lineage-ui`, a production-painter scenario that captures
  960x600 and 1366x768 and asserts pane separation, bounded actions, House,
  living/crypt/mastery state, directional selection, pointer oath choice, and
  the oath-bearing server admission command. Inspected evidence lives under
  `native/build/goal-captures-chronicles-lineage-final/`. The exact binding
  native build, complete test set, Gate-B succession journey, and all 26
  client scenarios pass; the 3440x1440 frame budget is 9.9 ms average against
  the 40 ms ceiling.

## 2026-09-03 - authoritative Framekit loadout and spatial backpack

- Replaced the native client's inferred single-weapon equipment view with an
  ordered mirror of all eleven authoritative `WearSet` seats. `dev:state` and
  `player:equippedAnItem` now refresh exact per-seat uuid, catalog id, name,
  footprint, equip slot, two-handed state, and living-Vessel detail; equipped
  goods no longer masquerade as backpack rows.
- Extended the shared item identity projection with `equipSlot` and
  `twoHanded`. The existing `size`, `slot`, and `qty` fields now survive the
  complete server-to-model-to-presentation path instead of being discarded by
  the remote client.
- Rebuilt the I pane as the real 12x7 spatial backpack used by native rules.
  WIZARD Framekit chrome draws every cell and each multi-cell footprint;
  stack counts, mapped item art, charted-tablet seals, selected-item detail,
  mouse hover, and directional nearest-neighbor keyboard navigation share the
  same authoritative geometry.
- Rebuilt the C pane as a responsive paper doll around the live Scion plate.
  Head, armor, back, gloves, belt, feet, both hands, necklace, and both rings
  expose filled and empty state explicitly, with a grouped vitals, attributes,
  combat, mobility, and ward readout below. Opening C and I together produces
  a focused, non-overlapping loadout diptych at both 960x600 and 1366x768.
- Added protocol regression coverage for full snapshot/equip-event WearSet
  refresh and for backpack slot/footprint/stack/equipment metadata. The
  `character-inventory-diptych` production scenario asserts all eleven seats,
  six bounded multi-cell footprints, hover identity, spatial arrow movement,
  and captures both supported reference sizes.
- Verification: the binding native build, complete test set, and full client
  scenario suite pass. Inspected evidence is in
  `native/build/goal-captures-character-inventory-final2/character-inventory-diptych-960x600.png`
  and `character-inventory-diptych-1366x768.png`; the 3440x1440 frame budget
  is 12.4 ms average against the 40 ms ceiling.

## 2026-09-03 - living Vessel powers in combat

- Learned Vessel properties now travel from the exact worn item through wear
  totals into the authoritative world simulation. Slaughter owns Health and
  Mana recovery on kill, four-second attack cadence, critical chance against
  Bleeding foes, and Echoing Kill's 15% doubled-spoils roll. Wayfaring owns
  three-second movement bursts, moving projectile avoidance, moving Health
  regeneration, and the first-strike Untraceable miss.
- Warding now participates in the actual incoming-hit pipeline. Ordinary
  shield block is active, Stand Your Ground augments it while stationary,
  Shieldwall heals on a successful block, Old Grudge raises the next two
  seconds of Armour, and awakened Last Stand catches one killing blow per
  floor. Base worn defense now supplies the Armour rating those effects use.
- Reactive rolls use a dedicated seeded combat stream. Equipping a conditional
  defense can change survival without perturbing deterministic floor layout or
  the independent loot stream. Once-per-battle awakened charges and all timed
  Bond states reset on each generated floor.
- `player:bond:effect`, ordinary defense feedback, combat-state packets, and
  reconnect snapshots carry resolved trigger identity, amount, duration,
  readiness, and remaining time. The client presents a colored world pulse,
  named toast/event-log beat, and compact Framekit chips above the quickbar for
  Rhythm, Sprint, Grudge, Last Stand, and Untraceable.
- The 768p+ Gear Framekit now expands into available vertical space and gives
  the selected item a wrapped, color-coded `LIVING VESSEL` card. Innate lines,
  Bonds, awakenings, and the two authoritative combat-total rows are visible
  together instead of silently clipping after two properties.
- Tooltips now label the ten Bond modifiers and three awakenings backed by live
  native rules as active. Clear Mind, Superstition, and Twinned Voice remain
  explicitly Dormant because native mana-ability, curse, and rite systems do
  not yet exist; no tooltip claims a trigger the game cannot produce.
- Coverage exercises tier scaling, kill recovery/buffs, Dead Sprint travel,
  Second Wind, Bleeding-conditional criticals, stationary block healing, Old
  Grudge Armour, and the Untraceable-to-Last-Stand defensive order. Scripted
  wire and shared-presentation checks prove state and named trigger FX survive
  end to end. The full succession driver now uses one shortest-path contract
  for revealed elites and surfaced relics and secures the room before handing
  control to the loot-only pickup leg.
- Verification: the binding native build, full test set, and complete client
  scenario suite pass. Inspected evidence is in
  `native/build/goal-captures-bond-combat-final/vesselforge-active-properties-1366x768.png`;
  the 3440x1440 frame budget is 9.7 ms average against the 40 ms ceiling.

## 2026-09-03 - living Vessel Bonds and awakening

- Ported the WIZARD living-item progression into native without importing its
  class archetypes. Only Vessel gear worn through an authoritative floor clear
  gains 16–30 Attunement; crypt/dungeon, marsh, grove, and other roads write
  stable classless theme memory into the item.
- Original thresholds are preserved: 80 for the first evolution and +55 for
  each later one. Free Vessel slots form unique themed Bonds, later evolutions
  deepen the weakest Bond through tier III, and three mature Bonds can awaken
  the item into a deterministic name bound to the Scion who lived those roads.
- The complete item block is refreshed atomically after a clear and crosses
  inventory/wear snapshots and reconnects: Bonds, bases, tiers, theme memory,
  XP/next threshold, evolution count, awakened name, power, and flavor.
- Tamar's Framekit detail now exposes Attunement, Bond/evolution counts, tiered
  Bond lines, and awakened state. At this milestone those conditional lines
  were still Dormant; the following living-power milestone activates every
  trigger supported by the native combat vocabulary. Brand searing counts
  Bonds against capacity, closing the possibility of overwriting a learned slot.
- Deterministic core coverage runs an item through its full three-Bond,
  tier-III, awakening life and replays the exact rolls. Protocol coverage wears
  one exact UUID through five real dungeon floors, verifies its Warding memory,
  dormant wire truth, and reconnect identity. The production Tamar scenario
  asserts its live progress and dormant Bond presentation at 960x600 and
  1366x768.
- Verification: the binding native build, full test set, and complete client
  scenario suite pass. Inspected Framekit evidence is in
  `native/build/goal-captures-vessel-bonds-final/town-vesselforge-960x600.png`
  and `town-vesselforge-1366x768.png`; the 3440x1440 frame budget remains
  9.7 ms average against the 40 ms ceiling.

## 2026-09-03 - Tamar's authoritative town Vesselforge

- Added Tamar the Vesselwright as the Crossroads' fifth authored townsfolk,
  with stable identity, plaza position, forge services, conversation copy, and
  a server-owned `Open the Vesselforge` option.
- Brand-searing is now a real social service: both opening and mutating require
  the Scion to stand within one tile of Tamar. Forged remote actions spend and
  mutate nothing.
- The server publishes exact carried vessel UUIDs, material/form, level,
  Vessel use/free capacity, Patience, existing active/Dormant lines, purse,
  100-gold cost, eligibility, and disabled reason. A successful sear refreshes
  the complete item block—name, ratings, modifiers, attributes, resources, and
  tooltip—then republishes inventory and the still-open forge.
- The production client mirrors that payload into a dedicated two-column
  Framekit service. Up/Down and Enter plus pointer rows route the exact UUID;
  five-row paging keeps compact windows bounded, and modal presentation now
  suppresses the normal top HUD rather than allowing text to bleed through.
- Coverage spans direct protocol authority, forged-distance rejection, full
  item refresh, remote wire-to-model parsing, shared close behavior, and the
  production `town-vesselforge` scenario at 960x600 and 1366x768.
- Verification: the full native test and client-scenario gate passes. Final
  evidence is in
  `native/build/goal-captures-town-vesselforge-final/town-vesselforge-960x600.png`
  and `town-vesselforge-1366x768.png`; `frame-budget` remains 9.8 ms average at
  3440x1440 against the 40 ms ceiling.
- Remaining Vesselforge progression is deliberately honest: Bonds,
  attunement/evolution, and awakening are not claimed by this milestone and
  remain the next expansion of Tamar's service.

## 2026-09-03 - final Vesselforge implicits

- The last mechanics-bearing WIZARD form lines are authoritative in native:
  Atlatl grants 20% projectile range, Sling ignores half of monster Armour,
  and non-weapon Grips grant 8% increased attack speed. Projectile range
  extends the ranged targeting boundary; attack speed shortens primary recovery
  with bounded increased-speed scaling; armour penetration reduces the target's
  flat mitigation before later offensive multipliers resolve.
- Generated monsters now own level- and role-scaled Armour. Exact Armour,
  prevented damage, and penetration facts cross combat events, protocol
  snapshots, the remote client model, and shared presentation. Hover identifies
  Armour, while a successful bypass gets a distinct cyan `PIERCE` number.
- The gear pane reports speed, reach, projectile range, penetration, bleed,
  movement, and both wards in two fitted Framekit rows. Regression geometry
  proves both rows remain inside the 1366x768 pane without shrinking the type.
- Core comparisons prove the sixth-tile Atlatl shot, 350-to-324 ms Grips
  recovery, and a controlled 20 damage / 100 Armour Sling result improving from
  10 to 15. Protocol, scripted-session, shared-presentation, and production-
  client coverage carry those same facts end to end.
- The long Chronicle Gate-B driver's relic-recovery leg now recomputes a route
  through the known warren ribs from every authoritative position echo, removing
  the sticky-heading timeout exposed by the release gate.
- Verification: the full native test gate, complete Chronicle succession and
  relic-recovery journey, and every client scenario pass. Evidence is in
  `native/build/goal-captures-final-implicits-b/vesselforge-atlatl-range-1366x768.png`
  and `native/build/goal-captures-final-implicits-b/vesselforge-sling-pierce-1366x768.png`;
  `frame-budget` measured 9.8 ms average at 3440x1440 against the 40 ms ceiling.

## 2026-09-03 - Act III: The Deep Roads

Historical checkpoint: this twelve-commission stopping point is superseded by
the complete 23-point campaign section above.

- At this checkpoint the authoritative native campaign contained twelve
  commissions. The
  four-road covenant ends at eight points by opening Act III rather than
  prematurely marking the House complete or awarding an endgame key.
- The Deep Roads revisit tier-two Tin, Salt, Chalk, and Copper holdings in a
  fixed narrative order: The Quarry Saint's Canon, The Brine Widow's Tithe,
  The Bell Beneath Chalk, and The Cinder Judgment. Each requires exact road
  entry, its named Warden's death, and a living return to the Crossroads.
- Tier-two road bosses carry canonical identities in the authoritative monster
  roster: The Quarry Saint, The Brine Widow, The Ossuary Bell, and The Cinder
  Judge. A road node's tier now becomes its actual instance depth, activating
  the existing monster/loot scaling instead of relabeling a depth-one floor.
  At this checkpoint the final Cinder return sealed twelve quest points and
  awarded the first consumable charted tablet.
- Campaign snapshots now publish their total and current act number, title,
  completed count, and act size. The remote model validates and mirrors that
  contract; the Framekit Chronicle header shows act and 10/12-style progress.
- Chronicle history is bounded to nine visible deeds with a truthful earlier-
  deed count, keeping later arcs inside the left rail. A new Scion born into a
  sealed House inherits all twelve campaign points, completed commissions, and
  endgame access, so succession cannot strand the passive-tree budget.
- The full road test drives all eight road commissions, verifies every named
  boss in the live snapshot, re-admits between Deep Road commissions, and then
  creates a successor to prove House-wide continuity. The long Gate-B relic
  hunter was updated to follow a revealed Warden through `monster:moved`
  events after production locomotion made stale-position pursuit unreliable.
  The Crossroads fountain now restores both life and combat resource, so a
  recovered Scion can open each fresh delve with the existing War Cry skill.
- Verification: the full native test gate, complete Chronicle succession and
  relic-recovery journey, and every client scenario pass. Production evidence
  is in
  `native/build/goal-captures-deep-roads-d/deep-roads-campaign-1366x768.png`;
  `frame-budget` measured 9.8 ms average at 3440x1440 against the 40 ms ceiling.

## 2026-09-03 - active Vesselforge combat properties

- Five previously display-only WIZARD properties now resolve through the
  authoritative native game: Macuahuitl/Bloodgroove bleeding, Long Reach,
  Sandals/Surefoot movement speed, Riverblessed resistance, and Emberward
  resistance. Defensive resistances cap at 75%; offensive and utility totals
  retain the existing 100% cap.
- Bleeding is a non-stacking three-second wound with one-second physical ticks.
  Reapplication refreshes it and keeps the stronger wound. Macuahuitl's
  implicit grants guaranteed application, while Bloodgroove supplies the same
  chance through a rolled brand.
- Reach expands every authoritative skill, sweep, thrust, held-primary, and
  leash range check. Movement bonuses accelerate the server's sampled steps
  without bypassing collision. Generated dungeon ranged attackers deal Ember
  damage; marsh and grove attackers deal River damage, with mitigation facts
  carried on each hit.
- The complete item-derived chain is live: item and wear aggregation, movement
  and combat resolution, protocol totals and monster snapshots, remote model,
  shared presentation, and Framekit. Selected gear shows active forge lines and
  a compact worn-total summary instead of calling these properties dormant.
- Bleed application, periodic damage, and persistent wound state have separate
  crimson treatments. Monster hover identifies live bleeding and River/Ember
  attack channels so resistance choices remain inspectable during play.
- Core, protocol, scripted-session, shared-presentation, and production-client
  coverage prove the full path. Evidence is in
  `native/build/goal-captures-forge-affixes-b/vesselforge-active-properties-1366x768.png`.
- Verification: the full native test gate, complete Chronicle succession and
  relic-recovery journey, and every client scenario pass. `frame-budget`
  measured 9.7 ms average at 3440x1440 against the 40 ms ceiling.

## 2026-09-03 - obstacle-aware monster locomotion

- The production server tick now moves nearby monsters instead of leaving the
  generated roster rooted at spawn. A deterministic bounded breadth-first
  search routes cardinal steps around authored walls, other living actors,
  the Scion, and both stair tiles.
- Melee foes and Wardens close to their attack bands, ranged foes advance to
  casting distance or retreat when crowded, and support foes reform within
  mend range of the nearest wounded ally. Ranged units must finish opening
  minimum space before beginning a volley.
- Every accepted step is published as `monster:moved` with exact destination,
  role, and 400 ms duration. The remote client mirrors those facts and applies
  smoothstep interpolation only while painting; reconnect and scene-scale
  jumps snap safely to the latest authoritative location.
- A finisher-cancelled volley now also emits `monster:interrupted`, clearing
  its warning immediately instead of leaving a harmless stale reticle.
- Core replay coverage proves deterministic movement, walkability, melee
  pursuit, and ranged spacing. Protocol, scripted-session, and production GDI
  coverage prove movement/cancellation wire parity and mid-step rendering.
- Verification: the full native test gate, complete Chronicle succession and
  relic-recovery journey, and every client scenario pass. Production evidence
  is in
  `native/build/goal-captures-locomotion-b/monster-pressure-roles-1366x768.png`;
  `frame-budget` measured 9.8 ms average at 3440x1440 against the 40 ms ceiling.

## 2026-09-03 - authoritative monster pressure roles

- Generated melee, ranged, and buffer identities now have distinct production
  behavior instead of cosmetic role labels. Melee units retain close-contact
  pressure; ranged units engage from six tiles and sample a target area for an
  800 ms volley; buffers mend the most-injured non-boss ally within five tiles.
- Ranged resolution checks the sampled destination, so leaving its one-tile
  area is a real server-side dodge. A third-beat melee finisher interrupts an
  in-flight volley, while the existing boss stagger immunity remains intact.
- `monster:telegraph` now reaches the remote presentation with its exact tile,
  radius, and duration. `monster:healed` carries source, recipient, amount, and
  resulting health; clients mirror those facts without computing a heal.
- The production GDI presentation distinguishes violet volley reticles from
  red boss/melee warnings and renders support mends as green rings, a cross,
  and a positive number. Common ranged attackers no longer become visually
  elite merely because their hit reached the player.
- Core, protocol, scripted-session, shared-presentation, and production-client
  scenarios cover dodges, hits, support targeting, finisher interruption, and
  exact wire-to-VFX parity. Evidence is written as
  `native/build/goal-captures-roles-b/monster-pressure-roles-1366x768.png`.
- Verification: the full native test gate, complete Chronicle succession and
  relic-recovery journey, and every client scenario pass. `frame-budget`
  measured 10.4 ms average at 3440x1440 against the 40 ms ceiling.

## 2026-09-02 - authoritative three-beat melee cadence

- The production remote combat path now resolves primary attacks as a
  server-owned three-beat cadence: Cut at 100% damage, Return at 115%, and a
  160% Finisher. The first two beats recover in 350 ms; the finisher recovers
  in 520 ms and staggers non-boss retaliation for 700 ms.
- A 900 ms continuation window permits pack-to-pack chaining. Letting it
  expire or using a named skill restarts the primary sequence at beat one.
  Bosses receive the finisher damage but remain immune to its stagger.
- `combat:hit`, `player:combat-state`, and reconnect snapshots carry the
  resolved step/window/stagger. Held-primary server ticks publish combat state
  immediately, so the remote HUD never has to predict the next beat.
- The Framekit quickbar mirrors the active step with three pips and changes
  its primary caption for Return, Finisher, and restart. A third-beat hit has
  a distinct verdigris impact ring, cross flare, target flash, and enlarged
  damage number independent of critical-hit styling.
- Core, protocol, scripted-session, shared-presentation, and production-GDI
  scenario coverage prove damage coefficients, recovery, non-boss stagger,
  reset rules, wire/model parity, and the unique finisher treatment. Evidence:
  `native/build/goal-captures-combo-b/combat-cadence-finisher-1366x768.png`.
- Verification: the full native test gate, complete Chronicle succession and
  relic-recovery journey, and every client scenario pass. `frame-budget`
  measured 9.8 ms average at 3440x1440 against the 40 ms ceiling.

## 2026-09-02 - House-wide Wayfinder Mastery endgame

- The consumable tablet loop now feeds a finite 64-objective mastery board:
  Barrow, Reeds, Crown, and Thorns each have one first-clear objective at
  tiers 1-16. Tablet payloads carry their canonical family and objective key.
- Seal-Bound Warden kills still count every completed expedition, but only a
  first family-and-tier clear grants mastery and tier-scaled House renown.
  Repeat clears cannot duplicate either reward.
- Mastery is House-persistent, validates imported Chronicle keys against the
  64 canonical objectives, and restores for later Scions. Every two mastery
  objectives add one percentage point to the next-tablet ascent chance, from
  35% to a bounded 65%, giving broad completion a durable sustain reward.
- The authoritative endgame snapshot exposes mastery, highest tier, ascent
  chance, active first-clear status, and the mastered objective keys. The
  Framekit Chronicle journal becomes a Wayfinder's Ledger after campaign
  completion, with four readable 16-tier rows; the tablet pane previews
  `NEW MASTERY` versus `MASTERED` before consumption.
- Coverage proves invalid/duplicate Chronicle keys are discarded, first
  clears persist, repeat clears do not duplicate rewards, and the entire
  server-to-client-to-presentation mirror carries the new contract.
- Verification: full native tests and all client scenarios pass; `frame-budget`
  measured 9.7 ms average at 3440x1440. Evidence:
  `native/build/goal-captures-mastery-b/endgame-mastery-board-1366x768.png`
  and `native/build/goal-captures-mastery-b/endgame-tablet-ui-1366x768.png`.

## 2026-09-02 — four-road campaign act + persistent world web

- The native campaign now has eight commissions. Oath of Tin, The Salt
  Reckoning, The Chalk Vigil, and The Copper Testament each bind entry,
  generated Warden defeat, and return to the exact procedural road.
- Finishing Copper seals the campaign at eight quest points and awards the
  first one-use charted tablet. The existing endgame loop remains House-wide.
- Cleared world-web nodes now persist on the House Chronicle and restore across
  Scion admissions. Re-entering a cleared holding advances the relevant rite
  without respawning its Warden.
- `world:zone:enter` now rejects barred deeper nodes unless the authoritative
  parent clear exists. Party returns use the same campaign/extraction path as
  solo returns.
- Coverage drives the complete Tin → Salt → Chalk → Copper act, validates
  renown/reward totals, reconnect restoration, chart state, barred-node denial,
  and endgame unlock.
- Verification: full native tests and all client scenarios pass; `frame-budget`
  measured 9.7 ms average at 3440x1440. Evidence:
  `native/build/goal-captures-roads-b/campaign-journal-1366x768.png`.

## 2026-09-02 — authoritative native campaign journal + Scion checkpoints

- Native quest snapshots now publish presentation-safe titles, commissioners,
  summaries, objective copy/progress, rewards, completed deeds, quest points,
  House renown, and campaign completion. Server-only objective criteria remain
  private.
- The remote client fail-closes malformed campaign snapshots and mirrors valid
  state from admission, `dev:state`, and live `quest:update` envelopes.
- `J` opens a centered Framekit Chronicle Commissions journal. It shows the
  current rite, prior deeds, reward, points, and renown; town HUD tracking uses
  the same authoritative objective. Escape closes the journal before exiting.
- Partial campaign progress is stored on the living Scion's Chronicle record
  and restored on later admission. House renown and House-wide campaign
  completion restore with it; a new Scion begins a fresh personal checkpoint
  unless the House has already completed the campaign.
- Verification: full native tests and all client scenarios pass; `frame-budget`
  measured 9.9 ms average at 3440x1440. Evidence:
  `native/build/goal-captures-campaign-b/campaign-journal-1366x768.png`.

## 2026-09-02 — tactical map modes and persistent navigation preferences

- The native HUD now has two production map reads: the existing compact
  corner compass and a translucent `Tab` tactical chart. The expanded chart
  uses the authoritative scene walkability grid to draw route topology under
  the live Scion, foe/elite, townsfolk, scenery, and extraction markers; it is
  not a client-authored destination or a scaled screenshot of the world.
- The large chart uses the WIZARD Framekit nine-slice plate with opacity-aware
  compositing and the shared Verdigris fallback skin. It has a route-aware
  heading, an on-plate legend, and discoverable controls. Five zoom levels are
  available through the wheel or brackets, five transparency levels through
  minus/equal, and `Shift+M` moves the compact map between the left and right
  rails. Those three preferences persist in the user's local client settings;
  the broad overlay itself always starts closed.
- Opening the chart dismisses narrower gear/character/tree/dialogue panes,
  world hover cards stay suppressed beneath it, and Escape closes the chart
  before requesting application exit. Right-rail placement participates in
  the top-HUD and telegraph safe-zone planners; when gear needs that rail, the
  compact map temporarily yields left without changing the saved preference.
- The new `tactical-map` production scenario proves topology, controls,
  clamping, pane dismissal, right-side placement, and Escape ordering, and
  emits `tactical-map-overlay-1366x768.png`. The full native gate passed:
  denylist, every core/networking/camera/session/presentation/audio suite, the
  complete Chronicle succession/relic journey, and all client scenarios. The
  3440x1440 frame-budget result was 9.9 ms average against the 40 ms ceiling.

## 2026-09-02 — Crossroads social hub + first House investment

- The production native town now uses the accepted owner-demo Crossroads
  roster: Aldwyn, Ludovicus, Selene, and Rhea at their seed-authored
  positions. Stable NPC keys, roles, services, actions, and examine copy
  cross the server snapshot into the presentation model; the retired Mara
  placeholder is gone. Ludovicus' Road Iron Yard and Selene's Rite Vault
  open distinct authoritative merchant fronts.
- Every townsfolk interaction can now open an authoritative dialogue screen.
  The server owns reach checks, quest-aware copy, service options, and House
  eligibility; the client mirrors those values into a keyboard/mouse
  Framekit modal with wrapped conversation text and the shared Escape
  contract. Rhea is surfaced as a conversation first, with banking inside
  her service pane rather than hiding the coffer behind the bank shortcut.
- Clearing the first floor opens one irreversible House founding choice at
  Rhea. `scion_gear` creates real tier-one Vesselforge-native named gear for
  the current Scion; `house_production` returns 5 gold to the House treasury
  after every later floor clear. Choice, eligibility, reward state, tier,
  yield, and treasury persist on the Chronicle House and restore on later
  Scion admissions. The existing dawn purse and wagon spending now persist
  through that same ledger boundary.
- Forged remote actions cannot choose from outside Rhea's reach, select
  before the first clear, or replace a sealed choice. Protocol, loopback
  mirror, and presentation tests cover the roster, dialogue verbs, both
  rewards, House persistence, successor restoration, and close behavior.
  `town-social-hub` emits a dedicated 1366x768 Framekit capture and verifies
  all three actions remain within the compact viewport.
- Full native gate passed: denylist, all core/networking/camera/session/
  presentation/audio suites, the long Chronicle succession/relic journey,
  and every client scenario. The 3440x1440 frame-budget result was 9.6 ms
  average against the 40 ms ceiling.

## 2026-09-02 — authoritative remote combat actions

- The native client's `skill` payload and the legacy/browser `skillId`
  payload now resolve through one canonical server vocabulary. Melee,
  Thrust, Sweep, War Cry, and Dash no longer fall through to one cosmetically
  renamed primary attack.
- Melee retains click-to-attack cadence at the shared three-tile contact
  scale. Thrust is a discrete forward five-tile strike with 1.3x damage and
  a 10-resource cost; Sweep is a discrete three-tile multi-target attack
  with 0.75x per-target damage, a 15-resource cost, and a longer cooldown.
  Rejected input cannot reset the server clock, spend resource, or create an
  extra hit.
- War Cry is now an authoritative timed self-buff: it spends 20 resource,
  adds the shared +4 attack bonus for its shared 20-tick window, emits apply
  and expiry beats, and never deals disguised attack damage. Dash advances
  through ordinary collision and stair rules and never enters combat.
- Resource regeneration, cooldown ticks, and War Cry duration now cross the
  protocol into the remote model. The Framekit quickbar and resource orb
  therefore show server truth immediately through `player:combat-state`,
  while confirmed Sweep and War Cry events select their distinct VFX.
- Focused protocol tests cover the native-client field name, thrust-only
  range, exact skill identity, anti-spam, resource costs, multi-body Sweep,
  War Cry's real damage delta, and damage-free dash. The full native gate
  passed, including the ordinary-combat Chronicle succession/relic journey
  and every visual scenario; the 3440x1440 frame-budget result was 9.7 ms
  average against the 40 ms ceiling.

## 2026-09-02 — owner-authored House and Scion founding

- The Chronicles front door no longer silently derives permanent lineage
  names. `F` and `C` open a Framekit-backed naming ceremony with real Win32
  character input, a visible caret, Backspace editing, Enter confirmation,
  and Escape cancellation that never exits the game.
- Empty confirmation preserves the existing deterministic account/ordinal
  fallback. Typed names are normalized, limited to 28 characters, and a
  too-short name remains in the modal with an actionable error. The shortcut
  character that opened the ceremony is explicitly swallowed rather than
  leaking into the field.
- House names now receive exactly one presentation prefix across the
  Chronicle roster and expedition identity chip. A completed House also
  exposes its charted-road unlock and authoritative clear count on the front
  door.
- `chronicles-gate-b` now drives the production key/character path, names
  House Emberwake and Scion Ilyra across the real network session, checks
  cancel and invalid-input behavior, and continues through oath, death,
  succession, relic recovery, and reconnect. It emits a dedicated 960x600
  founding capture through the isolated capture root.
- Full native gate passed: denylist, all core/network/session/presentation/
  audio suites, the expanded Chronicle journey, and every client scenario.
  The 3440x1440 frame-budget result remained 9.8 ms average against the
  40 ms ceiling.

## 2026-09-02 — consumable charted-tablet endgame loop

- Campaign completion now unlocks a server-authoritative House endgame and
  awards its first charted tablet. Four tablet families roll tiers 1-16 plus
  two distinct risk/reward clauses that alter monster level, population,
  life, damage, and goods found; the exact roll survives inventory refresh
  and reconnect payloads.
- Breaking a tablet in town consumes that exact UUID and opens its one-use
  themed expedition. Invalid or pre-unlock attempts consume nothing. The
  Seal-Bound Warden closes the run, advances the House clear count once, and
  drops a same-tier or next-tier tablet through the ordinary loot/pickup path.
  Extraction keeps that next tablet usable while banking ordinary spoils.
- Chronicle Houses persist campaign unlock and completed-map count, so later
  Scions inherit the endgame state. Active expedition tuning is explicitly
  cleared on return and cannot leak into campaign roads.
- The native gear pane now renders a tier seal, goods bonus, both rolled
  clauses, and concise map-specific controls; the town/expedition objective
  strip mirrors unlocked, active, and cleared state. The
  `endgame-tablet-ui` scenario emits a 1366x768 capture and asserts its footer
  remains inside the pane.
- Full native gate passed: denylist, core/networking/camera/session/
  presentation/audio suites and every client scenario. The 3440x1440
  frame-budget result was 9.8 ms average against the 40 ms ceiling.

## 2026-09-02 — authoritative HUD information + shared dark-metal skin

- Server snapshots now publish the authoritative current-level combat XP
  span. The client preserves explicit absence for older sessions, normalizes
  only that supplied span, and renders a thin segmented XP strip between the
  vital orbs without reimplementing the progression curve.
- Monster display names now survive model-to-presentation sync. Pointer hover
  exposes compact monster life/role, loot pickup, and NPC interaction cards;
  trade, gear, character, and passive-tree panes suppress world tooltips.
- `ui_skin.hpp` now carries the shared dark bronze, ruby, sapphire, and ledger
  gold panel language plus the reusable progress-bar primitive. A live
  3440x1440 expedition capture confirmed the bottom strip stays clear of the
  quickbar and orbs.
- Full native gate passed: denylist, core/networking/camera/session/
  presentation/audio suites and every client scenario, including the new
  `hud-information` contract. The 3440x1440 frame-budget result was 9.6 ms
  average against the 40 ms ceiling.
- Work is local on `codex/goal-aaa-systems` in
  `C:\Users\Alex\Documents\ChatGPT\verdigris-goal`; it includes the twenty
  coherent native commits recovered read-only from Claude's architect
  checkout. Nothing has been pushed.

## 2026-09-01 — vector art era + four playable themed roads

- vector_art.hpp: procedural animated art replaces the raster world set.
  Humanoid rig (walk/breathe/attack, held tools), lurker/wight/beast/
  ghast/totem monster rigs dispatched by theme+role, swaying trees,
  fountain, stalls, wagon, gate arches, per-theme terrain tiles painted
  into the floor cache, themed masonry walls. Framekit pane chrome and
  item art remain raster (WIZARD deliverables). frame-budget ~10-13 ms.
- Server: per-theme named monster roster (melee/ranged/buffer ids), and
  'theme' rides dev:state.
- Chart pane over open:screen 'chart': town gate tiles open road charts;
  Enter/click sets out via world:zone:enter. Salt/chalk/copper roads and
  the marsh/grove/crypt/wilds themes are reachable in play for the first
  time. Fixed the open:screen parser (payload is top-level, not nested -
  shop/bank panes were silently dead too).
- Live-verified: salt gate -> Rushweir marsh (murk tiles, pools, Mire
  Ghast in elite gold). Owner should feel-check walk/attack animation.

## 2026-08-31 (night) — 55 fps, monsters fight back visibly, first-floor balance

- Perf: floor cache (BitBlt except on tile-boundary crossings), persistent
  back buffer (was a 19 MB alloc/free per frame), cached GDI+ HUD chrome
  (premultiplied layers; orb liquid at 21 levels). Live: paint 21.4 ->
  13.1 ms, fps 43 -> 55 at 3440x1440. F3 shows floor/world/hud section ms.
- Monster body language (presentation-only, event/position-derived):
  telegraph windup lean, landed-strike lunge, mirror toward the player.
- Core balance (owner ruling): pack first strikes arm a staggered
  400-1300 ms windup instead of a same-millisecond burst; contact damage
  2 + level (was 4 + level*2). Journey harness camps for its first hit.

## 2026-08-31 (later) — pacing rework, assets everywhere, audio voiced

- 20 FPS was structural: one 50 ms timer drove simulation AND rendering.
  Now a 15 ms frame timer with a 50 ms fixed-tick accumulator (wire
  cadence preserved), dt-correct camera smoothing, no input-driven
  repaints. Live F3 fps counter; ~30-50 fps at 3440x1440 measured.
- Walls ride the wire (dev:state includeMap, fetched once per scene) and
  draw as raised cut stone. Loot renders as category glyphs; NPCs are
  vector silhouettes with role rings; strike lunge animates the body.
- Asset-path escaping bug had silently disabled the whole WIZARD pack;
  fixed (forward slashes), F3 now reports framekit/item-art/scenery
  state. Town landmarks anchored on server contract positions.
- TASK-0157 audio finally has a device: waveOut synth sink (six-handle
  pool, fail-closed without a device), fed from the remote event stream
  at the fixed tick. M mutes. Owner has not yet confirmed feel/sound.

## 2026-08-31 — perf fix + panes: loot, inventory, character, tree

- Move+attack stutter fixed (`a9944523`): trivial input handlers (the
  WM_MOUSEMOVE per-event sync/invalidate starved lowest-priority
  WM_PAINT/WM_TIMER), viewport-clipped floor tiling, capped predicted
  swing effects. Reproduced as a 198k-event/3s message flood: 164 ms
  frames -> 21.7 ms. `--scenario all` now carries a `frame-budget` gate
  (20 real 32bpp frames at 3440x1440, <40 ms average); F3 shows live
  paint ms.
- `883d642e`: loot draws at authoritative groundItems positions (per-uuid
  fan for same-tile stacks) and X picks up the nearest real uuid (the
  server ignores empty uuids — pickup previously did nothing). The
  vendored WIZARD framekit pack is finally consumed: nine-slice
  panel/slot chrome + item art in the inventory pane (I); new character
  sheet (C) with server-derived attributes; clickable passive-tree pane
  (P) over the authoritative passiveTree mirror (allocation -> 
  player:skilltree:save; verified live, +2 INT round-trip); trade/bank
  panes over open:screen. Pane interiors scale with hud_scale.
- AGENTS.md now carries the binding native presentation gate; agents
  capture the live window with `native/tools/capture-window.ps1`.

## 2026-08-30 — owner-feedback pass 2: presentation leaves the skeleton

- LMB now routes through `dispatch_skill`, so the primary attack draws the
  same instant facing-oriented swing arc as Q/E/R (it previously had no
  animation at all).
- Camera snaps instead of panning the whole map on scene loads (follow lerp
  is unchanged in play; a gap over one arena half-extent snaps).
- The client starts borderless windowed-fullscreen (WS_POPUP at the primary
  monitor size); F11 toggles back to a 1280x800 movable window.
- New `native/client/ui_skin.hpp`: GDI+ skin layer (rounded gradient panels
  with shadows, glass vital orbs, sunken quickbar slots, chips, Segoe/Georgia
  type ramp). All HUD chrome + the Chronicles front door render through it.
- Resolution scaling: `hud_scale(height)` (integer; 1 at the shipped test
  resolutions, 2 at 1440p) sizes the shared HUD geometry, fonts, minimap,
  orbs, quickbar, connection chip; camera zoom grows with window height so
  the world keeps its on-screen scale. Toast anchors above the quickbar.
- All suites green (`native/build.ps1 -RunTests`, `--scenario all`,
  denylist). Verified live at 3440x1440 via window captures.

## 2026-08-22 — shipped for cloud/other harnesses

- Program tip `bb454c3c` on `codex/native-reconstitution` shipped via PR #58.
  Protected `master` is `2d3e92a5`.
- TASK-0101 and TASK-0161 are ACCEPTED/INTEGRATED. Combined native G6 with
  `-CaptureRoot` passed (`COMBINED-EXIT=0`).
- TASK-0108 is READY (readable ranged combat, ports 7280-7299). Exact base
  `76368466`. Do not own `session_tests.cpp` (TASK-0162).
- PC Codex Sol is retired. No OpenCode writer is assumed. Owner launches
  workers on other harnesses from this tip. Standalone orchestration `main`
  remains Mac-owned.

## 2026-08-22 — Cursor successor + TASK-0101/0161 accepted

- Codex Sol retired; Cursor successor acknowledged at `5c62c904`.
- TASK-0101 revision 1 (`a742355d`) ACCEPTED (`34ff3137`) and integrated
  (`bdecf037`).
- TASK-0161 (`9f004d2a`) independently ACCEPTED and integrated (`76368466`).
  Combined program G6 passed on that implementation tip.
- TASK-0108 is READY from W1 with `session_tests.cpp` excluded (TASK-0162).

## 2026-08-21 — PC single-lane Ox Alpha surge runway

- Program truth at sweep start: `d2423873`; `origin/master` and
  `origin/codex/native-reconstitution` matched, latest exact-SHA CI was green,
  and no PR, active claim, REVIEW_REQUESTED, or REVISE transition existed.
- D-126 registers only `ox-pc-a` (Windows, ports 6620-6639). The stopped b/c
  tabs shared one OpenCode project, made no claim/write, and are explicitly not
  Verdigris lanes or incidents.
- `RUN_STATUS.md` now exposes 30 effective pairwise path-disjoint READY packets
  plus 18 DRAFT successors. `PROGRAM_GRAPH.md` carries terminal T1-T8 proof and the deeper journey,
  presentation, renderer, campaign, combat, skill, monster, item, progression,
  persistence, replay, performance, tooling, packaging, and polish graph.
- Initial one-at-a-time route: TASK-0081 Gate B wire-contract freeze. The
  isolated worktree now exists at
  `Z:\Code\.worktrees\verdigris\ox-pc-a` on
  `codex/TASK-0081-gate-b-wire-contract-ox-pc-a` at base `7f271691`. Its local
  ignored `START_HERE_OX_PC_A.md` carries the complete claim/implementation/
  evidence/push/continuation packet; the architect did not claim or write
  STATUS/REPORT.
- Recurring supervision is active through Codex app heartbeat
  `verdigris-surge-supervisor` every 15 minutes. It resumes this architect
  task, scans before action, reviews/integrates/restocks transitions, and
  suppresses unchanged-state noise.
- Owner-only decisions are batched under `orchestration/owner-input/`; none
  blocks TASK-0081. This milestone changes coordination only, not gameplay.

## 2026-08-20 — TASK-0070 reference scenes Stage 1 (cursor, REVIEW_REQUESTED)

- Worker `codex/TASK-0070-reference-scenes-cursor` off `27d2be62`.
  `verdigris_client.exe --reference-scene all` writes 10 PNGs (1920x1080 and
  1366x768) and 5 render-list JSON dumps. Two-run JSON must match.
- Gates: `build.ps1 -RunTests` green. Architect eyeballs one scene per
  resolution.

## 2026-08-20 — TASK-0069 remote reconnect/retry (cursor, REVIEW_REQUESTED)

- Worker `codex/TASK-0069-remote-reconnect-cursor` off `1f45eb33`. Unexpected
  drop enters `Retrying` (1s/2s/4s, three attempts), re-logs the same guest,
  and resumes from the login snapshot. `player:session-replaced` stays
  terminal `Disconnected`.
- Gates: `build.ps1 -RunTests` green (reconnect resume + replaced no-retry).

## 2026-08-20 — TASK-0064 remote presentation unify (cursor, REVIEW_REQUESTED)

- Worker `codex/TASK-0064-remote-presentation-unify-cursor` off program tip
  `5c41a048`. `--remote` uses the local `paint_scene` pipeline (billboards,
  FX, HUD, camera2d); the 0061 debug painter is gone. No Simulation in
  remote mode.
- Gates: `build.ps1 -RunTests -RunClientScenarios` green, including new
  `remote-render-list` (Monster/Swing/Drop via paint_scene) and session
  `render-list` ops. Architect still needs to play `--remote` and rescore
  Gate A (no zeroes, ≥9/12).
- Play: N enters tin route (E is Sweep); X take-underfoot; walk stairs to
  extract. Monster/loot positions are inferred until 0063 snapshots.

## 2026-08-15 (latest) — Orchestration program active

- The program is now coordinated through `orchestration/` (protocol, state,
  decisions, task specs). Claude/Fable is architect+reviewer; the Codex
  coordinator with Luna workers implements. Read
  `orchestration/PROTOCOL.md` first.
- The current coordinator snapshot is indexed in
  [`RECONSTITUTION_STATUS.md`](RECONSTITUTION_STATUS.md), including the
  original checklist, WIZARD seams, review-ready tasks, blocked ownership
  questions, and current gate evidence.
- Focused WIZARD seam verification is recorded in
  [`WIZARD_INTEGRATION_VERIFICATION.md`](WIZARD_INTEGRATION_VERIFICATION.md):
  Orbs, inventory/Brands & Bonds, and Cartographer/map tests pass 73/73.
- Historical Delaford-to-Verdigris coverage is mapped in
  [`DELAFORD_COVERAGE_MATRIX.md`](DELAFORD_COVERAGE_MATRIX.md), with PvP and
  resource skills explicitly deferred pending product authority.
- The checklist gap audit is maintained in
  [`VERDIGRIS_GAP_AUDIT.md`](VERDIGRIS_GAP_AUDIT.md), including evidence,
  parity boundaries, and unresolved owner decisions.
- Wave 1 READY: TASK-0001 (native Legends records), TASK-0002 (build/CI
  hardening), TASK-0003 (slice verification harness). DRAFT: TASK-0004
  (client control pass per decision D-007), TASK-0005 (legacy audit).
- `prototypes/founding-slice/` is a committed, verified browser feel-lab
  ("Founding of a House"): serve the folder statically and open
  `index.html`, or rebuild via `node build.mjs`. It answers camera/combat/
  founding presentation questions and has no architectural authority.

## 2026-08-15 (later) — Milestone E first visual pass and a build fix

- Fixed a real Milestone D defect: `build.ps1` never defined
  `VERDIGRIS_NATIVE_WINDOWS`, so the "windowed" client silently compiled the
  console fallback and exited immediately. The define is now passed, the entry
  point is a standard `main()` (console subsystem keeps `--headless` stdout
  working), and `NOMINMAX` protects the std algorithms.
- `native/client/main.cpp` now carries the Milestone E visual foundation:
  an adjustable 2.5D camera (wheel zoom, PgUp/PgDn pitch, -/= perspective,
  Home reset), back-to-front depth sorting, contact shadows, a projected
  ground grid and extraction pad, billboard actors with enemy life bars,
  client-side loot scatter around kill sites, and procedural event-driven
  effects (swing arcs, impact flashes, death rings, dust, loot sparkles),
  all double-buffered.
- Verified end to end on Windows: MSVC build clean, the eleven core tests and
  the legacy denylist gate pass, `--headless` completes the extraction loop,
  and a driven input pass (PostMessage WASD/LMB/P/E/X against the live window,
  PrintWindow captures) shows the fight, drops, route unlocks, and extraction
  rendering correctly.

## 2026-08-15 — Milestones A–D first runnable slice

- Repository was synchronized to `origin/master` (`882dd81`) and work moved to
  `codex/native-reconstitution`.
- The browser game remains intact as historical reference.
- Product authority, open decisions, WIZARD Arcane Lattice evidence, legacy
  matrix/denylist, canonical `AGENTS.md`, sprint map, and C++ ADR are complete.
- `native/` is a dependency-free C++20 workspace with a fixed-step command/event
  simulation. It proves House and Scion creation, shared player/monster stats,
  movement, melee, damage/death, generated item and trophy drops, pickup/equip,
  extraction, loss and relic candidacy, House-owned route/branch progression,
  and an external seasonal objective/reward.
- `native/client/main.cpp` is a first runnable client: Win32 opens a native
  placeholder-shape window with WASD, mouse actions, Space dash, pickup/equip,
  and extraction controls; `--headless` provides a self-terminating smoke run.
- The core test executable covers the eleven requested architectural behaviors.
- Explicit `platform/`, `renderer/`, `networking/`, `persistence/`, and
  `content/` seams are documented without coupling them into the simulation.
- WIZARD integration intent is now explicit: orbs and Splash feed the renderer
  or menu presentation, Brands & Bonds/inventory feeds the item/UI path, and
  Cartographer is a candidate deterministic map-content adapter.
- The Claude demo source and 22 external PNG plates are now inventoried in
  [`CLAUDE_DEMO_ASSET_INTAKE.md`](CLAUDE_DEMO_ASSET_INTAKE.md). The binaries
  remain outside the checkout pending asset provenance, size, and packaging
  approval.
- The incomplete product checklist is recorded in
  [`VERDIGRIS_FEATURE_CHECKLIST.md`](../product/VERDIGRIS_FEATURE_CHECKLIST.md),
  with economy, Legends, branch-length, travel-risk, and UI-setting questions
  promoted into `OPEN_DECISIONS.md`.

## Next exact steps

1. Run a manual Win32 play pass and capture control/feel notes.
2. Decide whether the next renderer experiment uses the existing Canvas 2.5D
   reference ideas or a focused native billboard layer.
3. Keep networking, persistence, complete magic, and production art out of the
   core until the first playable loop has been evaluated.

## 2026-08-17 — Native parity wave N2 in progress

- TASK-0044 is claimed by Kimi Code in the external worktree
  `C:\Users\Alex\Documents\KimiWork\verdigris`; the WIP adds native world,
  movement, solo-zone, metadata, monster, and stair-return seams without
  changing the unchanged browser harness.
- Coordinator evidence is kept in
  [`TASK-0044 BASELINE.md`](../../orchestration/tasks/TASK-0044-native-protocol-n2/BASELINE.md)
  and the provisional
  [`REPORT.md`](../../orchestration/tasks/TASK-0044-native-protocol-n2/REPORT.md).
- The current worker WIP now passes the native denylist, core tests, and
  networking tests. A rebuilt server also passes unchanged `movement` and
  `zones` attach scenarios 2/2, including all six zones and saved-position
  restoration. Coordinator captures are preserved in the TASK-0044 folder;
  Kimi committed the six native files as `d476788`, so the task is now
  `REVIEW_REQUESTED` pending Fable's architect rerun and acceptance decision.

## 2026-08-17 — N3 combat parity boundary prepared

- The coordinator completed a read-only audit of the native/browser combat
  seams and recorded the executable handoff in
  [`N3_PARITY_IMPLEMENTATION_BRIEF.md`](N3_PARITY_IMPLEMENTATION_BRIEF.md).
  It maps the existing `player:move` and `player:skill:trigger` wire events
  into the deterministic core and lists the unchanged `combat` and
  `encounter-variety` acceptance matrix.
- The committed N2 server intentionally fails those two scenarios at the
  N2 boundary (18 monsters instead of the combat scenario's minimum 20, and
  no authored melee/ranged/buffer roles). The raw negative transcript is
  preserved in the TASK-0044 captures; no assertion was weakened and no N2
  source was changed.
- This is a handoff and evidence checkpoint, not an N3 claim. Native combat
  implementation must wait for Fable to issue a READY N3 task/spec.
- The requested authority choice and task issuance are tracked in
  [`QUESTION-0009`](../../orchestration/questions/QUESTION-0009-native-n3-authority-bridge.md);
  no source workaround is authorized while it remains open.

## 2026-08-17 — Current coordinator evidence refresh

- The disposable combined parity candidate `codex/integration-parity-candidate-v3`
  (`3636b729`) applies the complete TASK-0043 correction chain and TASK-0044
  `d476788` directly onto coordinator tip `9fea5668`.
- That candidate passes browser unit `122/779`, browser playtest `31/31`, the
  native denylist/core/networking/client gate, and the unchanged native attach
  matrix `quickstart`, `single-session`, `movement`, `zones` at `4/4`.
- TASK-0043 is now accepted and integrated at `1f470e3` on program tip
  `50ca60ad`; Fable's architect review `1f833081` records the default-mode
  31/31 gate and loopback-bind guidance. TASK-0044 is now accepted and
  integrated at `5b84f51e` on program tip `71b6b207`; review `5b2ee5b6`
  records the personal 4/4 rerun and accepts the documented N3 stubs.
- A dependency-complete rerun of that exact staged timing correction was
  recorded at coordinator commit `9e5d9fd8`: a fresh worktree installed
  dependencies with normal install scripts, then passed browser unit `122/779`
  and `PLAYTEST_PORT=6538 npm run playtest` at `31/31` (`loadMode:false`,
  p99/max event-loop lag `32.178/109.642ms`). This strengthens the evidence
  package and is retained as coordinator provenance for the accepted
  correction.
- The WIZARD seam rerun is recorded at coordinator commit `6cb7b366`:
  Orbs, Brands & Bonds/inventory, and Cartographer/map tests remain `73/73`;
  Verdigris Splash remains intentionally presentation/reference-only.

## 2026-08-17 — Current-tip N2 candidate refresh

- The pre-integration coordinator tip `27db1611` is intentionally still N1:
  its unchanged native attach baseline is 2/4 (`quickstart` and
  `single-session` pass; `movement` and `zones` time out).
- Disposable candidate `f602dab4` cherry-picks N2 worker commit `d476788`
  onto that tip and passes the native denylist/core/networking/client gate plus
  unchanged `quickstart`, `single-session`, `movement`, and `zones` at 4/4.
- This supersedes the earlier candidate reference for handoff purposes but
  accepted implementation and loopback-bind correction are integrated.

## 2026-08-18 — N3 combat parity review handoff

TASK-0045 is `REVIEW_REQUESTED` on
`codex/TASK-0045-native-protocol-n3` at `6d39565c`. The worker owns only
`native/**`; `playtest/**`, `server/**`, and `src/**` remain read-only. Native
denylist/core/networking gates, unchanged seven-scenario attach, combat unit
coverage, and the authentic telegraph-radius negative are captured. The
architect must rebuild and rerun the attach before acceptance; no N3 source is
integrated into the program branch yet.

## 2026-08-18 — Current-master playability evaluation review handoff

TASK-0046 is `REVIEW_REQUESTED` on
`codex/TASK-0046-playability-reevaluation` at `1de6e45b`, based on current
program tip `45846af7`. It owns only task-folder evidence. The report records
two approximately ten-minute arcs, first-minute page-context `window.ws.url`
proofs on disposable ports, and a new disposition/ranking. Guest produces
readable melee kills/XP/gold; the mortal-oath Chronicles arc remains blocked
at a visually present but mechanically silent opener. Architect review is
pending.

## 2026-08-20 — Server/rules parity COMPLETE (32/32 attach)

D-122 axis 1 is done: the unchanged 32-scenario playtest harness passes
against the native C++ server, verified twice consecutively on fresh
servers (PR #45, hotfix PR #46). Native gates (build + unit + session +
client scenarios) all green; session tests survive 8/8 under heavy CPU
load after the reader-thread join fix.

Load-bearing findings for successors:

- Target selection: `start_player_attack` now does a true nearest scan
  with direction-aware tie-breaks. The old scan silently locked onto
  the first spawned monster; anything combat-adjacent that "worked"
  before 08-20 may have depended on that bug.
- Session semantics mirror JS exactly (proven by probing the live JS
  server): one shared anonymous guest account; concurrent anonymous
  login REPLACES the old session; adoption rebuilds a fresh Player
  while loot/levels/bank/tree/quest-record persist; permadeath
  survives reconnect; the commission chain resets on scion admission
  only. Two quest-point counters exist on purpose: quests.questPoints
  (persistent chain record) vs top-level questPoints (live tree
  budget, resets per login).
- WebSocketServer::stop() now joins per-connection reader threads.
  Never revert to detach: a reader waking after `delete server`
  dereferences the freed object (the 08-20 Native CI segfault).
- Ops: kill servers with PowerShell Stop-Process, never Git Bash
  pkill (MSYS pkill cannot kill native Windows exes; a stale server
  produced a bogus 26/32).

Remaining axes: presentation deltas #3/#4 (surface density TASK-0078,
panels/typography unspecced), Gate B Chronicles client (TASK-0077).
