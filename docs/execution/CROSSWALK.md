# VG planning ID ↔ existing TASK packets

Draft 2026-09-05. Planning IDs do **not** become TASK numbers.
Disposition: `reuse` / `extend` / `verify` / `new` / `superseded`.
Do not spawn implementation work that duplicates TASK-0108 or the Owner
Demo chain.

The 200-ID table is `CROSSWALK_REGISTRY.md` (generated from ATOMIC_GOALS;
planning IDs stay DRAFT). This file keeps the do-not-duplicate rules and
Cursor evidence notes.

## Do not duplicate

| Existing | Disposition for VG work |
|---|---|
| TASK-0108 combat-depth-wave | Predecessor for readable combat; VG-ART-003/006 extend, never re-spec. Cursor 2026-09-06: local Telegraph ingest from JS `world:projectile` keys (`ingest-ranged-projectile-warning.hpp`, `ranged-warning` scenario). Core+wire stays Kimi `3b929637` on `origin/kimiwork/TASK-0108-ranged-rev3`. `native/client/remote_session.cpp` is now narrow-released for Kimi's remaining remote parse; do not re-spec core/wire. |
| TASK-0145, 0177, 0178, 0197, 0203, 0205–0207 Owner Demo | Journey/content/perf gates; VG-UI-006, VG-ART-008, VG-PERF-008, VG-GOV-007 extend |

## GOV

| VG | Disposition | Existing |
|---|---|---|
| VG-GOV-001 | new | `BASELINE.md` + `decisions/freeze-a-reproducible-baseline.md`; dual heads `486058f3` / `e7b65360` |
| VG-GOV-002 | new | `decisions/resolve-orchestration-precedence.md` — **owner stamp still required** |
| VG-GOV-003 | extend | TASK-0119 READY, TASK-0152 INTEGRATED, TASK-0206 AUTO_RELEASE; scorecard + `first-session-clarity` journey; feature counts cannot pass; Slay wardens stays off WASD/objective/Tin village/Life |
| VG-GOV-004 | extend | this file + `decisions/crosswalk-existing-task-packets.md`; TASK-0108 extend never re-spec; TASK-0095/0097 superseded |
| VG-GOV-005 | verify | TASK-0114 INTEGRATED; `docs/execution/decisions/choose-the-renderer-trial-boundary.md` — gpu-sample is not an engine port |
| VG-GOV-006 | extend | TASK-0018, 0056, 0148; `death-disconnect` — owner Carry open / No extract; disconnect cannot ack uncommitted extract |
| VG-GOV-007 | extend | Owner Demo content lots |
| VG-GOV-008 | new | pack `tools/roadmap.py` validate + unittest; evidence `docs/execution/evidence/VG-GOV-008.json` |

## UI / ART / GPU / PERF (Cursor-leaning)

