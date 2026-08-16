# Verdigris 2.5D renderer overhaul plan — rev 2 (Phase 0 survey, TASK-0019)

Status: Phase 0 survey per `docs/reference/25d-overhaul/HANDOFF.md` ground rule 1.
Supersedes rev 1 of this document, which planned a greenfield port. That port has
since landed (the `src/core/rendering/perspective-*` stack), so this revision
re-surveys the CURRENT implementation against the reference and re-scopes phases
1–5 as conformance/remediation passes. The owner's verdict on the current state
("looks and plays awful… muddy, heavy fog/blur, low contrast, washed-out tiles",
root `HANDOFF.md`) and rulings D-107 (camera) and D-108 (the vendored demo is the
look/feel acceptance target) are the drivers.

Binding references:

- `docs/reference/25d-overhaul/docs/ARCHITECTURE.md` — coordinate model, camera
  math, shaders, pass order. Its §8 solved-bugs list is binding; no alternative
  solutions are invented below.
- `docs/reference/25d-overhaul/HANDOFF.md` — phase definitions and ground rules
  (feature-flag everything; renderer replacement, not game rewrite).
- `orchestration/DECISIONS.md` D-107: ARPG preset is the primary camera
  (pitch ~62, moderate perspective, no tilt-shift); miniature treatment blends
  in at close zoom; High Table rejected. Slice evidence-pack values
  (`orchestration/tasks/TASK-0012-slice-camera-evidence/REPORT.md`):
  ARPG = zoom 0.85, pitch 62, perspective 0.0006, anchor 0.52, fog 0.4, tilt 0.
- D-108: `docs/reference/25d-overhaul/dist/songs-of-the-mire.html` is the
  acceptance target for look and feel; its gameplay is throwaway.

## 1. Engine and stack findings

- **Stack**: Vue 3 (`package.json:56`) + Pinia, built with Vite 5
  (`vite.config.js:7-13`, aliases `@`→`src`, `@shared`→`server/shared`,
  `@server`→`server`). All world rendering is Canvas 2D plus one WebGL1 terrain
  canvas. There is no game engine; the renderer is hand-rolled.
- **Render loop**: `src/core/engine.js`. `Engine.loop` (`engine.js:46`) is a
  throttled `requestAnimationFrame` (60 fps cap, `SETTINGS:FPS` bus event at
  `engine.js:27`); each frame calls `paintCanvas(deltaSeconds)`
  (`engine.js:109-188`), which updates the map (`engine.js:110-112`) and then
  selects exactly one pipeline:
  - perspective: `map.drawPerspectiveFrame()` (`engine.js:114-119`);
  - legacy: `drawMap → drawGroundTelegraphs → drawItems → drawMonsters →
    drawNPCs → drawPlayers → drawPlayer → drawProjectiles → drawSkillEffects →
    drawAttackEffects → drawCombatFeedback → drawMouse` (`engine.js:120-168`).
  Both pipelines draw into an offscreen buffer; the engine blits buffer →
  visible canvas at `engine.js:174-187`.
- **Canvases**: one visible `<canvas id="game-map" class="main-canvas gameMap">`
  (`src/components/GameCanvas.vue:26-38`); a 2D buffer canvas
  (`src/core/map.js:99-100`); a WebGL1 terrain canvas
  (`src/core/rendering/terrain-renderer.js:33-38`, `preserveDrawingBuffer:true`)
  composited into the 2D buffer via `drawImage`
  (`perspective-renderer.js:118`) — matching reference §3. Lightmap/vignette
  (`lighting-renderer.js:53-56`) and sun-ray (`atmosphere-renderer.js:28-29`)
  offscreen canvases also exist.
- **Lifecycle**: `Client.buildMap()` constructs `Map`
  (`src/Delaford.vue:1739`); scene transitions construct a fresh `Map`
  (`src/core/client.js:167-224`); `Engine.stop`/`Map.destroy` are used on
  reconnect/unmount. GL resources must hang off this same lifecycle (they
  currently do, via the `PerspectiveRenderer` constructed at `map.js:115`).

