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
| TASK-0108 combat-depth-wave | Predecessor for readable combat; VG-ART-003/006 extend, never re-spec. Cursor 2026-09-06: local Telegraph ingest from JS `world:projectile` keys (`ingest-ranged-projectile-warning.hpp`, `ranged-warning` scenario). Core+wire stays Kimi `3b929637`. `remote_session.cpp` still successor. |
| TASK-0145, 0177, 0178, 0197, 0203, 0205–0207 Owner Demo | Journey/content/perf gates; VG-UI-006, VG-ART-008, VG-PERF-008, VG-GOV-007 extend |

## GOV

| VG | Disposition | Existing |
|---|---|---|
| VG-GOV-001 | new | `BASELINE.md` + `decisions/freeze-a-reproducible-baseline.md`; dual heads `486058f3` / `e7b65360` |
| VG-GOV-002 | new | `decisions/resolve-orchestration-precedence.md` — **owner stamp still required** |
| VG-GOV-003 | extend | TASK-0119 READY, TASK-0152 INTEGRATED, TASK-0206 AUTO_RELEASE; `docs/execution/decisions/freeze-the-parity-scorecard.md` — feature counts cannot pass |
| VG-GOV-004 | extend | this file + `decisions/crosswalk-existing-task-packets.md`; TASK-0108 extend never re-spec; TASK-0095/0097 superseded |
| VG-GOV-005 | verify | TASK-0114 INTEGRATED; `docs/execution/decisions/choose-the-renderer-trial-boundary.md` — gpu-sample is not an engine port |
| VG-GOV-006 | extend | TASK-0018, 0056, 0148; `death-disconnect` — disconnect cannot ack uncommitted extract |
| VG-GOV-007 | extend | Owner Demo content lots |
| VG-GOV-008 | new | pack `tools/roadmap.py` validate + unittest; see `docs/execution/decisions/audit-dependency-and-path-scheduling.md` |

## UI / ART / GPU / PERF (Cursor-leaning)

| VG | Disposition | Existing |
|---|---|---|
| VG-UI-001 | extend | native Escape stack (`pane-stack`); helper depth alone cannot pass |
| VG-UI-002 | extend | TASK-0171 INTEGRATED, 0184 BRIDGE_PREP; `pack-drag`; reject cannot lose/duplicate/silent-equip |
| VG-UI-003 | extend | paper-doll + ack-only HUD (`equipment`); pending compare cannot gold-frame as equipped |
| VG-UI-004 | extend | TASK-0156, 0159 INTEGRATED; `stat-explain` expandable ATK; dormant cannot fold into Attack |
| VG-UI-005 | extend | TASK-0076 INTEGRATED; 0178/0192/0203 Owner Demo; `route-map` zoom/opacity cannot reveal off-snapshot targets |
| VG-UI-006 | extend | TASK-0145/0177 + 0190/0197/0201 — **do not duplicate** |
| VG-UI-007 | extend | TASK-0159 INTEGRATED, 0118 READY, 0207; `vital-orbs` life left/red mana right/blue; mute is a chip not an X on mana |
| VG-UI-008 | new | XInput tick path + glyphs (`pad-path`); not mouse emulation |
| VG-ART-001/002/004 | extend | TASK-0141 INTEGRATED — village kit + collision-proxy ops (`kit-chunk`); not artist-local collision |
| VG-ART-003/006 | extend | TASK-0122 INTEGRATED; 0173/0174 READY; 0186/0187 BRIDGE_PREP — Cursor paints melee **poses** (VG-ART-003) and WarCry **weave labels** (VG-ART-006) without taking TASK-0173 models or re-speccing TASK-0108 |
| VG-ART-005 | extend | TASK-0182 AUTO_RELEASE, 0184; `held-item` world attachment; paper-doll seat alone cannot pass |
| VG-ART-007/008 | extend | TASK-0206/0205/0207 — Owner Demo, do not duplicate |
| VG-GPU-001 | new | isolated software quad (`gpu-sample`); not a D3D-only window |
| VG-GPU-002 | new | render-list → packets (`gpu-packets`); handles cannot snapshot |
| VG-GPU-003 | new | versioned software bindings (`shader-bindings`); no runtime .hlsl path |
| VG-GPU-004 | new | session-connected packet present (`gpu-reference`); not a disconnected demo |
| VG-GPU-005 | new | Y-sort + telegraph overlay (`grounding`); walls cannot erase warnings |
| VG-GPU-006 | new | moving light on bronze/stone (`material-light`); cannot wash out damage |
| VG-GPU-007 | new | BMP readback + provenance (`gpu-capture`); PNG R/B swap cannot certify; packet logs are not pixels |
| VG-GPU-008 | new | sample recreate/resize (`gpu-recover`); minimize/restore cannot leak |
| VG-PERF-001/002/008 | extend | TASK-0152, 0207; 008 Owner Demo — do not duplicate |
| VG-PERF-003–006 | new / extend 0207 | GDI batch, envelope, loot labels, hitch warmup |
| VG-PERF-007 | new | 32-cycle present/effect/resize soak (`memory-soak`); short scene fails |

