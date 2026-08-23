# TASK-0115 FINDINGS — Browser panel and typography inventory

Lane `ox-pc-bc` · base commit `d2423873c577d299b3b39c56024d1d840993c72b` ·
branch `worker/verdigris/pc/ox-pc-bc` · 2026-08-23.

Inventory only. **No file outside
`orchestration/tasks/TASK-0115-browser-panel-typography-inventory/**` was
created or modified**; no font choice was made and no native/browser behavior
was changed. Machine-readable registry: `captures/panels.json`.

## 1. What was produced

| Deliverable | Status |
| --- | --- |
| 19 persistent/situational panels identified, triggered, visibility-proven | 19/19 GREEN |
| Hard-fail capture per panel per viewport (1920x1080 + 1366x768) | 38 PNGs in `captures/` |
| Measured bounding boxes + computed typography per panel/viewport | `captures/capture-0115-evidence.json` |
| Assertion transcript (`CAPTURES OK`, 38/38 true, exit 0) | `captures/capture-0115-run.log` |
| Negative control (injected false assertion → exit 1) | `captures/capture-0115-negative-control.log` |
| Static sweeps (typography 827 lines, components 626 lines, @font-face map) | `captures/rg-typography-sweep.txt`, `captures/rg-components-sweep.txt`, `captures/rg-fontface-sweep.txt` |