## 2. Current perspective implementation vs the reference

The existing stack mirrors the reference architecture closely:

| Reference | Verdigris today | Where |
| --- | --- | --- |
| `projT`/`proj`/`unproj` | `project`/`projectTerrain`/`unproject`, identical equations | `perspective-camera.js:71-104` |
| `HOR = -0.45·H`, `FOCUS = 0.65·H` | same constants | `perspective-camera.js:47-48` |
| `zoom = max(W/1150, H/1500)·userZoom` | same, floored at `MIN_ZOOM` | `perspective-camera.js:49-52` |
| GPU heightmap mesh, one draw call | 161×161 grid over map + margin | `terrain-renderer.js:1-3,321-322` |
| Vertex shader = camera math exactly | mirrored (`projectWithShaderMath` for tests) | `terrain-renderer.js:86-114`, `perspective-camera.js:106-124` |
| POT texture + mipmaps + anisotropy | baked ground texture, `BAKE_TILE_SIZE = 16` | `terrain-renderer.js:4`, bake in `map.js:912-961` |
| y-sorted billboards, foot anchor | `draws.sort(depthY)`, bottom-center anchor | `perspective-renderer.js:130, 443-448` |
| Continuous sprite DoF | `ctx.filter = blur(...)` | `perspective-renderer.js:488-495` |
| Lightmap multiply, quarter-res | day cycle + clouds + lights | `lighting-renderer.js:4-15, 93-121, 153-175` |
| Sky + treeline before terrain | gradient + blurred silhouette | `perspective-renderer.js:179-207` |
| §8.1 zero-viewport guard | `MIN_VIEWPORT_SIZE` + `valid` flag | `perspective-camera.js:42-45, 61-63` |
| WebGL fallback | legacy ground + `alignLegacyGround` hack | `perspective-renderer.js:119-122, 157-177` |

So phases 1–5 of the brief each have a landed first pass. The overhaul ahead is
conformance: the current parameter set and grading diverge from the reference
and from D-107, producing the muddy look.

## 3. Coordinate / depth / anchor concept mapping

| Concept | Verdigris today | Reference mapping |
| --- | --- | --- |
| Gameplay position | integer tile `(x, y)`; collision from tile ids (`server/shared/ui.js:99-121`) | unchanged; rendering never feeds back into gameplay |
| Render world unit | interpolated pixels, 32 units/tile (`server/config.js:20-23`, `src/core/config/movement.js:3`); `centerOfTile()` at `src/core/utilities/movement-controller.js:11-14` | world unit = one source pixel (already true) |
| World extent | 200×200 tiles = 6400×6400 units (`server/config.js:40-43`) | unchanged |
| Depth axis | world pixel `wy`, larger = nearer; painter sort by foot `wy` (`perspective-renderer.js:130-131`) | matches reference §1 |
| Elevation | `heightAt` seam, stubbed to 0 (`perspective-renderer.js:68-70`, `terrain-renderer.js:29,204`, `perspective-camera.js:88`) | `h = terrainH(wx, wy)`; see §5 decision |
| Actor anchor | bottom-center foot in perspective (`perspective-renderer.js:443-448`); center-anchor 32px tile in legacy (`map.js:1090-1120`) | foot anchor correct; keep |
| Sprite scale | 64px source frames into 32px footprint (`src/core/config/animation.js:22-84`), `ACTOR_SCALE = 1.45` (`perspective-renderer.js:16`) | scale by projected `s`; already true |
| z-order | fixed pass order in legacy; single far→near `depthY` sort in perspective incl. vertical terrain (`perspective-renderer.js:267-297`) | matches reference |
| Mouse picking | perspective: `screenToWorld` → `camera.unproject` at h=0 (`GameCanvas.vue:478-489`, `map.js:194-199`); legacy: orthographic crop (`GameCanvas.vue:492-493`) | `unproj(sx, sy)`; already correct |
| Camera target | interpolated player foot (`perspective-renderer.js:72-95`); legacy pins player at viewport center (`map.js:1867-1874`) | `cam.x/y` lerp-follow; already true |

