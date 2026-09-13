# TASK-0118 — Native accessibility, options, and input audit

Lane: ox-pc-bd · Model: openrouter/stealth/ox-alpha · Branch: `worker/verdigris/pc/ox-pc-bd`
Base: `9bd689b4cebac0fe1f79ba54edcc9967a1a8f0d4` (verified ancestor of the audited branch head; all citations are to that descendant tree)
Scope: mechanical audit only. No option default is chosen, no balance value is
touched, and D-007 (`orchestration/DECISIONS.md:30-40`) is preserved as the
frozen control contract. Recommendations are content-neutral contracts (an
option may exist, a state must be persistable), never owner-only verdicts.

Machine-readable mirror: [`captures/accessibility-matrix.json`](captures/accessibility-matrix.json).

## 0. Frozen invariants honored

| Invariant | Source | Status in this audit |
|---|---|---|
| D-007 control contract (WASD, mouse aim, LMB/RMB, Space, Q/E/R skills, X/Z/F/I-Tab utilities) | `orchestration/DECISIONS.md:30-40`; constitution `docs/product/VERDIGRIS_CONSTITUTION.md:94-101` | Recorded as-is. Every gap below is phrased so closing it cannot move a D-007 binding or semantic |
| Pane system + two minimap modes with transparency/zoom/side placement as options | constitution `docs/product/VERDIGRIS_CONSTITUTION.md:108-111` | Native ships one fixed minimap mode only (§11) |
| Presentation requests / simulation resolves / events → display | constitution `VERDIGRIS_CONSTITUTION.md:157-168` | All accessibility gaps live client-side; none require touching core authority |
| Resource capsule read-only; no ports/settings mutation | SPEC frontmatter | Honored: zero files outside `orchestration/tasks/TASK-0118-accessibility-options-audit/**` |

## 1. Executive shape of the surface

- **Native client** (`native/client/main.cpp`, Win32/GDI): every binding is a
  hardcoded literal in `window_proc` (`main.cpp:3989-4107`) and `kSkills`
  (`main.cpp:909-913`). There is **no settings/options UI of any kind**
  (searching `setting|option|preference` in `native/client` hits only the
  Chronicles story menu), no persistence, no audio device, and one fixed
  minimap mode.
- **Browser reference** (`src/`): has a complete rebinding stack from TASK-0038
  (`src/core/config/controls.js`) with persistence, conflict refusal, reset,
  live subscribers, and 9 unit tests (`tests/unit/controls-bindings.spec.js`),
  plus a small settings pane (FPS cap, day-night toggle, sound-effects boolean:
  `src/components/slots/Settings.vue`). It still lacks volume sliders, text
  scale, contrast, reduced motion, sensitivity, and captions — i.e. the browser
  proves the *mechanisms* but not the *coverage*.
- **Native library seams ahead of the client**: `input_focus.hpp` (pure focus
  reducer) and `AudioMixer` (bus volume/mute) exist and are tested at library
  level but are wired into **neither** `main.cpp` nor any UI.

## 2. Key/mouse rebinding

| Surface | State | Evidence |
|---|---|---|
| Native client | **Absent** — all bindings are compile-time literals | `main.cpp:3989-4107` (`WM_KEYDOWN`/`WM_KEYUP`/mouse cases); skill table `kSkills {'Q',Thrust},{'E',Sweep},{'R',WarCry}` `main.cpp:909-913`; lookup `skill_for_key` `main.cpp:925-929` |
| Browser reference | **Present** — rebindable map incl. mouse buttons as first-class bindings | `src/core/config/controls.js` (`CONTROLS_STORAGE_KEY 'verdigris:controls:v1'`; `DEFAULT_ACTION_BINDINGS`; `MOUSE_BUTTON_TO_BINDING {0,1,2}`); UI capture flow `src/components/ui/SettingsBindings.vue:20-43` |
| Rebind propagation | Browser only — quickbar labels refresh on rebind | `src/components/hud/Quickbar.vue:114` ("Bumped whenever the player rebinds controls"); `subscribeBindings` in `controls.js` |

Gap contract (content-neutral): the native client needs a binding *map* with
collision detection between actions; defaults remain exactly D-007's.

## 3. Hold/toggle behavior

- Movement is hold-based key-state (`W/A/S/D` down/up pairs,
  `main.cpp:4007-4010` and `4071-4074`). Attacks fire once per press
  (`WM_LBUTTONDOWN` → `submit_action(Melee)` `main.cpp:4095-4103`); there is no
  hold-to-repeat and no press-and-hold variant anywhere.