| VG | Disposition | Existing |
|---|---|---|
| VG-UI-001 | extend | native Escape stack (`pane-stack`); owner Stack 2 / Escape closes; Stack 2 stays between the two panes; covering First Scion or gear cannot certify; absent tree hides seats; invented origin cannot pass; WASD cannot overlay the open tree; C or Esc closes stays in the sheet slot |
| VG-UI-002 | extend | TASK-0171 INTEGRATED, 0184 BRIDGE_PREP; `pack-drag`; reject cannot lose/duplicate/silent-equip; pack cells wrap Ember-edged axe; Pack place stays off WASD/LIFE; compare hint uses ASCII `|`; gear footer stays inside the pane |
| VG-UI-003 | extend | paper-doll + ack-only HUD (`equipment`); pending compare cannot gold-frame as equipped; compare plate stays off gear DEF/LVL; Ack only stays off WASD/LIFE; compare hint uses ASCII `|`; `progression-surface` owner Skill tree / No data yet (TASK folder cannot certify) |
| VG-UI-004 | extend | TASK-0156, 0159 INTEGRATED; `stat-explain` expandable ATK; dormant cannot fold into Attack; compact Sources uses Base/Gear; lowercase base/gear cannot certify; expanded sheet paints Conditional once; a duplicate Conditional cannot certify; Base Gear stays off the sheet/WASD/objective/Life; WASD stays off the C-key sheet; close hint stays in the slot |
| VG-UI-005 | extend | TASK-0076 INTEGRATED; 0178/0192/0203 Owner Demo; `route-map` zoom/opacity cannot reveal off-snapshot targets; Risk wardens stays off WASD/objective/Tin village/Life |
| VG-UI-006 | extend | TASK-0145/0177 + 0190/0197/0201 — **do not duplicate** |
| VG-UI-007 | extend | TASK-0159 INTEGRATED, 0118 READY, 0207; `hud-pane-readability` at 960/1366/3440 + tree/character keep-out + `hud-scale-floor` + live window; TASK-0159 folder capture cannot certify; clipped C or Esc closes cannot certify; clipped I or Esc closes gear footer cannot certify; clipped DEF/LVL cannot certify; Life left / Type floor stay off WASD/objective/Tin village/Life; VG-UI-006 Owner Demo not duplicated |
| VG-UI-008 | new | XInput tick path + glyphs (`pad-path`); not mouse emulation; Pad glyphs stays off WASD/objective/Tin village/Life |
| VG-ART-001/002/004 | extend | TASK-0141 INTEGRATED — village kit + collision-proxy ops (`kit-chunk`); not artist-local collision; Adult camera / Jointed warden / Uniform pan / Kit lock / Village kit stay off WASD/objective/Tin village/Life |
| VG-ART-003/006 | extend | TASK-0122 INTEGRATED; 0173/0174 READY; 0186/0187 BRIDGE_PREP — Cursor paints melee **poses** (VG-ART-003) and WarCry **weave labels** plus Phase A spawn/fade (VG-ART-006) without taking TASK-0173 models or re-speccing TASK-0108; Strike poses / Hit flash / War Cry weave stay off WASD/objective/Tin village/Life |
| VG-ART-005 | extend | TASK-0182 AUTO_RELEASE, 0184; `held-item` world attachment + `loot-to-bank` pickup-to-equip hold; paper-doll seat alone cannot pass; World hold / Unarmed first stay off WASD, the objective, Tin village, and Life |
| VG-ART-007/008 | extend | TASK-0206/0205/0207 — Owner Demo, do not duplicate |
| VG-GPU-001 | new | isolated software quad (`gpu-sample`); not a D3D-only window; Software quad stays off WASD/objective/Tin village/Life |
| VG-GPU-002 | new | render-list → packets (`gpu-packets`); handles cannot snapshot |
| VG-GPU-003 | new | versioned software bindings (`shader-bindings`); no runtime .hlsl path; Layout v1 stays off WASD/objective/Tin village/Life |
| VG-GPU-004 | new | session-connected packet present (`gpu-reference`); not a disconnected demo |
| VG-GPU-005 | new | Y-sort + Sweep overlay on the village gate (`grounding`); HUD or capture-black cannot certify; walls cannot erase warnings |
| VG-GPU-006 | new | moving bronze lantern pool on the village gate (`material-light`); HUD without a pool cannot certify; cannot wash out damage |
| VG-GPU-007 | new | BMP readback + provenance (`gpu-capture`); PNG R/B swap cannot certify; packet logs are not pixels |
| VG-GPU-008 | new | sample recreate/resize (`gpu-recover`); restored BMP stamped; Live buffers 1; leak cannot certify |
| VG-PERF-001/002/008 | extend | TASK-0152, 0207; 008 Owner Demo — do not duplicate |
| VG-PERF-003–006 | new / extend 0207 | GDI batch, envelope, loot labels, hitch warmup |
| VG-PERF-007 | new | 32-cycle present/effect/resize soak (`memory-soak`); short scene fails |

## MOVE (Cursor client lease only)