## 4. Gap analysis — why it looks muddy

Each item names the washing/blurring source, its location, and the conformance
target. This is the defect list phases 1–4 burn down.

1. **Playfield depth-of-field blur.** `dofStrength` is 0.32 even at default
   zoom (`perspective-camera.js:56-60`), so `circleOfConfusion`
   (`perspective-camera.js:126-133`) blurs sprites near the focus plane, and
   the terrain shader adds mip-bias blur (`terrain-renderer.js:125-129`).
   D-107 rejects tilt-shift at the primary zoom (ARPG `tilt 0`). Target: DoF ≈ 0
   across the playfield at default zoom; blur appears only as the close-zoom
   miniature blend (§6).
2. **Canvas-wide CSS grade.** `.main-canvas` carries
   `filter: brightness(1.12) contrast(1.08) saturate(0.9)`
   (`GameCanvas.vue:634-639`) over BOTH renderers. Its own comment says it
   exists to compensate for perspective resampling + multiply lighting
   crushing midtones — i.e. it is a global patch over the shader-side washing
   in item 3, and it desaturates everything including the playfield. Target:
   fix the grading at its source (item 3), then remove this filter (or scope
   it to legacy only if the legacy look depends on it).
3. **Terrain fragment grading.** Gamma lift `pow(colour, 0.82)`
   (`terrain-renderer.js:130`) plus desaturation cap (`:131-132`) lift and wash
   the ground before any atmosphere. The reference does its grading in the
   lightmap multiply, not in the terrain fetch. Target: neutral terrain fetch;
   grade lives in the lighting pass.
4. **Haze shape.** Current haze starts at `1.48·DZP` and caps at 0.24
   (`terrain-renderer.js:133-134`) — enough to grey the midfield, too weak to
   build a permanent horizon. Reference §3: haze MUST reach 1.0 ~5–6% from the
   top and be ~0 across the playfield. Target: adopt the reference curve
   (`clamp((dz/DZP − 1.12)/1.02, 0, 1) * 0.96`), tuned to the ARPG horizon.
5. **Per-sprite shadow blur.** `ctx.shadowColor/shadowBlur` on every billboard
   (`perspective-renderer.js:496-498, 358-360`) softens pixel art and costs
   per-sprite filter state changes. Reference uses flat foot ellipses only
   (which Verdigris also draws, `:451-460`). Target: drop shadowBlur; keep
   ellipses.
6. **Stacked atmosphere.** Mist blobs (`atmosphere-renderer.js:32-56`), god
   rays (`:121-130`), cloud multiply (`lighting-renderer.js:93-121`), vignette
   to `rgba(10,4,16,0.18)` (`lighting-renderer.js:74-91`) all compound on top
   of 1–5. Individually reference-faithful; collectively over-tuned for the
   ARPG preset's `fog 0.4`. Target: retune to the D-107 fog level after 1–5
   land, side-by-side with the demo.

## 5. Elevation-data decision

**Recommendation: keep flat `h = 0` as the shipped interim, and formalize the
existing `heightAt` callback as the single elevation contract.**

- Verdigris has no authored elevation layer or tile height metadata; maps are
  flat Tiled GID arrays (`server/maps/layers/surface.json`, two layers, 1-based
  GIDs; `server/maps/README.md`). Walkability derives from tile id lists
  (`server/config.js:24,33-34`), not geometry.
- The renderer already isolates elevation behind one function consumed by both
  terrain mesh and billboards (`perspective-renderer.js:68-70`,
  `terrain-renderer.js:29,204`, `perspective-camera.js:88`) — satisfying the
  reference §8.8 invariant (one height function for mesh and sprites).
