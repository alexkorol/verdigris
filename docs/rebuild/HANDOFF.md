# Native reconstitution handoff

## 2026-09-06 — owner objective strip + dash (Cursor)

- Owner HUD paints `Slay the wardens (1 remain)`, not `objective: ...`.
  HUD ops keep the protocol prefix. Compact controls restore `Space dash`
  (TASK-0153 first-session-clarity). Extract strip drops the `12u` dump.
- Scenario `first-session-clarity` PASS. Capture `visual-target-960x600.png`.

## 2026-09-06 — adult Scion rig (Cursor)

- Vector `humanoid` now uses adult proportions (head ~1/8, jointed legs,
  tapered torso). A 1/3 chibi head cannot pass. VG-ART-001 / VG-ART-003.
- Captures `visual-target-960x600.png` and `attack-poses-960x600.png`.
  TASK-0173 models untouched. Not Owner Demo.

## 2026-09-06 — composition sheet XP + owner risk/return (Cursor)

- `visual-target` seeds three level-1 kill XP so the sheet shows a filled
  meter, not a black hairline. Route card paints `Risk: wardens` and
  `Return: press F at the pad`. HUD op labels stay protocol-stable.
- Captures `visual-target-960x600.png` and `route-map-960x600.png`.

## 2026-09-06 — owner route card names (Cursor)

- Route card paints `Tin village` / `Town road`, not `route:tin:1:0`.
  A protocol colon-id cannot be the owner title. F3 still shows the raw
  id. Compact controls: `WASD | LMB strike | I gear | F3 binds`.
- Scenario `route-map`. Captures `route-map-960x600.png` and
  `visual-target-960x600.png`. Not TASK-0108, not Owner Demo.

## 2026-09-06 — hide skeleton art loader chip (Cursor)

- Owner HUD no longer paints `art: PNG billboards loaded`. Loaded art is
  silent; missing plates still warn. F3 keeps the diagnostic line.
- `first-fight` / `visual-target` reject a loader chip as the composition
  sheet. Mute chip stays. Capture
  `docs/execution/captures/art-wave/visual-target-960x600.png`.
- Not TASK-0108, not Owner Demo.

## 2026-09-06 — local XP meter fill (Cursor)

- Live local HUD showed `XP lv 1` over an empty black strip because
  `sync_world_from_simulation` hard-coded `xp_fraction = 0`.
- Local kill XP now uses the same RS curve as snapshot `state.xp`
  (12 per monster level). Scenario `xp-meter`: empty gold=0, filled
  gold=805 at fraction 0.493. Capture
  `docs/execution/captures/art-wave/xp-meter-960x600.png`.
- Did not touch `remote_session.cpp`, `native/src/core.cpp`, or the
  networking snapshot writer. Not TASK-0108.

## 2026-09-06 — VG-UI-007 pane vs HUD at owner 3440×1440 (Cursor)

- Extends TASK-0159: `hud-pane-readability` now presents 960×600, 1366×768,
  and 3440×1440. Open gear pane stays disjoint from identity, controls,
  objective, art chip, minimap, quickbar, and orbs. Captures write to
  `docs/execution/captures/art-wave/` — a TASK-0159 folder PNG cannot
  certify this wave.
- Viewed open/closed 3440×1440 plus open 960×600. Life red left, mana blue
  right, gear pane on the right, HUD chips clear of the pane.
- Evidence `docs/execution/evidence/VG-UI-007.json`. Not Owner Demo.
  `remote_session.cpp` remains narrow-released for Kimi.

## 2026-09-06 — live HUD window + VG-UI-007 scale/cues (Cursor)

- Presentation gate: launched `verdigris_client.exe`, captured the live
  3440×1440 window with `native/tools/capture-window.ps1`, viewed
  `docs/execution/captures/art-wave/live-hud-owner.png`. Life 100/100 red
  left, mana 50/50 blue right, XP lv 1, skill chips, objective, warden
  grounded. PrintWindow DIB is BGR; the committed PNG is RGB-corrected.
- VG-UI-007: `hud-scale-floor` now writes
  `docs/execution/captures/art-wave/hud-scale-floor-960x600.png`. Scale 0
  rejected; 640×480 still floors type; low life has a chevron; foe tooltip
  contrast is ink-on-panel. Shrinking type cannot pass. Not VG-UI-006 /
  Owner Demo.