| VG | Disposition | Existing |
|---|---|---|
| VG-MOVE-005 | extend | TASK-0165 `input_focus.hpp` wired in `fixed_game_tick` (`pane-focus`); not a core movement rewrite |
| VG-MOVE-006 | new | isolated `bindings.v1` (`remap-binds`); owner Documents cannot be the test path. Full VG-SHIP-001 packager stays Kimi |
| VG-MOVE-008 | new | `input-latency` p50/p95 on the paint path; owner To present / Input paint; photon rejected. MOVE-007 buffering stays Kimi |
| VG-MOVE-001 | new | eight-way `player:move` names (`eight-way`); owner Eight-way / Up-left; vertical-only collapse fails |
| VG-MOVE-002 | new | held aim survives locomotion (`aim-hold`); owner Aim hold / Face east; core move still clobbers without the adapter |
| VG-MOVE-003–004, 007 | extend / new | Kimi lease for sim travel distance / dash sweep / action buffering |

## ACT (Cursor presentation bridge only)

| VG | Disposition | Existing |
|---|---|---|
| VG-ACT-007 | new | `attack-beat` event bridge; owner Attack beat / Anticipate; fabricated swing cannot mint. Not TASK-0108 |
| VG-ACT-005 | new | catalog-typed warning window (`telegraph-spec`) plus dodge-clear (`telegraph-dodge`); owner strip paints ticks+footprint and Dodge clear / Life holds; Warning windows / Dodge clear stay off WASD/objective/Tin village/Life; ms/50 and ghost hit cannot invent damage. Core ACT stays Kimi |
| VG-ACT-001–006, 008 | new / extend | Kimi lease (`native/src/**`) |

## WORLD (Cursor presentation dressing only)

| VG | Disposition | Existing |
|---|---|---|
| VG-WORLD-008 | new | versioned `dressing-pass`; topology hash ignores decoration. Core WORLD-001–007 stay Kimi |
| VG-WORLD-001–007 | extend / new | Kimi lease |

## ITEM (Cursor presentation filter only)

| VG | Disposition | Existing |
|---|---|---|
| VG-ITEM-006 | new | `loot-filter` nameplates; owner Hide trophies; cannot mutate sim ground. ITEM-001–005/007–008 stay Kimi |
| VG-ITEM-001–005, 007–008 | new | Kimi lease |

## BUILD / QA (Cursor presentation + pack evidence only)

| VG | Disposition | Existing |
|---|---|---|
| VG-BUILD-001 | new | three named slice fixtures on the character sheet (`build-fixtures`); tinted copies of melee fail. Core STAT/BUILD algebra stays Kimi |
| VG-BUILD-002–008 | new | Kimi lease |
| VG-QA-001 | new | `docs/execution/pack/tools/evidence_manifest.py`; a PNG with no hash/command cannot certify. `native/tests/**` stays Kimi |
| VG-QA-002 | new | `headless-contract`: sim AttackStarted → swing intent + `attack-anticipate`. A mocked PresentationEvent cannot prove the journey |

### VG-PERF-001 paint trace (GDI client, 2026-09-05)

Printed by `--scenario frame-budget` and F3. Not a GPU capture.

| Field | Meaning |
|---|---|
| machine | `SM_CXSCREEN`×`SM_CYSCREEN`, logical CPU count, Win32 |
| floor | `paint_ms_floor` — terrain + walls |
| world | `paint_ms_world` — actors, loot, effects |
| hud | `paint_ms_hud` — chrome after world |
| upload | `paint_ms_upload` — `BitBlt` to the window; 0 in headless scenarios |
| total | `last_paint_ms` — paint_scene (+ blit in the live present path) |
| net | n/a until Kimi exposes a session RTT on this lease |

Negative: a run that omits the machine line or the section fields cannot claim a portable budget. Bound stays 40 ms average at 3440×1440.

## Cursor in-flight (not a new TASK)