- Toggles that exist: `Z` loot name labels (`main.cpp:4029-4032`, with text
  hint feedback "Loot names shown/hidden"), `I` gear overlay
  (`toggle_gear_overlay` `main.cpp:3961-3966`), `F3` debug overlay
  (`main.cpp:3991-3994`).
- **No hold/toggle preference exists**: nothing lets a player convert holds to
  toggles (movement, dash, aim) or repeat-on-hold, on either client. The
  browser's movement config carries only repeat timing constants
  (`MOVEMENT_REPEAT`, `src/core/config/controls.js`).

## 4. Sensitivity

- The only tunable-feeling input constant is the wheel zoom step factor
  `1.1` with fixed clamps (`main.cpp:4086-4093`,
  `kCameraMinZoom/kCameraMaxZoom`, `Home` resets zoom `main.cpp:4064-4066`).
- No mouse-pointer sensitivity, no camera invert, no dash-distance scalar, no
  double-click window option exists on either client
  (`rg sensitivity|invert|deadzone` over `native src` returns no feature code).

## 5. Focus (UI focus model)

- `native/client/input_focus.hpp` is a complete, pure, fail-closed focus
  reducer: surface stack (`Gear/Character/Passive/Modal/Text`), intents incl.
  `NavigatePrevious/NavigateNext/Confirm/Cancel`, disposition split
  Consumed / PassToGameplay / RequestQuit (`input_focus.hpp:21-59`,
  reducer `230-345`). Its own header states it has **zero integration with
  main.cpp** (`input_focus.hpp:12-14`), and `rg -ln input_focus
  native/tests native/client` matches only the header itself: **unwired and
  untested at the integration layer**.
- What actually ships: an ad-hoc gear-grid arrow/Enter/U navigator
  (`main.cpp:4041-4056`, legend line `main.cpp:2311`) and the TASK-0153
  Escape-dismisses-topmost contract (`handle_escape_key`,
  `main.cpp:3968-3978`). There is no visible focus/selection indicator model
  beyond the gear grid's selected index and no shared focus concept across
  HUD regions.

## 6. Text scale

- All native fonts are fixed pixel sizes: beat-legend chips 13 px
  (`main.cpp:3544-3547`), hints 19/30 px (`main.cpp:3002`), damage numerals
  clamped 13–22 px (crit 16–26 px) scaling only with *camera* zoom
  (`main.cpp:1730-1745`). No UI-scale/text-scale option exists on either
  client; the browser styles sizes via static SCSS tokens
  (`src/assets/scss/typography/sizes.scss`) with no runtime control.

## 7. Contrast

- Positive native evidence: bordered monster life bar with dark backing
  (TASK-0142, `main.cpp:3447-3448`); legend chips paint light-on-dark
  (`RGB(226,238,230)` text on `RGB(12,18,16)` chip, `main.cpp:3554-3565`);
  damage numerals fade toward the background deliberately over lifetime
  (`fade_to_background`, `main.cpp:1748`).
- No contrast/Gamma/high-contrast mode exists anywhere; nothing measures
  contrast ratios. The acceptance scan shows zero `contrast` hits outside
  tests/docs noise.

## 8. Color independence

Text/shape redundancy that already exists (keep these when adding options):

- Vital orbs carry numeric captions beside the fill (`life_caption` /
  `resource_caption`, `main.cpp:2370-2377`).
- Loot names render as text labels when toggled (`state.loot_labels`,
  draw site `main.cpp:3523`; toggle `main.cpp:4029-4032`).
- Elites differ from raiders by silhouette family, height (1.85 vs 1.58 tile
  units), and sprite, not just ring hue (`main.cpp:3421-3425`).
- Action feedback uses a text hint line (`show_hint`, `main.cpp:898-900`;
  e.g. "Blocked by scenery" `main.cpp:3927`, "Picked up nearest drop"
  `main.cpp:4025`) and the always-on controls line
  (`main.cpp:3634-3636`).

Color-only state conveyance (gaps):

- Minimap dots encode entity class by hue alone: extraction gold
  `RGB(239,208,116)` (`main.cpp:2528`) vs monsters red `RGB(196,58,48)`
  (`main.cpp:2535`), scenery grey-green (`main.cpp:2522`); no shapes, no
  legend.