- `native/client/remote_session.cpp` narrow-released for Kimi's remaining
  TASK-0108 `world:projectile` parse. `main.cpp` lease stays ACTIVE.

## 2026-09-06 — remaining Cursor-lease SOUND/MOVE/WORLD/ITEM/PERF (Cursor)

- VG-SOUND-006: mute cannot reset SFX/music volumes (`audio-prefs`).
- VG-SOUND-007: mixed-pack mixer tape; isolated preview fails (`dense-mix`).
- VG-MOVE-005: focused panes swallow WASD/combat (`pane-focus`).
- VG-MOVE-006: isolated `bindings.v1`; Documents cannot be the test path
  (`remap-binds`).
- VG-WORLD-008: dressing-pass v1/v2 cannot change topology.
- VG-ITEM-006: loot nameplates; hiding cannot mutate sim ground.
- VG-PERF-001: named Win32 machine + floor/world/hud/upload
  (`frame-budget`, 11.5 ms avg at 3440×1440, bound stays 40 ms).
- VG-PERF-003–007: effect-batch, resource-envelope, loot-label-budget,
  hitch-warmup, memory-soak.

## 2026-09-06 — kit, weave, pad, beats, combat audio (Cursor lease)

- VG-ART-004: tin village kit + collision proxies (`kit-chunk`).
- VG-ART-006: WarCry weave labels; spectacle cannot hide telegraph
  (`weave-vfx`). Does not take TASK-0173 models.
- VG-UI-008: XInput on the fixed tick (`pad-path`). Mouse cannot mint
  `pad:connected`.
- VG-ACT-007: AttackStarted/DamageApplied drive `attack-beat:*`. A
  fabricated swing cannot mint the beat.
- VG-SOUND-003/004/005: `combat-audio` + `ambience-layer`. Same event ID
  cannot double-play; cosmetics cannot starve `scion-lost`; rapid reentry
  cannot stack ambience.

## 2026-09-06 — GPU present path 003–006/008 (Cursor lease)

- VG-GPU-003: `software-albedo-rim-v1` bindings; stale/wrong backend fail
  closed. Scenario `shader-bindings`. Capture
  `docs/execution/captures/art-wave/shader-bindings-quad.bmp`.
- VG-GPU-004: live session packets present; disconnected demo rejected.
  Scenario `gpu-reference`. BMP + PNG
  `docs/execution/captures/art-wave/gpu-reference-*`.
- VG-GPU-005: Y-sort + telegraph overlay after scenery. Scenario
  `grounding`. Capture `docs/execution/captures/art-wave/grounding-960x600.png`.
- VG-GPU-006: moving light, channel cap 220, damage chroma not washed.
  Scenario `material-light`. Quad BMP + HUD PNG.
- VG-GPU-008: recreate/resize/minimize-restore keep one buffer; `0x0`
  surfaces `gpu-error:recreate`. Scenario `gpu-recover`.

## 2026-09-06 — packets, bronze/stone, legal sounds, graph audit (Cursor)

- VG-GPU-002: Telegraph draw class copies to handle-free packets
  (`gpu-packets`). Poisoned `backend_handle` cannot snapshot. Capture
  `docs/execution/captures/art-wave/gpu-packets-960x600.png`. Snapshot
  `docs/execution/captures/art-wave/gpu-packets-snapshot.txt`.
- VG-ART-002: cooked bronze/stone albedo+rim, SPDX CC0. Magenta fill
  cannot pass. Scenario `bronze-stone`. Capture
  `docs/execution/captures/art-wave/bronze-stone-960x600.png`.
- VG-SOUND-002: combat family includes swing windup `attack-anticipate`.
  `unlicensed-preview` cannot ship. Scenario `legal-sounds`. Capture
  `docs/execution/captures/art-wave/legal-sounds-960x600.png`.
- VG-GOV-008: pack `roadmap.py validate` (200/689) plus unittest overlap
  fixtures. Decision already at
  `docs/execution/decisions/audit-dependency-and-path-scheduling.md`.

## 2026-09-06 — first-wave P0 + mute-on-unload (Cursor lease)

- VG-GPU-001: isolated software 64×64 bronze/stone quad (`gpu-sample`).
  Capture `docs/execution/captures/art-wave/gpu-sample-quad.bmp`. Unknown
  backend cannot pass. Not a D3D presenter.