## MOVE (Cursor client lease only)

| VG | Disposition | Existing |
|---|---|---|
| VG-MOVE-005 | extend | TASK-0165 `input_focus.hpp` wired in `fixed_game_tick` (`pane-focus`); not a core movement rewrite |
| VG-MOVE-006 | new | isolated `bindings.v1` (`remap-binds`); owner Documents cannot be the test path. Full VG-SHIP-001 packager stays Kimi |
| VG-MOVE-008 | new | `input-latency` p50/p95 on the paint path; `Simulation::dispatch` time is not photon. MOVE-007 buffering stays Kimi |
| VG-MOVE-001 | new | eight-way `player:move` names (`eight-way`); vertical-only collapse fails |
| VG-MOVE-002 | new | held aim survives locomotion (`aim-hold`); core move still clobbers without the adapter |
| VG-MOVE-003–004, 007 | extend / new | Kimi lease for sim travel distance / dash sweep / action buffering |

## ACT (Cursor presentation bridge only)

| VG | Disposition | Existing |
|---|---|---|
| VG-ACT-007 | new | `attack-beat` event bridge in `ingest_events`; not TASK-0108 and not `native/src/core.cpp` |
| VG-ACT-005 | new | catalog-typed warning window (`telegraph-spec`); remote durationMs cannot invent a longer cone. Core ACT-001–004/006/008 stay Kimi |
| VG-ACT-001–006, 008 | new / extend | Kimi lease (`native/src/**`) |

## WORLD (Cursor presentation dressing only)

| VG | Disposition | Existing |
|---|---|---|
| VG-WORLD-008 | new | versioned `dressing-pass`; topology hash ignores decoration. Core WORLD-001–007 stay Kimi |
| VG-WORLD-001–007 | extend / new | Kimi lease |

## ITEM (Cursor presentation filter only)