- Player-damage vs monster-damage hit rings use identical yellow tones; only
  the hidden render-list label differs ("player"/"monster",
  `main.cpp:1767-1776`).

## 9. Reduced motion / flash

- None. The critical-hit treatment adds a near-white burst cross
  (`kCriticalFlashColor{255,246,214}`, `native/client/presentation_events.hpp:60`)
  plus a longer TTL (`kCriticalFlashTtlTicks = 6` ≈ 300 ms vs 200 ms,
  `presentation_events.hpp:53`), drawn at `main.cpp:1754-1764`; a
  `ScreenPulse` op exists in the render vocabulary (`main.cpp:5729`).
  There is no reduced-motion, flash-reduction, or screenshake-intensity option
  on either client, and no `prefers-reduced-motion` equivalent is consulted
  anywhere (`rg` over `src native`: zero feature hits).

## 10. Subtitles/captions

- Absent by construction: the native client plays **no audio at all** (§11),
  and the browser reference contains no caption/subtitle system either
  (the only `subtitle` token is an item-tooltip prop,
  `src/components/inventory/InventoryItemTooltip.vue:10,124`). Any future
  audio work inherits a caption obligation it currently has no seam for.

## 11. Audio controls

- Library seam present and tested: `AudioMixer` exposes per-bus volume in
  permille (clamped 0–1000) and mute (`native/audio/audio_mixer.hpp:54-59`,
  `audio_mixer.cpp:42-56`), with dedicated coverage incl.
  `bus_mute_and_volume` (`native/tests/audio_mixer_tests.cpp:235-257`;
  35 checks file-wide).
- Client wiring absent: `rg AudioMixer native/client/main.cpp` matches
  nothing — the shipped client instantiates no mixer and emits no sound, so
  players have no volume/mute control to operate.
- Browser reference: a single boolean `soundEffects`
  (`src/stores/ui.js:22,80`; toggle `Settings.vue:44-54`) persisted via pinia;
  no master/music/sfx volume sliders.

## 12. Minimap/pane options

- Native ships exactly one minimap presentation: fixed 108 px top-left panel
  (`paint_minimap`, `kSize = 108` `main.cpp:2486-2488`, reserved zone
  `{0,0,132,132}` `main.cpp:1474`). No large-overlay second mode, no
  transparency, no zoom, no side placement — while the constitution names two
  modes with those very options (`VERDIGRIS_CONSTITUTION.md:108-111`). This
  is the largest single product-contract gap found.
- Panes: one production pane (gear) plus the readability harness that keeps
  global HUD regions and the open pane pairwise disjoint at 960×600 and
  1366×768 (`scenario_hud_pane_readability`, `main.cpp:5427-5506`). No pane
  layout/diptych options yet (constitution diptych language remains future
  work; noted, not judged).

## 13. Difficulty-independent readability

- Neither client has any difficulty setting, so no readability aid is gated by
  difficulty today. Aids ship unconditionally: always-on controls line
  (`main.cpp:3634-3636`, wrapped deterministically `main.cpp:3639-3654`),
  objective strip with mode-aware extraction hint (`main.cpp:3625`,
  `extraction_action_hint`), text hint line, loot-name toggle, and the
  quickbar with binding labels (`main.cpp:2397`).
- Risk to watch: the VFX *beat legend* machinery renders only when entries
  exist; `beat_legend` is declared (`main.cpp:328`) and drawn
  (`main.cpp:3540-3572`) but populated only inside the animation capture
  scenario (`main.cpp:6223`) — live play runs with an empty legend, so VFX
  beats have no standing text explanation during real sessions.

## 14. Persistence

- Browser: bindings persist to localStorage under `verdigris:controls:v1`
  (`CONTROLS_STORAGE_KEY`, `src/core/config/controls.js`) with corrupt-payload
  fallback to defaults; fps/soundEffects/dayNight persist through the pinia
  store (`persist.paths` including `settings`, `src/stores/ui.js` last block).
- Native: **no settings/options persistence exists**. The only file writes in
  the client are capture artifacts (PNG billboards/captures,
  `save_hbitmap_png` `main.cpp:5801+`, scenario JSON dumps
  `main.cpp:6322`). Any option added needs a persistence seam that does not
  exist yet.

## 15. Reset

- Browser: full factory-reset path — `resetBindings()` repopulates defaults
  and persists (`src/core/config/controls.js`), surfaced as the Reset button
  in `SettingsBindings.vue:56-62`.