- VG-ART-001: in-game HUD names camera/proportion/palette/contrast
  (`visual-target`). External concept-art token cannot substitute.
- VG-SOUND-001: software 440 Hz adapter (`sound-adapter`). Zero-duration
  cue cannot pass as audible.
- VG-SOUND-008: `theme_for` + music-bus mute on `music:none` so unload
  cannot voice a leftover combat loop. Scenario `music-phase`. Capture
  `docs/execution/captures/art-wave/music-phase-960x600.png`. Device stays
  muted in the harness; STORY phase authority stays Kimi.
- VG-ART-003: idle cannot wear the active strike pose (`attack-poses`).
  Capture `docs/execution/captures/art-wave/attack-poses-960x600.png`.
  Does not take TASK-0173 models or re-spec TASK-0108.

## 2026-09-06 — TASK-0108 local Telegraph ingest (Cursor lease)

- Client stage of Kimi's ranged `world:projectile` windup: JS payload keys
  become the existing Telegraph op, then attributed Damage/Impact.
  Helper `native/client/ingest-ranged-projectile-warning.hpp`. Lock in
  `native/tests/presentation_events_tests.cpp`. Scenario `ranged-warning`.
  Does not edit `native/src/**`, `native/include/**`, or
  `native/client/remote_session.cpp`. Slam `monster:telegraph` is not this
  mapper. A hit without a preceding warning cannot mint a Telegraph.

## 2026-09-06 — ship Cursor pack wave (owner asked to push)

Architect checkout `codex/native-reconstitution`. Lands `docs/execution/`
(pack ingest, GOV-001/004 baseline+crosswalk, GOV-002 draft, evidence)
plus the native client/GPU HUD wave. TASK-0108 stays Kimi's core+wire;
client Telegraph ingest stays on this lease. VG-GOV-002 is **not**
owner-stamped. Dual program heads: this branch vs
`origin/codex/goal-aaa-systems` @ `e7b65360`.

## 2026-09-06 — native pane Escape stack (Cursor, uncommitted)

- VG-UI-001: Escape dismisses character then gear; bare Escape quits.
  Helper depth without native paint/Escape cannot prove. Scenario
  `pane-stack`. Capture
  `docs/execution/captures/art-wave/pane-stack-960x600.png`.

## 2026-09-06 — pack-grid drag occupancy (Cursor, uncommitted)

- VG-UI-002: valid pack drop moves the cell; a rejected drop cannot lose,
  duplicate, or silently equip. Scenario `pack-drag`. Capture
  `docs/execution/captures/art-wave/pack-drag-960x600.png`. Sim
  `inventory_move` stays Kimi.

## 2026-09-06 — ack-only equip compare (Cursor, uncommitted)

- VG-UI-003: gear compare plate uses the acknowledged seat. A pending
  request paints `compare:pending`, not gold `currently equipped`.
  Scenario `equipment`. Capture
  `docs/execution/captures/art-wave/equipment-960x600.png`.

## 2026-09-06 — equipped hold on the actor (Cursor, uncommitted)

- VG-ART-005: world `held:*` attachment must follow the acknowledged equip.
  A paper-doll seat with `held:none` cannot pass. Scenario `held-item`.
  Capture `docs/execution/captures/art-wave/held-item-960x600.png`. Does
  not re-spec TASK-0108 or Owner Demo.

## 2026-09-06 — readable ATK sources (Cursor, uncommitted)

- VG-UI-004: character sheet Attack is base+gear+passive only while Cond
  is inactive. `B` expands four source rows. Folding dormant into Attack
  cannot pass. Scenario `stat-explain`. Capture
  `docs/execution/captures/art-wave/stat-explain-960x600.png`. Core STAT
  stays Kimi.

## 2026-09-06 — map/route overlay (Cursor, uncommitted)

- VG-UI-005: minimap zoom/opacity are overlay settings. Scenario `route-map`
  proves max zoom cannot paint `off-snapshot-warden`. Capture
  `docs/execution/captures/art-wave/route-map-960x600.png`. Owner Demo
  journeys not duplicated.

## 2026-09-06 — death/disconnect extract ack (Cursor, uncommitted)