No panel is RED. The fallback path ("finish static inventory, mark uncaptured
panels RED") was not needed.

## 2. The frozen browser typography contract

Three declared bitmap faces (`src/assets/scss/typography/fonts.scss:2-18`)
plus the body token (`src/assets/scss/abstracts/_tokens.scss:60-65`):

| Token | File | Role in live UI |
| --- | --- | --- |
| `GameFont` | pixelmix.ttf (+bold) | structural default: body, pane chrome, headings, buttons, HUD numerals (`_tokens.scss:61`) |
| `ChatFont` | PxPlus_IBM_VGA8.ttf | dense prose: chat messages (`Chatbox.vue:494`), guide banner body (`GuideBanner.vue:102`), labels/hints/tooltips across Quickbar, ItemTooltip, SettingsBindings, Quests, Logout, PartyPanel, Chronicles/auth |
| `UIFont` | Px437_IBM_PS2thin2.ttf | **declared but referenced by zero components** — a dead face in the current contract |

Measured rendered values (computed style at each anchor, both viewports):
the scale is fluid via `--font-size-base: clamp(14px, 1.1vw, 16px)` — every
token-sized surface measures **16px at 1920** and **15.026px at 1366**
(1.1vw). Weight is uniformly **400** except quickbar slot values
(`Quickbar.vue:358`, weight 600). Body color is uniformly
`rgb(238,226,197)` = `#eee2c5` (`--color-text-primary`); accent text leans on
`#e7c570` / `#efe6cf`; the chat peek pill is `#f2d391`
(`GameContainer.vue:1289`). Two micro surfaces are hard-pinned **12px**
(escape the clamp): the chat peek label and context-menu actions
(`ContextMenu.vue:300`).

### Deviations worth freezing before native porting

1. **Serif "moment" overlays**: `LootMoment.vue:109` and
   `DeathOverlay.vue:168` root their cards in `Georgia, serif` — the only two
   panels whose measured family is not GameFont. They are deliberate
   narrative-register exceptions, with GameFont titles layered back on top
   (`LootMoment.vue:125`, `DeathOverlay.vue:210`).
2. **Zone-note prose** uses Georgia at 0.64rem inside the expedition menu
   (`GameContainer.vue:1256-1261`); loading-screen subline too
   (`GameContainer.vue:1030`). Same "voice" as (1): Georgia = narration.
3. **`UIFont` is orphaned** — nothing consumes it. Any native font bundling
   decision can drop it from the contract without browser parity loss.
4. **12px pins** (chat peek, context menu) sit below the fluid floor of
   14px; native must reproduce them as fixed sizes, not clamped tokens.
5. ChatFont carries an italic variant usage in legacy panes only (Shop,
   Chart, Wagon) — all four game-panes are unmounted legacy code (see §5),
   so italic ChatFont is effectively out of the live contract.

## 3. Panel inventory (19)

Full machine rows incl. gameplay-load notes and CSS citations:
`captures/panels.json`. Sizes are measured bounding boxes (px), not
approximations read from CSS.

| # | Panel | Persistence | Trigger | Anchor | Size 1920x1080 | Size 1366x768 | Mounted Vue path (citation) | Rank |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | HUD chrome (identity, orbs, quickbar, XP) | persistent | always in game | `.game-container__hud` | 1852x97 | 1212x91 | layout/GameHUD.vue ← GameContainer.vue:251; hud/HudOrb.vue, hud/Quickbar.vue | 1 |
| 2 | Inventory pane | situational (docked R) | `i` hotkey / EscapeMenu | `[aria-label="Inventory panel"]` | 1240x968 | 680x490 | Delaford.vue:124 registry → PaneHost.vue:43-65 | 2 |
| 3 | Chatbox expanded | persistent state | `/` key / peek click | `.chatbox` (expanded overlay) | 320x247 | 280x256 | Chatbox.vue ← GameContainer.vue:208-249 | 3 |
| 4 | Chat peek pill | persistent state | collapsed by default | `.game-container__chat-peek` | 340x29 | 280x29 | GameContainer.vue:167-207 | 4 |
| 5 | World minimap | persistent | always (stage free) | `.world-minimap` | 168x183 | 168x183 | hud/WorldMinimap.vue ← GameContainer.vue:45 | 5 |
| 6 | Panel nav pills | persistent | always | `.game-container__pane-menu` | 247x37 | 280x37 | GameContainer.vue:58-104 | 6 |
| 7 | Expedition zone menu | situational | Adventure button | `[aria-label="Choose a zone"]` | 443x392 | 280x278 | GameContainer.vue:121-148 | 7 |
| 8 | Guide banner | persistent while active | tutorial beat / setGuide hook | `.guide-banner` | 500x55 | 500x55 | world/GuideBanner.vue ← GameContainer.vue:49 | 8 |
| 9 | Character pane | situational (docked L) | `c` hotkey / EscapeMenu | `[aria-label="Character panel"]` | 922x968 | 656x490 | slots/Stats.vue ← Delaford.vue:123 | 9 |
| 10 | Quest journal | situational overlay | `j` / nav / EscapeMenu | `[aria-label="Quests overlay"]` | 560x314 | 560x314 | slots/Quests.vue ← Delaford.vue:127 | 10 |
| 11 | Escape menu | situational | Escape with all layers closed (Delaford.vue:1265) | `.escape-menu` | 480x371 | 480x370 | panes/EscapeMenu.vue ← Delaford.vue:129 | 11 |
| 12 | Settings overlay | situational | Settings nav | `[aria-label="Settings overlay"]` | 560x567 | 560x490 | slots/Settings.vue + ui/SettingsBindings.vue ← Delaford.vue:125 | 12 |
| 13 | Logout overlay | situational | Exit nav | `[aria-label="Logout overlay"]` | 560x325 | 560x325 | slots/Logout.vue ← Delaford.vue:126 | 13 |
| 14 | Party panel | situational | Party toggle / incoming invite | `.party-panel` | 300x101 | 280x101 | world/PartyPanel.vue ← GameContainer.vue:105-120 | 14 |
| 15 | Roads chart menu | situational | Roads nav | `[aria-label="Choose a road"]` | 250x241 | 280x241 | GameContainer.vue:149-165 | 15 |
| 16 | First-find loot moment | situational, auto-dismiss 7s | first curated drop / showLoot hook | `[aria-label="First find"]` | 440x159 | 440x159 | ui/LootMoment.vue via GameCanvas seam (GameCanvas.vue:8-25) | 16 |
| 17 | Death overlay | situational | death summary event / showDeath hook | `.death-overlay__panel` | 720x458 | 720x376 | world/DeathOverlay.vue ← GameContainer.vue:267 | 17 |
| 18 | Context menu | situational | Shift+RMB canvas (plain RMB = ability-1, controls.js D-007) | `#context-menu #actions` | 156x76 | 156x76 | sub/ContextMenu.vue ← GameContainer.vue:266 | 18 |
| 19 | Skill tree overlay | situational fullscreen | `p` hotkey / EscapeMenu | `[aria-label="Skill Tree overlay"]` | 1904x968 | 1350x490 | passives/GeometricSkillTreePane.vue ← Delaford.vue:128 | 19 |

Typography per row is recorded in `captures/panels.json` (`font_measured` +
`font_source_refs`); headline pattern: GameFont/400/#eee2c5 everywhere, 16px
@1920 vs 15.026px @1366, with the §2 deviations.

## 4. Capture evidence protocol

Driver: `captures/capture-0115.mjs` through the shared hard-fail helper
`tests/e2e/lib/capture-harness.mjs` (owned server lifecycle, Chronicles/guest
login, `boxOf` visibility proof, `<prefix>-<viewport>-<label>.png` naming,
hard-fail JSON exit semantics). One disposable capsule port: **6620**
(loopback only; 6500 untouched). Production build from this branch's source;
browser behavior unchanged.

Per panel the driver (a) triggers the real production path (hotkeys, nav
buttons, bus hooks registered for captures in `GameContainer.vue:776-809`),
(b) waits for the named anchor to be visible and records its bounding box
(`boxOf` — a null box fails the run), (c) probes computed typography, (d)
screenshots. `runCapture` then requires **all 38** `<viewport>.<panel>-visible`
checks true before exiting 0 — transcript ends
`CAPTURES OK {…}` (`capture-0115-run.log`).

Context-menu note (behavior preserved, no source change): plain RMB is bound
to ability-1 (`src/core/config/controls.js:45-52`, D-007); the menu trigger is
Shift+RMB (`GameCanvas.vue:345-350`). CDP-injected Shift+RMB never reached the
Vue handler, so the driver dispatches the same trusted-shape DOM
`MouseEvent('contextmenu', {shiftKey:true})` the browser produces; the server
answered a real `player:context-menu:build` round-trip with `Walk here` +
Cancel and `#context-menu #actions` mounted (hit at 966,540 / 689,384).

Negative control: rerun with `NEGATIVE_CONTROL=1 SKIP_BUILD=1 CAPTURE_PORT=6621`
redirects all output into disposable `captures/.tmp-negative-control/` and
injects one false visibility check. Result: `CAPTURE FAILED:
negative-control.false-visibility-inventory`, **exit 1**, evidence JSON still
written — the hard-fail path genuinely fails. Disposable directory removed
afterwards; transcript kept at `capture-0115-negative-control.log`.

## 5. Exclusions (cited, not captured)

- `game-panes/Bank.vue`, `Chart.vue`, `Shop.vue`, `Wagon.vue`: imported by no
  component under `src/` (`rg "from.*game-panes"` → no matches). Legacy,
  unmounted; they carry the only italic-ChatFont usages left in the tree.
- Auth flow surfaces (`Login.vue`, `ChroniclesScreen.vue`,
  `AccountCreate.vue`, `CharacterCreate.vue`, `AuthContainer.vue`,
  `AudioMainMenu.vue`, `LoginBackdrop.vue`): pre-game screens outside the
  "persistent/situational game panel" scope; their typography (ChatFont-dominant)
  is cited in the sweep files for whoever freezes the auth contract later.
- Tooltips ride along in captures (inventory shot) but were not separately
  hovered-out; their styles are cited (`ItemTooltip.vue:98-236`,
  `InventoryItemTooltip.vue:194-220`).

## 6. Native phasing proposal (owner input reserved on fonts)

Rank order in §3 = proposed reconstitution order: P0 first-playable chrome
(HUD, inventory, chat both states) → P1 navigation wave (minimap, nav pills,
zone menu, guide banner) → P2 session panes (character, quests, escape,
settings, logout, party) → P3 situational moments (roads chart, loot moment,
death, context menu) with the bespoke fullscreen skill tree last. This is an
ordering recommendation only; final font selection remains owner-only per
SPEC.