- Authoring a real heightmap for the 200×200 surface is a content project, not
  a renderer one; at the D-107 ARPG pitch (~62°) flat ground with billboarded
  props reads correctly (most ARPGs do exactly this). Reference §8.5 warns that
  quantized/derived elevation bakes false relief — so tile-metadata-derived
  height is rejected as an interim: it would invent geometry the maps never
  authored.
- When elevation content is wanted (later wave, owner decision), the path is an
  AUTHORED source — a Tiled tile-property or dedicated layer exported into
  `server/maps/layers/` and shipped in the scene payload — feeding the same
  `heightAt` sampler. No projection, billboard, or picking code changes when
  that lands.

## 6. Camera integration plan (D-107)

The browser camera's knob space is `horizon`/`focus`/`zoom`/`dofStrength`
(`perspective-camera.js:47-60`), not the slice's pitch/perspective scalars;
translate D-107 as follows:

- **Primary = ARPG**: steeper, subtler perspective than today's `-0.45·H`
  horizon. Reference §9 maps drama to `HOR` (−0.3·H strong … −1.2·H subtle);
  pitch 62 / perspective 0.0006 lands around `HOR ≈ −0.55·H`…`−0.7·H`, to be
  pinned against the demo side-by-side. `FOCUS` moves from `0.65·H` to ≈
  `0.52·H` (slice anchor 0.52). Wheel zoom recenters around the ARPG base
  (slice zoom 0.85 feel) inside the existing 0.72–1.6 clamp
  (`perspective-renderer.js:1245-1247`).
- **Miniature = zoom blend, not a mode**: `dofStrength` maps to zoomProgress
  with zero at/below the ARPG base zoom and rising only on zoom-in, replacing
  today's `interpolate(0.32, 0.82, …)` floor (`perspective-camera.js:56-60`);
  horizon/perspective strength may ease toward miniature values in the same
  blend. High Table is not implemented (D-107).
- Camera presets become named constants in one module
  (`src/core/rendering/perspective-camera.js` or a sibling
  `camera-presets.js`) so the native client can mirror them and the A/B
  evidence pack can be reproduced.
- Mouse picking (`unproject`, `perspective-camera.js:91-104`) and the WASD
  movement model are unaffected by constant changes; the e2e picking assertions
  guard regressions.

## 7. Phase-by-phase file-touch forecast

Each phase is a conformance pass over existing code. Browser-game rules apply:
`npm run playtest` green plus `npm run test:e2e` (or at minimum
`smoke:browser`) and a real-browser look per phase; the legacy toggle stays
until phase 5 sign-off.

### Phase 1 — Camera + focus conformance (kills playfield blur/mud)

- `src/core/rendering/perspective-camera.js:47-60, 126-133` — ARPG constants,
  zero-floor DoF, preset module.
- `src/core/rendering/perspective-renderer.js:488-495, 496-498` — sprite blur
  now inert at default zoom; remove shadowBlur.
- `src/components/GameCanvas.vue:634-639` — remove/scope the CSS filter (its
  comment ties it to the shader-side wash fixed in phase 2; keep the two
  changes coordinated).
- `src/core/rendering/renderer-mode.js` — unchanged behavior; add optional
  `?camera=` debug override for evidence captures.
- Accept: D-107 defaults live; near/far scale ratio and picking verified
  numerically (existing `projectWithShaderMath` test seam,
  `perspective-camera.js:106-124`); game reads crisp at default zoom.

### Phase 2 — Terrain + horizon conformance

- `src/core/rendering/terrain-renderer.js:124-135` — neutralize gamma/desat in
  the fetch; replace haze curve with the saturating reference curve.
- `src/core/map.js:912-961` — ground bake unchanged (GID/atlas semantics
  already shared with legacy); revisit bake density only if shimmer appears.