- VG-GOV-006: disconnect cannot silently ack uncommitted extraction.
  HUD `extract:uncommitted` + chip; `extract:ok` only after sim bank.
  Scenario `death-disconnect`. Capture
  `docs/execution/captures/art-wave/death-disconnect-960x600.png`.
  Decision `docs/execution/decisions/rule-on-death-and-disconnect.md`.
  Does not edit `native/src/core.cpp`.

## 2026-09-06 — capture channels + renderer trial (Cursor, uncommitted)

- VG-GPU-007: GDI+ PNG save now swaps DIB B,G,R so COLORREF red/blue
  survive the file. A channel-swapped still cannot certify. Scenario
  `gpu-capture` and recaptured `vital-orbs`. Capture
  `docs/execution/captures/art-wave/vital-orbs-960x600.png` (life 208,69,69
  left; mana 91,146,239 right).
- VG-GOV-005: `docs/execution/decisions/choose-the-renderer-trial-boundary.md`.
  The software sample is the GPU trial; a green quad is not an engine port.
  Extends TASK-0114; does not pick sokol/SDL.

## 2026-09-06 — vital orbs + parity scorecard (Cursor, uncommitted)

- VG-UI-007: life stays the left vessel, mana the right. Mute is a HUD
  chip (`audio muted`), not an X on the mana globe. Scenario `vital-orbs`.
  Swapping the blue sheet crop onto life fails. Capture
  `docs/execution/captures/art-wave/vital-orbs-960x600.png`.
- VG-GOV-003: `docs/execution/decisions/freeze-the-parity-scorecard.md`.
  A feature or VG-ID count cannot pass. Does not mint TASK numbers.

## 2026-09-06 — eight-way move + held aim (Cursor, uncommitted)

- VG-MOVE-001: `encode_eight_way` keeps both axes on diagonals (`up-left`).
  A vertical-only encoder cannot pass. Scenario `eight-way`. Capture
  `docs/execution/captures/art-wave/eight-way-960x600.png`.
- VG-MOVE-002: remote `player:move` no longer overwrites held aim.
  Local tick re-aims after move because core `resolve_move` still turns
  facing. Scenario `aim-hold`. Capture
  `docs/execution/captures/art-wave/aim-hold-960x600.png`. Does not edit
  `native/src/core.cpp`.

## 2026-09-06 — input-to-present latency (Cursor, uncommitted)

- VG-MOVE-008: key/button QPC paired with `paint_scene` present QPC.
  Scenario `input-latency` reports p50/p95 on the named Win32 machine.
  `Simulation::dispatch` elapsed time is not `input-latency:photon`.
  Protocol `docs/execution/decisions/measure-native-input-response.md`.
  Capture `docs/execution/captures/art-wave/input-latency-960x600.png`.
  Report `docs/execution/captures/art-wave/input-latency-report.txt`.
  VG-MOVE-007 buffering stays Kimi.

## 2026-09-06 — headless presentation contract (Cursor, uncommitted)

- VG-QA-002: `AttackStarted` from the simulation maps to `intent:swing` and
  `attack-anticipate`. Removing that bridge fails the fixture. A mocked
  PresentationEvent with swing FX cannot prove the journey. Scenario
  `headless-contract`. Capture
  `docs/execution/captures/art-wave/headless-contract-960x600.png`. Does
  not take `native/tests/**` or mint TASK numbers.

## 2026-09-06 — telegraph timing and geometry (Cursor, uncommitted)

- VG-ACT-005: warning duration and reach come from
  `Simulation::presentation_catalog()`, not `event.value / 50`. Local ticks
  and a remote millisecond payload render the same window. AttackStarted
  cancels; expired entries cannot stay a silent damaging cone. Scenario
  `telegraph-spec`. Capture
  `docs/execution/captures/art-wave/telegraph-spec-960x600.png`. Does not
  edit `native/src/core.cpp`.

## 2026-09-06 — slice build fixtures + evidence schema (Cursor, uncommitted)

- VG-BUILD-001: character sheet names reach (thrust/pike), pressure
  (melee/close blade), and magic (war-cry/vessel). Each lists tactics,
  weakness, gear, and an encounter answer. Three tinted copies of melee
  fail `distinct_slice_loops`. Scenario `build-fixtures`. Capture
  `docs/execution/captures/art-wave/build-fixtures-960x600.png`. Does not
  edit `native/src/core.cpp`.