- Native: the single "reset" affordance is `Home` → restore default camera
  zoom (`main.cpp:4064-4066`). No settings exist to reset.

## 16. Keyboard-only navigation

- Working today: gear pane fully keyboard-operable (arrows select, Enter
  equips, U unequips, I closes — `main.cpp:4041-4056`, footer
  `main.cpp:2311`); number-row 1–9 equips backpack slots
  (`main.cpp:4058-4063`); Chronicles screens accept per-action hotkeys plus
  Enter for the primary action (`handle_chronicles_key`,
  `main.cpp:2709-2724`); Esc dismisses topmost before quitting
  (`main.cpp:3968-3978`).
- Structural limits: gameplay itself requires mouse aim + LMB/RMB by D-007
  (`DECISIONS.md:32-33`) — full keyboard-only *combat* is out of contract and
  is not requested here. But menus/HUD lack a traversal system: no Tab order,
  no arrow navigation in Chronicles lists, no focus ring outside the gear
  grid. `input_focus.hpp`'s Navigate/Confirm/Cancel intents are exactly the
  missing contract and remain unwired (§5). The quickbar, orbs, and minimap
  expose no keyboard path at all.

## 17. Test/capture coverage relevant to this audit

- Native scenario runner registers 12 scenarios
  (`run_scenarios`, `main.cpp:5682-5700`) including accessibility-adjacent
  proofs: `hud-pane-readability` (HUD-region disjointness + PNG evidence,
  `main.cpp:5427+`), `telegraph-dodge`, `zoom-invariance`,
  `first-session-clarity`. Reference scenes `01-route-entrance` …
  `05-critical-health` emit PNG + JSON render lists (`main.cpp:6283-6314`).
- Library tests: audio bus mute/volume (`audio_mixer_tests.cpp:235-257`);
  core/session/presentation suites exist but contain no binding/focus tests.
- Browser: `tests/unit/controls-bindings.spec.js` covers D-007 defaults,
  conflict refusal, add/remove/persist/reset, subscribers, display labels.
- Missing coverage: no test drives any rebinding, remapping, scale, volume,
  or focus-traversal behavior in the native client (nothing exists to drive),
  and `input_focus.hpp` has no direct unit test file in `native/tests/`.

## 18. Negative control (required by SPEC)

**Primary:** the elite attack telegraph is conveyed **only** by a translucent
red cone/wedge — thrust fill `visibility*0.38 × RGB(214,52,52)` with edge
`RGB(238,72,64)` (`draw_thrust_telegraph`, `main.cpp:1519-1543`), sweep
identical palette (`draw_sweep_telegraph`, `main.cpp:1572-1591`) — fading by
`telegraph_visibility` (`main.cpp:1594+`). It carries **no text label, no
legend entry** (live-play `beat_legend` is empty, §13), **no audio cue**
(the client plays no sound, §11), no colorblind-safe alternative, and no
option/rebinding/focus proof of any kind. A player with red-deficient vision,
playing muted-by-absence, receives dodge-critical information solely through
red hue + geometry. This is the audit's canonical negative control.

Secondary instances of the same failure class (recorded, not belabored):
minimap dot semantics are hue-only (`RGB(239,208,116)` vs `RGB(196,58,48)`,
`main.cpp:2528/2535`); the critical-vs-normal hit distinction leans on a
white burst cross + size jump whose only non-visual anchor is the numeral
itself (`main.cpp:1730-1764`).

## 19. Standards-first continuation path (content-neutral)

Ordered so each step is a contract, not an owner verdict:

1. Wire `input_focus.hpp` into `window_proc` behind intent translation and
   give every pane a focus indicator (the model already encodes Esc-close and
   modality contracts).
2. Port the browser binding-map mechanics (map, conflict refusal, reset,
   persistence key, live relabel) to the native client with D-007 literals as
   compiled-in defaults.
3. Add an options surface with persistence + reset covering, at minimum:
   master/bus volumes (permille buses already exist), text/UI scale, zoom-step
   sensitivity, telegraph/minimap redundancy flags (shapes+text alongside
   hue), reduced-flash mode for crit bursts/pulses, and minimap mode/placement.
4. Extend the scenario ladder: every new option gets a driven-input scenario +
   PNG/JSON capture per the established harness rules
   (`native/README.md:97-126`).
5. Caption/subtitle seam becomes mandatory the moment any audio is emitted.

None of these steps selects a difficulty, a balance number, or an owner-only
default; final defaults stay flagged for owner play verdicts per SPEC.
