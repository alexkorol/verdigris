# FINDINGS — TASK-0093: Native typography and text-rendering contract audit

- **Lane:** ox-pc-bc · **Model:** openrouter/stealth/ox-alpha
- **Base SHA:** `d2423873c577d299b3b39c56024d1d840993c72b` (ancestor-verified of worktree HEAD)
- **Claim head:** `0a8aa40e` on `worker/verdigris/pc/ox-pc-bc`
- **Frozen invariants respected:** render-list determinism (byte-equal JSON across runs,
  `native/client/main.cpp:6308-6320`) and D-113 art authority
  (`orchestration/DECISIONS.md` D-113). **No font was downloaded, generated,
  licensed, or selected. No renderer was chosen.** This document defines the
  contract surface only.
- **Machine-readable coverage:** `captures/coverage.json`

---

## 1. Current native text operations

Every visible string in the native client is drawn by GDI inside
`native/client/main.cpp`. There is no text anywhere else in `native/client`
(`rg -l "TextOutA|DrawText" native/client` → `main.cpp` only).

### 1.1 Primitive inventory

| Primitive | Where used | Citation |
| --- | --- | --- |
| `TextOutA` (ANSI, single-line) | every drawn string | ~40 call sites, all in `native/client/main.cpp` |
| `GetTextExtentPoint32A` (measure) | chip sizing, planner layout, legend centering, trace rects | main.cpp:2097, 2158, 2192, 2281, 2304, 2314, 3550, 3662-3679 |
| `CreateFontA` + `SelectObject` | only 3 sites create explicit fonts; everything else inherits the memory DC's stock font | main.cpp:1741, 3002, 3544 |
| `SetTextColor` / `SetBkMode(TRANSPARENT)` | per-call-site inline `RGB()` literals (~30 distinct values) | e.g. main.cpp:2153, 2180, 2478, 3035-3045 |
| Fixed pixel offsets | text placement is hardcoded (`left+14`, `top+12`, `base.x - 9`, y steps 26/34/44) | main.cpp:2163, 2197, 1750, 3011 |

### 1.2 Explicit fonts today

| Site | Face / size | Charset / quality | Notes |
| --- | --- | --- | --- |
| Damage numbers (main.cpp:1738-1753) | Verdana bold, height `clamp(kTileUnits·0.34·scale, 13, 22)`; critical `clamp(…0.44…, 16, 26)` | `DEFAULT_CHARSET`, `DEFAULT_QUALITY` | drawn at fixed offset `base.x - 9` — **not centered**, no outline |
| Beat-legend chips, capture-only (main.cpp:3544-3553) | Verdana bold 13 px | `DEFAULT_CHARSET`, `DEFAULT_QUALITY` | extent-measured, centered chip |
| Chronicles front door lines (main.cpp:3002-3012) | Georgia bold 19 px body / 30 px accent | `ANSI_CHARSET`, `CLEARTYPE_QUALITY` | fixed line steps 26/44 px; loop breaks at `bounds.bottom - 40` |

**Contract hazard:** the remaining majority of HUD/pane/chip/toast/debug text
(gear pane main.cpp:2150-2319, status chips 2094-2117, quickbar 2477-2482,
connection chip 3027-3077, top-HUD rows 3720-3775, debug overlay 3790-3834)
selects no font at all — it renders with whatever stock font the offscreen
memory DC carries, which varies across Windows versions and machines.

### 1.3 Render-list ops that carry text labels

Defined once in `native/client/render_list.hpp:16-45`; labels are plain ASCII
`std::string`: `Hud`, `PaneStat`, `PaneWeapon`, `PaneItem`, `PaneBanked`,
`Chronicles`, `HouseChip`, `Quickbar`, `Damage` ("player"/"monster"/
"critical:<style>"), `Drop` (item id), `Extraction` ("stairs-up"), plus
non-pane labels on `Tile`/`Floor`/`Orb`/`Minimap`/`Telegraph`/`TargetFlash`.
The render list is a semantic twin of the frame — it records *that* text with a
position, never glyph geometry — and its determinism is already test-locked.