- VG-QA-001: `docs/execution/pack/tools/evidence_manifest.py` rejects
  template-only records and screenshots without sha256/`produced_by`.
  Does not mint TASK numbers or take `native/tests/**`.

## 2026-09-06 — loot filter facts (Cursor, uncommitted)

- VG-ITEM-006: ground drops publish `loot-fact:weapon|trophy|misc`. Hiding
  trophies suppresses nameplates only; Drop sprites, `loot_positions`, and
  sim ground tables stay put. Scenario `loot-filter`. Capture
  `docs/execution/captures/art-wave/loot-filter-960x600.png`. Does not
  edit `native/src/core.cpp` or item definitions.

## 2026-09-06 — visual dressing vs topology (Cursor, uncommitted)

- VG-WORLD-008: versioned decoration pass on the tin village layout.
  Dressing trees are non-solid (`dressing:tree`). v2 changes the
  decoration hash only; spawn, scenery seed, and topology hash stay put.
  A solid dressing tree is an unreported obstacle. Scenario
  `dressing-pass`. Capture
  `docs/execution/captures/art-wave/dressing-pass-960x600.png`. Does not
  edit `native/src/core.cpp`.

## 2026-09-06 — attack presentation beat (Cursor, uncommitted)

- VG-ACT-007: `ingest_events` maps AttackStarted → anticipate (plus
  `attack-anticipate` cue), DamageApplied → impact, ActorDied →
  aftermath, dash during anticipate → cancel. A swing effect with no
  sim event cannot mint `attack-beat:*`. Scenario `attack-beat`. Capture
  `docs/execution/captures/art-wave/attack-beat-960x600.png`. Does not
  edit `native/src/core.cpp` or re-spec TASK-0108.

## 2026-09-06 — remapped controls (Cursor, uncommitted)

- VG-MOVE-006: versioned keyboard bindings persist under
  `%TEMP%\verdigris-isolated-profile`. Duplicate codes paint
  `bind:conflict`; unknown devices paint `bind:invalid-device`. Saving
  into a Documents path fails `bind:owner-profile`. Restart reloads the
  remapped dash; restore defaults returns Space. Scenario `remap-binds`.
  Capture `docs/execution/captures/art-wave/remap-binds-960x600.png`.
  VG-SHIP-001's packager in `native/tools/**` stays Kimi.

## 2026-09-06 — pane focus + 200-ID registry (Cursor, uncommitted)

- VG-MOVE-005: TASK-0165 `input_focus` now gates the production tick. WASD,
  strike/dash, pickup, and pack-drag do not leak through focused panes.
  A held attack cannot fire when the pane closes. Scenario `pane-focus`.
  Capture `docs/execution/captures/art-wave/pane-focus-960x600.png`.
- VG-GOV-004: 200-row registry `docs/execution/CROSSWALK_REGISTRY.md` (no
  TASK mint). VG-GOV-008: pack `roadmap.py` validate + unittest evidence
  in `docs/execution/decisions/audit-dependency-and-path-scheduling.md`.

## 2026-09-06 — dense mix + pane stack (Cursor, uncommitted)

- VG-SOUND-007: score the mixer tape from a mixed pack plus elite telegraph
  and a danger cue. An isolated tone preview cannot pass. Scenario
  `dense-mix`. Record `docs/execution/captures/art-wave/dense-mix-score.txt`.
- VG-UI-001: native Escape stack — character then gear then quit. Scenario
  `pane-stack` presents the gear pane; a depth helper alone is not the proof.

## 2026-09-06 — sound adapter, prefs, ambience, equip ack, soak (Cursor, uncommitted)

- VG-SOUND-001: software PCM tone adapter; unknown backend fails; shutdown
  releases the buffer. Scenario `sound-adapter`.
- VG-SOUND-006: prefs file keeps SFX/music through mute toggles; zero SFX
  volume drains silence. Scenario `audio-prefs`.
- VG-SOUND-005: one `ambience:<route>` loop; salt reentry cannot stack.
  Scenario `ambience-layer`.
- VG-UI-003: `EquipView` is ack-only. Live HUD `equip:pending:` cannot be
  `equip:ok`. Helmets cannot occupy main-hand. Scenario `equipment`.
- VG-PERF-007: 32 present/resize/effect cycles stay inside the resource
  envelope. A short scene cannot pass. Scenario `memory-soak`.