- `src/core/rendering/perspective-renderer.js:179-207` — sky/treeline tuned to
  the new horizon; `:157-177` legacy-ground fallback realigned.
- Accept: permanent horizon, zero playfield haze, no shimmer at distance,
  sprites glued to terrain while walking (numerical parity check).

### Phase 3 — Lighting + atmosphere retune

- `src/core/rendering/lighting-renderer.js:4-15` (day-cycle keyframes; 300 s
  today vs reference 90 s — pick per owner taste), `:74-91` vignette, `:93-121`
  clouds (screen-space only, §8.2), `:153-175` multiply composite.
- `src/core/rendering/atmosphere-renderer.js:32-56, 121-130` — mist/ray
  densities to the D-107 fog level.
- Hook existing light emitters only (projectiles/spells); no new gameplay.
- Accept: dawn/noon/dusk/night graded correctly beside the demo; lights read at
  night.

### Phase 4 — DoF coupling + polish

- `src/core/rendering/perspective-camera.js` dofStrength↔zoom blend (miniature
  on close zoom), continuous radii only (§8.3).
- `src/core/rendering/perspective-renderer.js:1245-1279` — wheel/pinch range
  aligned to the blend.
- Terrain occlusion (`occlY`-style clip) stays DORMANT while h=0; the
  `heightAt` seam keeps the hook.
- Accept: close-zoom miniature blend with no banding/stepping anywhere.

### Phase 5 — Performance + hardening

- One terrain draw call, quarter-res lightmap, cached gradients, minimized
  filter-state churn (audit `perspective-renderer.js` per-sprite `ctx.filter`
  and shadow usage removed in phase 1).
- Zero-viewport guard and zoom floor already present
  (`perspective-camera.js:42-45`); verify WebGL context-loss handling and
  resource disposal on scene transitions (`client.js:167-224`).
- Full gates: `npm run lint`, `test:unit`, `playtest`, `test:e2e`; profile on
  the weakest target; legacy-renderer retirement is an owner sign-off decision
  at this point and not before (ground rule 3).

## 8. Feature-flag / toggle design

The required toggle already exists and is the canonical pattern:
`src/core/rendering/renderer-mode.js` (perspective default, `?renderer=legacy`
override, localStorage `verdigris:renderer`, F6 hotkey at
`GameCanvas.vue:524-531`, dispatch at `engine.js:114-119`). Phases mutate the
perspective pipeline only; legacy stays byte-identical as the fallback. The
only addition is a non-persisted `?camera=` debug override (phase 1) so the
owner can A/B camera constants without a rebuild — same query-param pattern,
no new flag system. Toggling never changes game/server state.

## 9. Risks

- **Constant-tuning churn**: phases 1–4 are judgement calls against a reference;
  mitigate with the `?camera=` override and side-by-side demo captures per
  phase (the 0012 evidence-pack method).
- **Two camera models**: the slice presets (pitch/perspective) and the browser
  constants (HOR/FOCUS) are different parameterizations of the same feel; the
  mapping in §6 must be validated visually, not assumed.
- **Viewport-coupled zoom**: `zoom` derives from window size
  (`perspective-camera.js:49-52`), so "default zoom" varies per device; DoF
  zero-floor must be defined against the ARPG base, not a fixed userZoom.
- **CSS filter removal** (phase 1) also changes the legacy renderer's look if
  scoped globally — scope deliberately.
- **Bake memory**: the 6400×6400 world bakes at 16 px/tile
  (`terrain-renderer.js:4`); raising density for crispness must respect POT +
  mipmap legality (§8.7).
- **Harness blindness**: `playtest` proves protocol, not pixels (root
  HANDOFF.md lesson); every phase needs the browser gate and human/automated
  captures.

## 10. Verification summary for this survey

Findings above were collected by direct reads of every cited file in the
`codex/native-reconstitution` checkout at `2af6b2d` (plus the TASK-0012
evidence report and the vendored reference docs). No code was changed; the
working tree contains only this document.