## 2. Browser text roles (historical reference)

Browser typography is tokenized and role-separated:

| Role | Font family | Size / weight | Color treatment | Citation |
| --- | --- | --- | --- | --- |
| Canvas combat numbers | GameFont (pixelmix) | `600 max(13, 15·scale)px`, centered | fill color-coded (crit #fff176, bane #8de6a5, blocked #8bd5ff, player #ff5252, monster #ffd54f) **+ black stroke outline rgba(0,0,0,.85), width ≥ 3** | src/core/rendering/perspective-renderer.js:964-1018 |
| Legacy flat-map numbers | GameFont | `600 12px`, centered | same stroke treatment | src/core/map.js:1715-1737 |
| Loot beam label | GameFont | `600 max(11, 12·scale)px`, centered | #f1d58d + black stroke | perspective-renderer.js:704-716 |
| Death overlay headline | GameFont / Georgia serif | 1.8rem / 0.72rem | white-on-dark | perspective-renderer.js:896-900; src/components/ui/world/DeathOverlay.vue:168,286 |
| Chat log | ChatFont (PxPlus IBM VGA8) | `var(--font-size-sm)` = clamp(12px,1vw,14px) | wrapped (`overflow-wrap:break-word`), scrollback `max-height:320px` | src/components/Chatbox.vue:439-441,468,494-511 |
| Panel titles / pane body | GameFont via CSS | tokens 8px–1rem, weights vary | CSS colors, ellipsis truncation | src/components/ui/panes/PaneCard.vue:122; src/components/slots/Stats.vue:450,520,642,675 |
| Tooltip flavor line | Georgia serif | 0.72–0.82rem | italic-style flavor | src/components/inventory/InventoryItemTooltip.vue:194,220 |
| Auth screens | ChatFont subtitles + GameFont headings | fluid clamp tokens | — | src/components/layout/AuthContainer.vue:269-379 |
| Minimap labels | `var(--font-pixel, monospace)` fallback chain | 9-11px | — | src/components/hud/WorldMinimap.vue:255 |
| Fluid size tokens | — | `--font-size-base/sm/lg = clamp(14px,1.1vw,16px)/clamp(12px,1vw,14px)/clamp(16px,1.3vw,18px)` | viewport-responsive | src/assets/scss/abstracts/_tokens.scss:30-32 |

Font files shipped by the browser reference:
`src/assets/fonts/pixelmix.ttf`, `pixelmix_bold.ttf`,
`PxPlus_IBM_VGA8.ttf`, `Px437_IBM_PS2thin2.ttf`
(declared in `src/assets/scss/typography/fonts.scss:2-18`; note `UIFont`
→ Px437 has **no consumer** — dead declaration). Their licensing/provenance is
exactly the owner decision this audit must not make.

Upstream panel inventory TASK-0079 (`orchestration/tasks/TASK-0079-browser-panel-inventory/SPEC.md`)
is still SPEC-only — no FINDINGS/captures exist yet to cite; the second
acceptance command confirms only the SPEC references 1920x1080/typography today.

## 3. Resolution / DPI requirements

- **Native capture matrix exercised by tests:** 960×600
  (`scenario_present`, main.cpp:4148-4159; front-door/HUD evidence 4935-4937,
  5051-5053), 1366×768 (main.cpp:5365-5367, 6338-6340), 1920×1080
  (main.cpp:6303, 6330-6336).
- **DPI awareness:** `rg -i dpi native/client` finds **zero** calls
  (`SetProcessDpiAwareness*`, `GetDpiForWindow`, per-monitor manifests absent).
  GDI text therefore scales with system DPI virtualization, blurring on
  scaled displays; font pixel sizes are absolute.
- **Layout robustness:** the measured-extent top-HUD planner
  (`plan_top_hud`, main.cpp:3080-3202) already guarantees no region collides
  with minimap/quickbar/orbs/gear pane "at any width, including 960x600 and
  1366x768", with a deterministic fallback ladder and a two-line wrap for the
  controls hint (main.cpp:3109-3115, 3639-3757). This is the model the
  contract should generalize: **measure → place → wrap → draw**, pure integer
  geometry, painter follows planner.
- **Browser pins:** benchmark screenshots are pinned at 1920×1080
  (`orchestration/benchmarks/side-by-side-2026-08-20/browser-scenes.mjs:16`);
  TASK-0079 specifies panel anchors "at 1920x1080"; browser type itself is
  fluid via clamp() tokens (_tokens.scss:30-32).

## 4. Glyph ranges (requirements, not font choice)

Today's native strings are ASCII-plus-one: `"CONNECTION LOST — not playing
offline"` (main.cpp:3071) contains U+2014 EM DASH, drawn through
`TextOutA`/ANSI codepage — it works only while the ANSI charset maps it, which
is exactly the latent class of bug the contract must close. House/Scion/item
names arrive over the protocol and can contain anything the server accepts.

Required minimum ranges for the native text layer:

1. Basic Latin + Latin-1 Supplement (accents in names: á é í ó ü ñ ç).
2. Typographic punctuation: — – ’ ‘ “ ” … · ° ± ×.
3. Digits with tabular figures (damage numerals must not jitter width).
4. Explicitly out of scope until owner input: CJK, Arabic/Hebrew shaping,
   Indic complex scripts, emoji. The contract should fail loudly (measurable
   substitution flag) rather than silently tofu these.

## 5. Wrapping, alignment, clipping — current state vs required contract

| Behavior | Browser reference | Native today | Required contract |
| --- | --- | --- | --- |
| Wrapping | CSS flow + `overflow-wrap:break-word` (Chatbox.vue:468); canvas never wraps | one deliberate seam: controls hint wraps at the " \| " boundary nearest middle into two stacked lines (main.cpp:3639-3659, 3741-3754); Chronicles lines hard-break at bottom (3012); everything else single-line | backend-neutral `wrap(text, max_width_px) -> lines[]`, greedy on word boundaries, deterministic tie-breaking; callers must never emit a line wider than budget |
| Alignment | canvas `textAlign='center'` everywhere (perspective-renderer.js:998 etc.) | damage numerals at fixed `-9` px x-offset (main.cpp:1750) — mis-centers multi-digit crits; legend chips center via measurement (3552); everything else left-aligned | first-class left/center/right with baseline-correct vertical metrics; damage numerals centered by measure |
| Clipping/truncation | CSS `text-overflow:ellipsis` (Stats.vue:450,520,642,675; GeometricSkillTreePane.vue:1056; GameHUD.vue:142; GameContainer.vue:1333) | character-count heuristic: `name.size() > 12 → substr(0,11)+"."` (main.cpp:2259) — metric-blind; connection chip draws into fixed 168×22 box with **no clipping** (3025-3066); pane cells can overflow cell_w | pixel-measured `truncate_to_width(text, max_px) -> {text, ellipsized}`; clipped draws declare clip rect in the op |
| Contrast aids | black stroke outline behind all world-anchored canvas text (perspective-renderer.js:999-1000) | none — colored text straight onto terrain (damage numbers main.cpp:1748-1751) | world-anchored text requires an outline/shadow parameter so backends render identically |

## 6. Accessibility risks (measured, WCAG 2.1 relative-luminance ratios)

Computed from the literal RGB pairs in source (script + results preserved in
`captures/coverage.json`):

| Pair (fg on bg) | Ratio | Risk |
| --- | --- | --- |
| Pane title RGB(230,235,220) on RGB(25,33,37) — main.cpp:2153 | 13.44 | fine |
| Quickbar available name RGB(205,221,207) on RGB(35,42,44) — 2481 | 10.31 | fine |
| Gold accent/key labels RGB(239,208,116) — 2478 | 10.85 | fine |
| Muted stats RGB(150,170,158) on panel — 2180 | 6.64 | passes AA small-text |
| Item bonus RGB(150,165,152) on cell — 2266 | 5.97 | passes AA |
| Seat label RGB(170,190,178) — 2215 | 7.66 | fine |
| **Quickbar unavailable skill RGB(112,119,115) on RGB(29,33,34)** — 2481 | **3.54** | below AA 4.5:1 at ~11px — availability state carried only by color+dimness |
| **Player damage RGB(255,118,104) over mid terrain (sample 74,74,58)** — 1724 | **~3.45 worst-case** | no outline; contrast collapses over bright/sand terrain; browser solves this with a ≥3px black stroke |
| Debug muted RGB(150,160,150) over uncontrolled world — 3812 | variable | debug-only, acceptable if flagged |

Risk summary: (R1) state-by-color-only dimming on quickbar; (R2) no outline
behind world-anchored text; (R3) fixed-pixel sizes give owners on high-DPI
scaled displays blurred text (no DPI awareness); (R4) ANSI-only pipeline will
silently mangle non-Windows-1252 names.

## 7. Offscreen-capture determinism

- **Locked today:** render-list JSON byte-equality across two full runs of
  every reference scene (main.cpp:6308-6320); PNG evidence written through a
  32-bpp top-down DIB section + GDI+ (`reference_present` 5814-5834,
  `save_hbitmap_png` 5801-5812) at 960×600/1366×768/1920×1080; capture writes
  contained via `VERDIGRIS_CAPTURE_ROOT` (2726-2841).
- **Not deterministic today:** pixels. Text rasterization depends on
  ClearType-vs-grayscale resolution inside memory DCs, Windows version stock
  fonts (the unstyled majority text), `DEFAULT_CHARSET`/`DEFAULT_QUALITY`
  defaults, and GDI font linking. Two machines produce different PNGs around
  glyphs; even the em-dash path differs by codepage.
- **Contract requirement:** captures must remain honest evidence without
  pretending pixels are portable — keep semantic determinism in the render
  list, add explicit rasterizer settings (fixed anti-alias mode, no subpixel
  ClearType for capture paths, app-shipped versioned font file rather than OS
  stock fonts) so cross-machine pixel drift shrinks to zero where feasible.

## 8. Windows/macOS backend needs (neutral requirements)

The current implementation is Windows-GDI-specific (`TextOutA`,
`GetTextExtentPoint32A`, `CreateFontA`). Whatever backend wins later — staying
GDI, DirectWrite, CoreText, or FreeType+atlas — must be able to provide:

1. **Measure:** `metrics(font_id, size_px, weight) -> {advance-per-glyph,
   ascent, descent, line_height}`; `width(text)`.
2. **Draw single line:** position + alignment + color + optional outline,
   UTF-8 in, no platform charset conversion (replaces the TextOutA boundary).
3. **Wrap / truncate:** the §5 functions implemented once, above the backend,
   consuming only `width()` — identical results on any backend.
4. **Deterministic rasterization:** explicit AA/hinting mode; font bytes
   loaded from an app-owned, version-registered file (id-stable), never OS
   lookup-by-name as first resort.
5. **DPI:** sizes expressed in logical px with an explicit scale factor
   supplied by the platform layer (enables per-monitor v2 on Windows, Retina
   backing on macOS) instead of baked pixel heights.
6. **Glyph coverage query:** `has_glyph(cp)` or substitution flag so the
   required ranges (§4) are testable without pixels.

## 9. Recommended backend-neutral text contract (for successor tasks)

```text
Role        : enum { Title, Body, Muted, Accent, WorldAnchored, InputEcho }
TextOp      : { role, utf8_text, x, y, max_width?, align, size_class,
                weight, color_token, outline?: {color, width_px} }
Surface API : measure(text, style) -> Size
              wrap(text, style, max_width) -> string[]
              truncate(text, style, max_width) -> string   // adds "…"
Invariants  : I1 every drawn string has exactly one TextOp in the render list
              I2 render-list order/JSON stays byte-deterministic (existing lock)
              I3 wrap/truncate consume only measure(); no backend types leak
              I4 world-anchored ops always carry an outline
              I5 color comes from named tokens; each (token, surface) pair is
                 table-checked >= 4.5:1 (or documented >= 3.0 large-text exception)
              I6 UTF-8 end-to-end; unsupported codepoints surface as a flagged
                 substitution, never silent mojibake
```

This preserves the existing discipline (planner measures, painter obeys,
render list records) and leaves font identity, file format, and rasterizer as
plug-in decisions behind `measure/draw`.

## 10. Proposed locking tests (backend-neutral)

| # | Test | Asserts | Seam |
| --- | --- | --- | --- |
| T1 | Role completeness sweep | For front door, expedition HUD, gear pane open, chronicles menu: every visible string appears as a render-list op tagged with its role; counts equal across two consecutive presents | extend `render::Item.label` with `role:` prefix or new field; scenarios assert presence (pattern of presentation_events_tests.cpp:95-101) |
| T2 | Wrap determinism | Controls hint at 960×600 wraps to ≤ 2 lines; every returned line's `measure()` ≤ budget; same output twice | new `wrap()` called over real frames like plan_top_hud (main.cpp:3697) |
| T3 | Truncation fit | For long item names: truncated result fits cell width by measure; ends with "…" when shortened; replaces the char-count rule (main.cpp:2259) | unit + scenario over gear pane cells |
| T4 | Damage centering | 1-, 2-, 3-digit damage numbers share measured centers (±0 px) instead of fixed −9 px (main.cpp:1750) | render-list x positions vs measured widths |
| T5 | Contrast table lock | Every (color_token, surface) pair meets ratio threshold computed from constants; quickbar-unavailable pair (3.54) must either gain a token fix or a recorded exception | constants-only unit test |
| T6 | Glyph-range smoke | Every codepoint in the §4 required set returns nonzero advance / `has_glyph`; em-dash renders through UTF-8 path, not ANSI hope | measure-level check |
| T7 | Capture honesty | Inside each recorded `hud_rect_trace` rect, the offscreen PNG contains non-background pixels (text actually rasterized where claimed) | existing `hud_rect_trace` seam (TASK-0159) + DIB readback |
| T8 | Determinism extension | Existing scene JSON equality (main.cpp:6316-6320) extended to require equality of all text-op fields including roles/outlines | run_reference_scenes harness |

## 11. Owner-only choices (audit does NOT decide)

1. **Font family selection** for native UI/world text.
2. **License approval** for any candidate (including the browser-shipped
   pixelmix/PxPlus_IBM_VGA8/Px437 files — provenance/licensing unresolved;
   UIFont/Px437 currently unused even in the browser).
3. **Renderer/backend** (GDI continuation vs DirectWrite/CoreText-backed).
4. **Unicode scope expansion** beyond §4 minimum.
5. **Contrast target** (AA 4.5:1 blanket vs documented exceptions) and whether
   quickbar-unavailable gets non-color affordance.
6. **Chat scope** for native (see negative control) — build now or defer.

## 12. Negative control (required by spec)

**Persistent chat log.** The browser renders an always-available scrolling
chat: `src/components/Chatbox.vue` (log area `overflow-y:auto; max-height:320px`
at 439-441, wrapped with `overflow-wrap:break-word` at 468, ChatFont at 494).
The native client has **no equivalent**: `render::Op`
(`native/client/render_list.hpp:16-45`) carries no chat operation, and
`ClientState` has no chat buffer or input seam. Any native pane-completion wave
that assumes chat parity must first introduce the op + layout contract above.
(Second-order gap, same shape: tooltip flavor text
(`InventoryItemTooltip.vue:220`, Georgia serif) has no native op.)

---

*Transcripts of the literal acceptance commands are in `REPORT.md`; raw sweeps
in `captures/`.*