## 2026-09-06 — material light, pixel capture, GPU recover (Cursor, uncommitted)

- VG-GPU-006: moving light on bronze/stone (`shade_texel_lit`); channels
  cap at 220. Damage-zone chroma cannot be concealed by additive white.
  Live HUD `material-light:moving`. Scenario `material-light`.
- VG-GPU-007: software readback writes a BMP plus provenance
  (backend/content/platform). A semantic packet log cannot count as the
  capture. Scenario `gpu-capture`.
- VG-GPU-008: `RecoverablePresenter` resize/minimize-restore keeps one
  live buffer. Failed recreate surfaces `gpu-error:recreate` and releases
  pixels. Scenario `gpu-recover`.

## 2026-09-06 — grounding / telegraph overlay (Cursor, uncommitted)

- VG-GPU-005: contact shadows stay at feet; painter sorts by world Y;
  threat telegraphs paint after scenery so a foreground wall cannot erase
  the warning. Scenario `grounding`.

## 2026-09-06 — GPU reference scene from live packets (Cursor, uncommitted)

- VG-GPU-004: `present_reference_scene` shades the software sample from
  session packets (Player/Monster, scenery, impact, HUD target sheet).
  A disconnected textured-quad demo fails. Scenario `gpu-reference`.
  BMP `docs/execution/captures/art-wave/gpu-reference-session.bmp`.

## 2026-09-06 — visual target + bronze/stone + shader bindings (Cursor, uncommitted)

- VG-ART-001: live HUD names the in-game composition target
  (`target:camera:top-down`, adult proportion, bronze-stone palette,
  ink-on-panel contrast). Concept-art HUD tokens are rejected. Scenario
  `visual-target`. Capture
  `docs/execution/captures/art-wave/visual-target-960x600.png`.
- VG-ART-002: cooked albedo/rim maps in `bronze_stone.hpp` (CC0). Village
  shrine/ruin/gate sample the family; magenta placeholder cannot pass.
  Scenario `bronze-stone`.
- VG-GPU-003: `cook-shaders-and-resource-bindings.hpp` layout v1. Software
  load has no runtime shader path. Stale layout and non-Software backends
  fail instead of drawing a silent fill. Scenario `shader-bindings`.

## 2026-09-06 — GPU sample + semantic packets (Cursor, uncommitted)

- VG-GPU-001: isolated `native/renderer/gpu` software sample draws a
  bronze/stone textured quad, writes a BMP, and shuts down. Unknown
  backends fail. Not a D3D-only window. Scenario `gpu-sample`.
- VG-GPU-002: `packets_from_render_list` copies Telegraph/etc with
  `backend_handle == 0`. Snapshot text has no HDC/D3D/pointer tokens.
  Scenario `gpu-packets`. Live HUD `gpu-backend:software`.

## 2026-09-06 — pad path + legal sounds + music phases (Cursor, uncommitted)

- VG-UI-008: XInput on the 20 Hz tick (injected `PadReport` for harness).
  Glyphs `pad-glyph:*`, hotplug in/out. Mouse coordinates cannot set
  `pad:connected`. Scenario `pad-path`.
- VG-SOUND-002: SPDX CC0 family in `sound_family.hpp`. Scenario
  `legal-sounds`.
- VG-SOUND-008: `music:explore|combat|recovery|none|muted`. Coalesced
  submit; unloaded session cannot keep a competing want. Scenario
  `music-phase`.

## 2026-09-06 — village kit + WarCry weave (Cursor, still uncommitted)

- VG-ART-004: tin village kit includes dwelling, shrine, tree, ruin, and a
  non-solid dressing gate. Solid pieces publish `collision-proxy:<kind>`
  on the production render list (same solids as movement). Scenario
  `kit-chunk`. Capture `docs/execution/captures/art-wave/kit-chunk-960x600.png`.
- VG-ART-006: WarCry aura/fade labeled `vfx-weave:cast|travel|impact|cancel`;
  radius capped to 1/6 of the short viewport edge; elite telegraph still
  draws. Extends TASK-0122; does not re-spec TASK-0108. Scenario
  `weave-vfx`. Capture `docs/execution/captures/art-wave/weave-vfx-960x600.png`.

## 2026-09-05 night — melee attack poses (Cursor, still uncommitted)