HUD chrome using web-client tokens, wizard orb plates, hover tooltips,
authoritative XP bar (`xp-meter`: empty strip cannot pass; local kills fill
the RS curve without a second core), route card under the minimap (client zoom `[`/`]`),
character-sheet attack source vs dormant conditional, backpack drag
(VG-UI-002), combat audio (VG-SOUND-003/004/005) with event-id
dedup, warning-priority steal, and one ambience layer per route.
Snapshot field `xp` remains reserved. VG-PERF-001: F3/`frame-budget`
prints named display, CPU count, and floor/world/hud/upload fields.
Owner strip paints Named machine / Paint fields after the timed loop.
Unnamed HW cannot certify. Evidence
`docs/execution/evidence/VG-PERF-001.json`. Bound stays 40 ms.
VG-PERF-005: loot nameplates cap at 12 nearest; Drop sprites stay.
Owner strip paints Nearest 12 / Drop stays. Cull pickup cannot certify.
Scenario `loot-label-budget`. Evidence
`docs/execution/evidence/VG-PERF-005.json`.
VG-PERF-003: effect/telegraph GDI pens and brushes are reused; scenario
`effect-batch` keeps Impact/Swing/Telegraph ops. Owner strip paints Reuse
pens / Keep warning. Drop FX cannot certify.
VG-PERF-004: one floor bitmap, GDI caps, effect cap 128 across resize
cycles (`resource-envelope`). Owner strip paints Cap 128 / One floor.
Grow FX cannot certify. core.cpp stays Kimi.
VG-PERF-006: `warm_combat_glyphs` before first strike; scenario
`hitch-warmup` prints cold/warm/prepared and fails if cold is omitted.
Owner strip paints Warm glyphs / Cold trace. Hide cold cannot certify.
VG-UI-007: tooltip titles use ink-on-panel contrast plus a triangular mark.
VG-ART-004: tin village kit (all five scenery kinds); solid pieces emit
`collision-proxy:*`; dressing gate does not. Owner strip paints Village
kit / Solid proxy and parks off WASD, the objective, Tin village, and
Life. Covering those combat surfaces cannot certify. A lollipop tree
cannot certify. Dwellings are
mudbrick/thatch huts; ruins are a broken wall with rubble; shrines are a
fountain; gates are two pillars with a lintel. Shrine and gate sit inside
the spawn capture. Scenario `kit-chunk`. Evidence
`docs/execution/evidence/VG-ART-004.json`.
VG-ART-006: WarCry family `vfx-weave:cast|travel|impact|cancel`; radius
capped vs screen-fill; telegraph remains. Owner strip paints War Cry weave
/ Travel and parks off WASD, the objective, Tin village, and Life.
Covering those combat surfaces cannot certify. Screen fill cannot
certify. Scenario `weave-vfx`. Phase A spawn beats: owner strip paints
Spawn once / Fade ttl; re-spawn cannot certify.
Scenario `animation-vfx-phase-a` pack capture in art-wave (TASK-0122 folder
cannot certify). TASK-0173 models stay Kimi. Evidence
`docs/execution/evidence/VG-ART-006.json`.
VG-UI-008: XInput sampled on the fixed tick; A strike / B dash / X take /
Y gear; hotplug in/out; mouse position cannot mint `pad:connected`.
Owner strip paints Pad glyphs / A strike and parks off WASD, the
objective, Tin village, and Life. Covering those combat surfaces cannot
certify. Mouse pad cannot certify. Scenario `pad-path`. Evidence
`docs/execution/evidence/VG-UI-008.json`.
VG-SOUND-002: CC0 provenance table for combat cues including swing windup
`attack-anticipate`. Live HUD paints Family combat / Anticipate CC0.
`unlicensed-preview` cannot ship. Scenario `legal-sounds`.
Evidence `docs/execution/evidence/VG-SOUND-002.json`.
VG-SOUND-008: explore/combat/recovery music coalesced per drain; unload
sets `music:none` and mutes the music bus so a leftover combat loop cannot
voice. Owner strip paints Theme Combat / Music none. Leftover loop cannot
certify. Scenario `music-phase`. Device mute still silences waveOut.
Evidence `docs/execution/evidence/VG-SOUND-008.json`.
VG-GPU-001: isolated software backend (64×64 textured quad + shutdown).
Owner strip paints Software quad / No D3D and parks off WASD, the
objective, Tin village, and Life. Covering those combat surfaces cannot
certify. Unknown GPU cannot certify. Not a Windows-only D3D proof.
Scenario `gpu-sample`. Evidence
`docs/execution/evidence/VG-GPU-001.json`.
VG-GPU-002: Telegraph draw class copied to handle-free packets. Owner
strip paints Handle-free / Telegraph class. Poisoned `backend_handle`
fails snapshot. Scenario `gpu-packets`. Evidence
`docs/execution/evidence/VG-GPU-002.json`.
VG-ART-001: in-game HUD names camera/proportion/palette/contrast. Owner
strip paints Adult camera / Bronze palette and parks off WASD, the
objective, Tin village, and Life. `first-fight` owner strip paints
Jointed warden / Snout claws and parks off WASD, the objective,
Tin village, and Life. Covering those combat surfaces cannot certify.
`zoom-invariance` owner strip paints Uniform pan / Zoom lock and parks
off WASD, the objective, Tin village, and Life.
`move-and-camera` owner strip paints Kit lock / Same delta and parks
off WASD, the objective, Tin village, and Life. An external
concept-art token cannot substitute, a skeleton `art: PNG billboards
loaded` chip cannot count as the composition sheet, a chibi (1/3) head
cannot pass as adult, a crate foe cannot certify, a free tile cannot
certify the camera contract, and a sliding kit cannot certify pan.
Scenarios `visual-target` / `first-fight` / `zoom-invariance` /
`move-and-camera`. Evidence `docs/execution/evidence/VG-ART-001.json`.
VG-ART-003: windup/active/recovery/cancel silhouettes on Strike poses.
Idle still cannot certify the family. Frame count alone cannot pass.
Owner strip parks off WASD, the objective, Tin village, and Life.
Covering those combat surfaces cannot certify.
`combat-juice` owner strip paints Hit flash / Number fade and parks off
WASD, the objective, Tin village, and Life. Covering those combat
surfaces cannot certify. Silent hit cannot certify readable contact.
Scenarios `attack-poses` / `combat-juice`. Evidence
`docs/execution/evidence/VG-ART-003.json`.
VG-ART-002: bronze/stone albedo+rim maps with SPDX CC0; magenta fill
cannot ship. Owner strip paints Bronze stone / Cooked CC0. Village kit
uses the family. Scenario `bronze-stone`. Evidence
`docs/execution/evidence/VG-ART-002.json`.
VG-GPU-003: layout v1 software program `software-albedo-rim-v1`. Wrong
backend or stale layout fails; failed load does not paint. Owner strip
paints Layout v1 / No source and parks off WASD, the objective, Tin
village, and Life. Covering those combat surfaces cannot certify. Stale
HLSL cannot certify. 64×64 BMP hash is unchanged. Scenario
`shader-bindings`. Evidence
`docs/execution/evidence/VG-GPU-003.json`.
VG-GPU-004: software present of live session packets (actors/world/effects/HUD).
Owner strip paints Live packets / Session present. A textured-quad demo with
no session cannot pass. Scenario `gpu-reference`. Evidence
`docs/execution/evidence/VG-GPU-004.json`.
VG-GPU-005: scenery/actors sort by world Y; telegraphs paint after that pass.
Owner strip paints Y-sort / Sweep disc. Sweep is a readable red disc on the
village gate; a HUD token, wall hide, or capture-black fill cannot certify.
Scenario `grounding`. Evidence `docs/execution/evidence/VG-GPU-005.json`.
VG-GPU-006: moving light on the cooked family; channel cap 220; damage chroma
cannot be washed white. Owner strip paints Lantern pool / Bronze light. Live
scene paints a bronze lantern pool at the village gate; a HUD token without
that pool cannot certify. Scenario `material-light`. Evidence
`docs/execution/evidence/VG-GPU-006.json`.
VG-GPU-007: BMP readback plus provenance sidecar of the painted tin-village
scene. Packet snapshot text is not pixel evidence. A PNG whose red/blue
channels are swapped versus the DIB cannot certify. `vital-orbs` cannot
stand in for the scene BMP. Scenario `gpu-capture`. Evidence
`docs/execution/evidence/VG-GPU-007.json`.
VG-GPU-008: recreate/resize/minimize-restore keep one live buffer; the
restored BMP carries an L-bracket survival mark (hash diverges from
`gpu-sample`). Live HUD paints Live buffers 1; leak cannot certify.
Failure surfaces `gpu-error:recreate`. Scenario `gpu-recover`. Evidence
`docs/execution/evidence/VG-GPU-008.json`.
VG-SOUND-001: software tone adapter plays a generated PCM burst and shuts
down. Live HUD paints Adapter software / Tone 440 Hz. A zero-duration cue
is not audible. Scenario `sound-adapter`. Evidence
`docs/execution/evidence/VG-SOUND-001.json`.
VG-SOUND-005: one ambience cue per route; live HUD paints Loop Tin village
wind. Owner strip paints Zone loop / Loop Tin village wind and parks off
WASD, the objective, Tin village, and Life. Covering those combat surfaces
cannot certify. A protocol `ambience:route` token or stacked reentry loops
cannot certify. Scenario `ambience-layer`. Evidence
`docs/execution/evidence/VG-SOUND-005.json`.
VG-SOUND-004: SFX cap 8; scion-lost stays voiced under twelve World
cosmetics. Owner strip paints Warning held; cosmetic x12 cannot certify.
Scenario `combat-audio`. Evidence
`docs/execution/evidence/VG-SOUND-004.json`.
VG-SOUND-006: mute cannot reset SFX/music volumes; zero SFX stays silent.
Live mixer paints persisted SFX/Music numbers while muted. Owner strip
paints Mixer prefs / SFX persist. A mute chip alone cannot certify.
Scenario `audio-prefs`. Evidence
`docs/execution/evidence/VG-SOUND-006.json`.
VG-UI-003: equip HUD is ack-only (`equip:ok` after ItemEquipped). Owner
strip paints Ack only / No pending. Pending compare cannot gold-frame as
equipped. Owner gear pane paints `Skill tree: no data yet`; PaneStat
keeps `TREE no authoritative data`. Scenario `equipment`. Absent
progression capture: owner strip paints Skill tree / No data yet; TREE
jargon cannot certify. Scenario `progression-surface` pack capture in
art-wave (TASK-0156 folder cannot certify). Absent P-key tree hides
seats (`tree-pane-960x600.png`, shared with VG-UI-001) and keeps WASD
off the pane. Gear footer I or Esc closes stays inside the pane. Compare
plate parks left of the pane; covering DEF/LVL cannot certify. Ack only
parks off WASD and gear LIFE/ATK. Compare hint is Enter equips |
U unequips. Evidence
`docs/execution/evidence/VG-UI-003.json`.
VG-PERF-007: 32 resize/effect cycles; floor bitmaps stay 1. Owner strip
paints 32 cycles / Cap holds. Short scene cannot certify. Scenario
`memory-soak`. Evidence `docs/execution/evidence/VG-PERF-007.json`.
VG-SOUND-007: mixer tape from mixed pack + elite; live HUD paints
Encounter mix / Hit + warning. An isolated preview cannot certify.
Scenario `dense-mix`. Evidence `docs/execution/evidence/VG-SOUND-007.json`.
VG-UI-001: Escape dismisses character then gear; bare Escape quits. Owner
strip paints Stack 2 / Escape closes in the world lane between the two
panes. Covering First Scion or gear cannot certify. Helper depth alone
cannot prove. Absent P-key tree paints No seats yet; an invented origin
seat cannot certify. Open tree keep-out relocates WASD; overlaying the
pane cannot certify. Open character sheet keep-out relocates WASD;
deleting the hint cannot certify. The C or Esc closes footer stays
inside the sheet slot; clipping it cannot certify. Scenario
`pane-stack`. Evidence
`docs/execution/evidence/VG-UI-001.json`.
VG-MOVE-005: TASK-0165 focus reducer gates WASD, combat, pickup, and pack
drag. Owner strip paints Focus gear / No buffer. Closing a pane cannot
release a held attack. Scenario `pane-focus`. Evidence
`docs/execution/evidence/VG-MOVE-005.json`.
VG-MOVE-006: versioned bindings persist under an isolated test profile.
Owner strip paints Isolated profile / Dash remap. Duplicate codes and
unknown devices fail on the HUD; owner Documents cannot be the write
target. Scenario `remap-binds`. Evidence
`docs/execution/evidence/VG-MOVE-006.json`.
VG-MOVE-008: input QPC to `paint_scene` present QPC. Owner strip paints
To present / Input paint. Photon cannot certify. p50/p95 on this
machine. Command dispatch time is not `input-latency:photon`. Scenario
`input-latency`. Evidence `docs/execution/evidence/VG-MOVE-008.json`.
VG-MOVE-001: eight-way encoder keeps both axes. Owner strip paints
Eight-way / Up-left. A vertical-only name cannot pass a diagonal.
Scenario `eight-way`. Evidence `docs/execution/evidence/VG-MOVE-001.json`.
VG-MOVE-002: `player:move` does not replace held aim; local tick
re-applies aim after move. Owner strip paints Aim hold / Face east.
Move facing cannot certify. Scenario `aim-hold`. Core `resolve_move` still
turns facing — that is the defect the adapter covers.
Evidence `docs/execution/evidence/VG-MOVE-002.json`.
VG-ACT-007: AttackStarted/DamageApplied/ActorDied drive
`attack-beat:*`. Owner strip paints Attack beat / Anticipate. A
fabricated swing cannot mint the beat. Scenario `attack-beat`. Core melee
resolution stays with Kimi. Evidence `docs/execution/evidence/VG-ACT-007.json`.
VG-SOUND-003: ordinary fight voices hit and death; the same event ID
cannot double-play. Owner strip paints Beats mapped / Hit once. Scenario
`attack-beat` capture `combat-beats`. Evidence
`docs/execution/evidence/VG-SOUND-003.json`.
VG-ACT-005: local ticks and remote `durationMs` share
`presentation_catalog().telegraph_ticks` and reach. `value/50` cannot
invent a longer window. Owner strip paints Thrust/Sweep ticks and
footprint; a protocol HUD token without that window cannot certify.
Expired/cancelled warnings leave no footprint. Owner strip paints
Warning windows and parks off WASD, the objective, Tin village, and
Life. `telegraph-dodge` owner strip paints Dodge clear / Life holds and
parks off WASD, the objective, Tin village, and Life. Covering those
combat surfaces cannot certify. Ghost hit cannot certify an avoided
sweep. Scenarios `telegraph-spec` / `telegraph-dodge`. Evidence
`docs/execution/evidence/VG-ACT-005.json`.
VG-WORLD-008: dressing-pass v1/v2 change decoration hash only.
Owner strip paints Dressing / Not solid. A tree visual cannot become an
unreported solid. Scenario `dressing-pass`. Evidence
`docs/execution/evidence/VG-WORLD-008.json`.
VG-ITEM-006: loot category facts and a presentation filter. Owner strip
paints Hide trophies; mutate ground cannot certify. Hiding trophies
cannot move ownership or drop tables. Scenario `loot-filter`.
Evidence `docs/execution/evidence/VG-ITEM-006.json`.
VG-BUILD-001: reach/pressure/magic fixtures list tactics, weakness, gear,
and encounter answers. Owner strip paints Three slices / Reach pike.
Three melee clones that differ only by tint fail. Scenario `build-fixtures`.
STAT-001 algebra is not on this lease.
VG-QA-001: evidence manifests require command exit codes and artifact
hashes. Pack template-only JSON cannot certify. CI under `native/tests`
stays Kimi.
VG-QA-002: live `AttackStarted` maps to `intent:swing` and
`attack-anticipate`. Owner strip paints Sim event / Intent swing. Dropping
`presentation_from_sim` for that event fails the fixture. A mocked
PresentationEvent is not a journey proof. Scenario `headless-contract`.
VG-GOV-003: `docs/execution/decisions/freeze-the-parity-scorecard.md`.
Owner strip paints Kill fill / Gold pit. A feature or VG-ID count cannot
pass. Each dimension names a journey. `first-session-clarity` owner strip
paints Slay wardens / Dash hint and parks off WASD, the objective, Tin
village, and Life so Space dash stays visible. Covering those combat
surfaces cannot certify. Local walk-on cannot certify. Scenarios
`xp-meter` / `first-session-clarity`. Evidence
`docs/execution/evidence/VG-GOV-003.json`.
VG-UI-007: `vital-orbs` — life left/red, mana right/blue. Mute is a HUD
chip. Owner strip paints Life left / Mana right and parks off WASD, the
objective, Tin village, and Life. Covering those combat surfaces
cannot certify. `hud-scale-floor` owner
strip paints Type floor / Ink contrast and parks off WASD, the objective,
Tin village, and Life. Covering those combat surfaces cannot certify.
Shrink type cannot certify.
`hud-pane-readability` recapture at 960/1366/3440 stays pairwise
disjoint; no owner review strip on that scenario. Open P-key tree never
intersects identity/controls/objective/minimap/quickbar/orbs. Open C-key
sheet never intersects identity/controls/objective; WASD remains. The sheet sits below the minimap and above Life. C or Esc closes stays
in the slot. An X on
the mana globe cannot count as a non-color cue. Open gear footer
Enter equips | I or Esc closes stays inside the pane; a clipped line
cannot certify. Open gear DEF and LVL stay inside the pane; a clipped
stats line cannot certify. Scenarios `vital-orbs` /
`hud-scale-floor` / `hud-pane-readability`. Evidence
`docs/execution/evidence/VG-UI-007.json`.
VG-GOV-006: disconnect/crash/quit cannot ack uncommitted extraction.
Owner strip paints Carry open / No extract. Extract ok cannot certify.
Scenario `death-disconnect`. Core D-106 stays Kimi/TASK-0018.
Evidence `docs/execution/evidence/VG-GOV-006.json`.
VG-UI-005: `route-map` — overlay zoom/opacity cannot move the player or
mint an off-snapshot warden blip. Owner strip paints Tin village / Risk
wardens and parks off WASD, the objective, the production Tin village
card, and Life. Covering those combat surfaces cannot certify. The owner
card titles Tin village, not `route:tin:1:0`. Owner Demo journeys not
reimplemented. Scenario `route-map`. Evidence
`docs/execution/evidence/VG-UI-005.json`.
VG-UI-004: `stat-explain` — expanded ATK names base/gear/passive/cond on
HUD ops. Compact Sources paints Base N | Gear signed (Pixelmix cannot
paint a middle-dot). Owner strip paints Base Gear / Cond off and parks
in the world lane right of the C-key sheet. Covering First Scion, WASD,
the objective, or Life cannot certify. Owner paint uses Base/Gear, not
`src` jargon. Lowercase base/gear cannot certify. Expanded sheet paints
Conditional once; a duplicate Conditional cannot certify. Slice builds
on the sheet are role | gear chips; tactics stay on HUD ops. A dormant
conditional cannot fold into Attack. Core STAT stays Kimi. WASD stays
off the C-key sheet. The sheet sits below the minimap and above Life.
The close hint stays in the slot. Scenario `stat-explain`. Evidence
`docs/execution/evidence/VG-UI-004.json`.
VG-UI-002: `pack-drag` — reject cannot lose, duplicate, or silently equip.
Owner strip paints Pack place / Reject keeps and parks below the minimap,
left of the gear pane. Covering WASD or LIFE cannot certify. Silent equip
cannot certify. Pack cells wrap Ember-edged axe at the type floor; a
12-char period clip cannot certify. Compare hint is Enter equips |
U unequips. Gear footer I or Esc closes stays inside the pane. Core
inventory-move stays Kimi.
VG-ART-005: `held-item` — world actor hold must change on equip. Owner
strip paints World hold / Ack equip and parks off WASD, the objective,
Tin village, and Life. Covering those combat surfaces cannot certify.
`loot-to-bank` owner strip paints Unarmed first / World hold the same
way. A filled paper-doll seat with `held:none` cannot pass. Scenarios
`held-item` / `loot-to-bank`. Evidence
`docs/execution/evidence/VG-ART-005.json`.

## VG-GOV-002 draft (not owner-stamped)

Until the owner stamps PROTOCOL vs pack claims: Cursor holds
`native/client/**` in the architect checkout; Kimi Work holds core/tools
in `kimiwork_verdigris`. First writer of a path in
`orchestration/CURSOR_KIMI_LANES.md` wins. Owner pushes; agents commit
locally. This is a working rule, not a VG-GOV-002 ruling.