| VG | Disposition | Existing |
|---|---|---|
| VG-ITEM-006 | new | `loot-filter` nameplates; cannot mutate sim ground. ITEM-001–005/007–008 stay Kimi |
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
authoritative XP bar, route card under the minimap (client zoom `[`/`]`),
character-sheet attack source vs dormant conditional, backpack drag
(VG-UI-002), combat audio (VG-SOUND-003/004/005) with event-id
dedup, warning-priority steal, and one ambience layer per route.
Snapshot field `xp` remains reserved. VG-PERF-001: F3/`frame-budget`
prints named display, CPU count, and floor/world/hud/upload fields.
VG-PERF-005: loot nameplates cap at 12 nearest; Drop sprites stay.
VG-PERF-003: effect/telegraph GDI pens and brushes are reused; scenario
`effect-batch` keeps Impact/Swing/Telegraph ops. VG-PERF-004: one floor
bitmap, GDI caps, effect cap 128 across resize cycles (`resource-envelope`).
VG-PERF-006: `warm_combat_glyphs` before first strike; scenario
`hitch-warmup` prints cold/warm/prepared and fails if cold is omitted.
VG-UI-007: tooltip titles use ink-on-panel contrast plus a triangular mark.
VG-ART-004: tin village kit (all five scenery kinds); solid pieces emit
`collision-proxy:*`; dressing gate does not. Scenario `kit-chunk`.
VG-ART-006: WarCry family `vfx-weave:cast|travel|impact|cancel`; radius
capped vs screen-fill; telegraph remains. Scenario `weave-vfx`.
VG-UI-008: XInput sampled on the fixed tick; A strike / B dash / X take /
Y gear; hotplug in/out; mouse position cannot mint `pad:connected`.
Scenario `pad-path`.
VG-SOUND-002: CC0 provenance table for combat cues. Scenario `legal-sounds`.
VG-SOUND-008: explore/combat/recovery music coalesced per drain; unload
clears the want state. Scenario `music-phase`. Device mute still silences
waveOut.
VG-GPU-001: isolated software backend (64×64 textured quad + shutdown).
Not a Windows-only D3D proof. Scenario `gpu-sample`.
VG-GPU-002: Telegraph draw class copied to handle-free packets. Poisoned
`backend_handle` fails snapshot. Scenario `gpu-packets`.
VG-ART-001: in-game HUD names camera/proportion/palette/contrast. An
external concept-art token cannot substitute. Scenario `visual-target`.
VG-ART-002: bronze/stone albedo+rim maps with SPDX CC0; magenta fill
cannot ship. Village kit uses the family. Scenario `bronze-stone`.
VG-GPU-003: layout v1 software program `software-albedo-rim-v1`. Wrong
backend or stale layout fails; failed load does not paint. Scenario
`shader-bindings`.
VG-GPU-004: software present of live session packets (actors/world/effects/HUD).
A textured-quad demo with no session cannot pass. Scenario `gpu-reference`.
VG-GPU-005: scenery/actors sort by world Y; telegraphs paint after that pass.
Scenario `grounding`.
VG-GPU-006: moving light on the cooked family; channel cap 220; damage chroma
cannot be washed white. Scenario `material-light`.
VG-GPU-007: BMP readback plus provenance sidecar. Packet snapshot text is not
pixel evidence. A PNG whose red/blue channels are swapped versus the DIB
cannot certify. Scenario `gpu-capture` / `vital-orbs`.
an image. Scenario `gpu-capture`.
VG-GPU-008: recreate/resize/minimize-restore keep one live buffer; failure
surfaces `gpu-error:recreate`. Scenario `gpu-recover`.
VG-SOUND-001: software tone adapter plays a generated PCM burst and shuts
down. A zero-duration cue is not audible. Scenario `sound-adapter`.
VG-SOUND-005: one ambience cue per route; rapid reentry does not stack.
Scenario `ambience-layer`.
VG-SOUND-006: mute cannot reset SFX/music volumes; zero SFX stays silent.
Scenario `audio-prefs`.
VG-UI-003: equip HUD is ack-only (`equip:ok` after ItemEquipped). Pending
compare cannot gold-frame as equipped. Scenario `equipment`.
VG-PERF-007: 32 resize/effect cycles; floor bitmaps stay 1. Scenario
`memory-soak`.
VG-SOUND-007: mixer tape from mixed pack + elite; isolated preview fails.
Scenario `dense-mix`. Review `docs/execution/captures/art-wave/dense-mix-score.txt`.
VG-UI-001: Escape dismisses character then gear; bare Escape quits. Helper
depth alone cannot prove. Scenario `pane-stack`.
`pane-stack`.
VG-MOVE-005: TASK-0165 focus reducer gates WASD, combat, pickup, and pack
drag. Closing a pane cannot release a held attack. Scenario `pane-focus`.
VG-MOVE-006: versioned bindings persist under an isolated test profile.
Duplicate codes and unknown devices fail on the HUD; owner Documents
cannot be the write target. Scenario `remap-binds`.
VG-MOVE-008: input QPC to `paint_scene` present QPC. p50/p95 on this
machine. Command dispatch time is not `input-latency:photon`. Scenario
`input-latency`. Protocol
`docs/execution/decisions/measure-native-input-response.md`.
VG-MOVE-001: eight-way encoder keeps both axes. A vertical-only name
cannot pass a diagonal. Scenario `eight-way`.
VG-MOVE-002: `player:move` does not replace held aim; local tick
re-applies aim after move. Scenario `aim-hold`. Core `resolve_move` still
turns facing — that is the defect the adapter covers.
VG-ACT-007: AttackStarted/DamageApplied/ActorDied drive
`attack-beat:*`. A fabricated swing cannot mint the beat. Scenario
`attack-beat`. Core melee resolution stays with Kimi.
VG-ACT-005: local ticks and remote `durationMs` share
`presentation_catalog().telegraph_ticks` and reach. `value/50` cannot
invent a longer window. Expired/cancelled warnings leave no footprint.
Scenario `telegraph-spec`.
VG-WORLD-008: dressing-pass v1/v2 change decoration hash only.
A tree visual cannot become an unreported solid. Scenario `dressing-pass`.
VG-ITEM-006: loot category facts and a presentation filter. Hiding
trophies cannot move ownership or drop tables. Scenario `loot-filter`.
VG-BUILD-001: reach/pressure/magic fixtures list tactics, weakness, gear,
and encounter answers. Three melee clones that differ only by tint fail.
Scenario `build-fixtures`. STAT-001 algebra is not on this lease.
VG-QA-001: evidence manifests require command exit codes and artifact
hashes. Pack template-only JSON cannot certify. CI under `native/tests`
stays Kimi.
VG-QA-002: live `AttackStarted` maps to `intent:swing` and
`attack-anticipate`. Dropping `presentation_from_sim` for that event fails
the fixture. A mocked PresentationEvent is not a journey proof. Scenario
`headless-contract`.
VG-GOV-003: `docs/execution/decisions/freeze-the-parity-scorecard.md`.
A feature or VG-ID count cannot pass. Each dimension names a journey.
VG-UI-007: `vital-orbs` — life left/red, mana right/blue. Mute is a HUD
chip. An X on the mana globe cannot count as a non-color cue.
VG-GOV-006: disconnect/crash/quit cannot ack uncommitted extraction.
Scenario `death-disconnect`. Core D-106 stays Kimi/TASK-0018.
VG-UI-005: `route-map` — overlay zoom/opacity cannot move the player or
mint an off-snapshot warden blip. Owner Demo journeys not reimplemented.
VG-UI-004: `stat-explain` — expanded ATK names base/gear/passive/cond.
A dormant conditional cannot fold into Attack. Core STAT stays Kimi.
VG-UI-002: `pack-drag` — reject cannot lose, duplicate, or silently equip.
VG-ART-005: `held-item` — world actor hold must change on equip. A filled
paper-doll seat with `held:none` cannot pass.

## VG-GOV-002 draft (not owner-stamped)

Until the owner stamps PROTOCOL vs pack claims: Cursor holds
`native/client/**` in the architect checkout; Kimi Work holds core/tools
in `kimiwork_verdigris`. First writer of a path in
`orchestration/CURSOR_KIMI_LANES.md` wins. Owner pushes; agents commit
locally. This is a working rule, not a VG-GOV-002 ruling.