- VG-ART-003: Scion melee is four rig poses (windup / active / recovery /
  cancel) driven by swing lifetime, cooldown, and dash dust — not a single
  sine of frame count. Scenario `attack-poses`. Does not implement
  TASK-0108 or TASK-0173 model files.

## 2026-09-05 night — combat hitch warmup (Cursor, still uncommitted)

- VG-PERF-006: `warm_combat_glyphs` starts GDI+, Pixelmix, damage fonts,
  and combat pens/brushes, then draws a dummy numeral/ellipse before the
  first player strike. Live local and remote clients call it after
  billboards. Scenario `hitch-warmup` prints cold, warm, and prepared
  paint times; omitting the cold number fails. Swing and Damage ops stay.

## 2026-09-05 night — resource envelope (Cursor, still uncommitted)

- VG-PERF-004: floor cache shrinks when the view is less than half the
  bitmap; effects use `add_effect` with a 128 cap (oldest dropped).
  Scenario `resource-envelope` cycles 1920/640/960 eight times, then
  300 impacts. One floor bitmap; pens/brushes ≤ 128; fx = 128. A cheap
  frame cannot excuse growth.

## 2026-09-05 night — effect batch + tooltip contrast (Cursor, still uncommitted)

- VG-PERF-003: `fill_ellipse` / `ring_ellipse` / `draw_line` reuse cached
  GDI pens and brushes (128 cap). Damage numerals reuse fonts by height.
  Scenario `effect-batch`: 40 impacts + 40 swings still emit ops; a
  thrust telegraph cannot be dropped to pass; second paint reuses pens.
- VG-UI-007: hover tooltip titles and facts paint `kInk` on the panel
  (contrast ≥ 4.5 vs `kPanelMid`); accent is a triangle mark. Extended
  `hud-scale-floor`.

## 2026-09-05 night — loot nameplates + paint trace (Cursor, still uncommitted)

- VG-PERF-005: Z-key loot names are the 12 nearest pouches (X-target
  always included). Every drop still paints as `Drop`. Scenario
  `loot-label-budget` (120 pouches).
- VG-PERF-001: `frame-budget` prints display size, logical CPUs, and
  last-paint floor/world/hud/upload fields. F3 overlay matches. Live
  present times `BitBlt` as upload; headless scenarios report upload 0.0.

## 2026-09-05 night — route card + stat source (Cursor, still uncommitted)

- VG-UI-005: route card under minimap (return/risk, no foe names);
  client-only `[`/`]` zoom. Hidden while gear/character/tree panes own
  the left column. VG-UI-004: character sheet ATK src / Passive / Cond
  dormant. VG-UI-007: life chevron when low. VG-SOUND-006: mute flag
  next to the client exe plus a mute glyph on the resource orb.
- VG-UI-002: backpack drag uses `inventory_grid` occupancy. Valid drop
  moves the cell; rejected drop cannot lose, duplicate, or equip.
  Equip stays Enter / drop-on-weapon / `Command::equip`.
  Evidence: `loot-to-bank` and `hud-pane-readability` PASS (0 failures).
- VG-SOUND-003/004/005: local combat events voice through the mixer;
  duplicate event keys cannot double-play; Scion-lost outranks cosmetics;
  ambience does not stack on the same route. Scenario `combat-audio` PASS.
- VG-UI-007: type floor (`skin::kMinSmallPx` / `kMinBodyPx`); scale 0 is
  rejected; low-life chevron; hover tooltip stays in-frame. Scenario
  `hud-scale-floor`.

## 2026-09-05 — execution pack ingest + native HUD chrome (Cursor)

- Planning pack (200 DRAFT VG goals) lives at `docs/execution/pack/`.
  VG IDs are not TASK numbers. Lanes vs Kimi:
  `orchestration/CURSOR_KIMI_LANES.md`. Crosswalk:
  `docs/execution/CROSSWALK.md`. Baseline HEAD `486058f3`.
- Native HUD: web-token skin, Pixelmix, wizard orb plates, hover
  tooltips, authoritative XP bar (`state.xp` on the snapshot). Cursor
  claims `native/client/**` until released.
- Evidence: `hud-pane-readability` PASS (0 failures) with isolated
  captures under `docs/execution/captures/hud-wave/`.
- Uncommitted; owner pushes. Do not duplicate TASK-0108 / Owner Demo.

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
