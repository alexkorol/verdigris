# Songs of the Mire — 2.5D Rendering Architecture

Reference implementation: `src/game_template.html` (single file, ~1100 lines, zero
dependencies). Everything below cites the actual identifiers in that file so you can
grep straight to the code. The demo's gameplay (combat, AI, pickups) is throwaway;
the rendering stack is the deliverable.

## 1. The coordinate model

World space is a flat 2D plane, exactly like any top-down 2D RPG:

- `(wx, wy)` — world position in "world units" (the demo's world is 2600 × 1900).
- `wy` doubles as the **depth axis**: larger `wy` = closer to the viewer.
- `h = terrainH(wx, wy)` — elevation in the same units, from an analytic heightfield
  (sum of gaussian hills; swap in your own heightmap sampler freely).

**Nothing about gameplay changes.** Entities move, collide, and path on the flat 2D
plane. Elevation is read-only for rendering (plus optional slope forces on movement).
This is the property that makes the overhaul non-invasive for an existing 2D RPG.

## 2. The camera (grep: `updateProjection`, `proj`, `projT`, `unproj`)

A tilted pinhole camera is expressed with four scalars, recomputed per frame:

```
HOR   = -0.45 * H        // screen y of the vanishing horizon (above the frame)
FOCUS =  0.65 * H        // screen row the player is pinned to
zoom  = max(W/1150, H/1500) * userZoom   // px per world unit at the focus plane
DZP   = (FOCUS - HOR) / zoom             // camera depth-distance to the focus plane
A     = (FOCUS - HOR) * DZP
D0    = cam.y + DZP                      // world-y of the virtual camera foot
```

Projection of a world point at elevation `h`:

```
dz = D0 - wy                             // depth from camera (clamped >= 40)
s  = zoom * DZP / dz                     // scale (px per world unit at this depth)
screenX = W/2 + (wx - cam.x) * s
screenY = HOR + A/dz - h * s             // projT(); proj() is the h=0 case
```

`unproj(sx, sy)` inverts this at h=0 for mouse picking. Perspective strength is the
single knob `HOR` (closer to 0 = lower, more dramatic camera). `FOCUS` is where the
player sits; `zoom` handles portrait vs landscape.

Everything in the world — sprites, shadows, particles, light positions, damage
numbers, VFX — goes through `projT` and multiplies its size by `s`. That single rule
is 90% of the 2.5D effect.

## 3. Terrain: GPU heightmap mesh (grep: `initGL`, `renderGLTerrain`)

The ground is a static grid mesh (176 × 140 quads spanning world+margin) with
per-vertex elevation, drawn by one WebGL draw call under the 2D canvas.

The vertex shader reproduces the camera equations *exactly* in clip space
(w = dz gives perspective-correct interpolation for free):

```
xc = (2/W) * (wx - cam.x) * K            //  K = zoom*DZP
yc = dz - (2/H) * (HOR*dz + A - h*K)
zc = dz * (2*(dz-near)/(far-near) - 1)   // monotonic depth for the z-buffer
gl_Position = vec4(xc, yc, zc, dz)
```

Verified in the demo to match the JS `projT` to ~1e-13 px — sprites sit on the
surface with zero drift. If you change the JS camera, change the shader identically.

The ground texture is the entire pre-rendered map (see §4) uploaded once to a
power-of-two texture with `generateMipmap` + `LINEAR_MIPMAP_LINEAR` + anisotropic
filtering (ext, clamp 8x). **Mipmaps are not optional** — they are what prevents the
pixel-crawl/shimmer that killed the CPU voxel attempt (kept in the file as a
no-WebGL fallback; grep `voxel`).

Fragment shader = texture sample + depth-of-field + fog:

```
coc  = clamp((|dz-DZP|/DZP * 1.6 - 0.12) / 0.40, 0, 1)
color = mix(texture(uv), texture(uv, bias=3.5), coc)   // mip-bias blur ≈ free DoF
haze  = clamp((dz/DZP - 1.12)/1.02, 0, 1) * 0.96
out   = mix(color, skyColor, haze)
```

The haze **must saturate on-screen** (it reaches 1.0 ~5–6% from the top) — that is
what produces a permanent horizon; without it flat-ish maps read as 2D.

GL canvas is created with `preserveDrawingBuffer:true` and composited into the 2D
canvas each frame via `ctx.drawImage(glCanvas)`, so the whole post stack (lighting,
vignette) still operates on one canvas. Sky gradient + blurred treeline silhouette
are drawn on the 2D canvas *before* the GL image (grep `skyline`); the GL clear is
transparent so sky shows through above the terrain silhouette.

## 4. The ground texture bake (grep: `buildGround`, `buildBlurs`)

One-time at load, on an offscreen canvas covering world + a 420-unit margin (`GM`):

1. Tile the base terrain patterns (grass everywhere, then irregular swamp blob,
   stone plaza, dirt road stamped along a polyline path).
2. **Hillshade bake** — 8px cells, slope lighting from the analytic gradient
   (light dir ≈ NW), warm highlight / cool shadow overlays. Cheap, and because the
   mesh is genuinely displaced, this shading reads as real relief.
3. Margin painted darker so out-of-world sampling looks like dark forest edge.

A half-res blurred copy (`groundBlur`) feeds the CPU fallback's DoF only; the GL
path gets blur from mip bias.

## 5. Billboards (grep: `drawSpriteW`, `shadowW`, `occlY`)

Sprites are plain 2D `drawImage` calls, y-sorted by `wy` (depth), scaled by `s`,
elevated by `projT`, with:

- **Continuous DoF**: per-sprite `ctx.filter = blur(cocT(dz)*3.6*DPR px)` (quantize
  to 0.25px steps for filter-cache friendliness). Never use 2–3 discrete blur
  levels — the step between levels is visibly jarring (learned the hard way).
- **Terrain occlusion**: `occlY(wx,wy)` re-marches one column of the heightfield
  (LUT-sampled, ~60 steps) from camera to the sprite's depth and returns the lowest
  screen-y of nearer terrain; the sprite is clipped to above that line, so
  characters disappear behind ridgelines correctly.
- Shadows are flat ellipses at the terrain surface, scaled by `s`.
- Squash/flash on hit, skew-sway for trees — all via the same transform stack.

Pixel-crisp sprites: `imageSmoothingEnabled=false` for billboards, `true` for the
terrain composite (GL upscale acts as AA).

## 6. Lighting & atmosphere (grep: `ambient`, `lm`, `L(`, `clouds`, `rays`)

Screen-space, quarter-res lightmap canvas multiplied over the composed frame:

1. Fill with the **ambient color** from a day/night keyframe table (`ambient()`,
   90s cycle). The multiply grade alone sells time-of-day.
2. **Cloud shadows**: multiply blend, drawn in *pure screen space* — a handful of
   huge (⅓–½ screen), slow (minutes per crossing), low-contrast radial gradients.
   ⚠ Do NOT project clouds through the world camera: anything near the camera's
   near-plane clamp explodes in scale and smears gradient garbage across the whole
   frame. This was a real shipped bug; clouds are sky-space, keep them there.
3. **Light sources**: additive radial gradients at `projT`-projected positions
   (hearth, boss staff glow, hero aura, projectiles). Intensity scales with the
   night factor so lights matter after dusk.

On top of the lit frame: swamp mist (projected soft blobs), fireflies at night
(additive, projected), pre-rendered **god rays** (built once in `buildLight`: 3
skewed beams with a baked vertical fade dying by 60% screen height; per frame just
alpha-breathe + drift a few px — never regenerate or translate rays per frame, it
reads as glitching), vignette, low-HP pulse, zone title cards.

## 7. Per-frame order (grep: `function render`)

```
updateProjection → camera lerp-follow
sky gradient + treeline silhouette          (2D)
terrain                                     (GL → drawImage into 2D)
corpses, pickups, telegraphs                (projT)
y-sorted billboards (props/enemies/player)  (projT + occlY clip)
projectiles, particles, damage numbers      (projT)
swamp mist                                  (projT)
lightmap multiply (ambient × clouds + lights)
fireflies (additive) → water glints → god rays
vignette → low-HP pulse → zone card → HUD
```

## 8. Hard-won pitfalls (all were real bugs in this demo)

1. **Zero-size viewport**: embedded iframes/webviews can report 0×0 at first frame;
   `zoom=0 → DZP=NaN` poisoned every gradient call. Heal: compare canvas size to
   the live viewport every frame, skip rendering under 10px, floor `zoom`.
2. **Cloud projection blowup** — see §6.
3. **Discrete DoF levels** on either ground or sprites produce visible bands/steps.
   Ground: continuous mix in-shader. Sprites: continuous blur radius.
4. **Strip abutment** (CPU fallback only): consecutive ground strips must sample
   boundary-exact source rects (`wy(sy1) - wy(sy0)`), never center±derivative.
5. **Quantized "terrace" terrain** from a smooth heightfield bakes contour rings
   that read as arbitrary walls. If you want smooth topo, render smooth topo (the
   mesh); terraces only work if the map is *authored* as terraces.
6. **mediump overflow**: keep large intermediates (`A` ≈ 1e6) in the vertex shader
   (highp by default); only `dz`-scale values reach the fragment shader.
7. **NPOT + mipmaps** is illegal in WebGL1 — resample the map into a POT texture.
8. One height function, used by *both* the mesh attributes and sprite `projT`. Two
   implementations (e.g. LUT vs analytic) drift and sprites float. (The occlusion
   march may use the LUT — a few px of clip error is invisible.)

## 9. Tuning cheat-sheet

| Feel                    | Knob                                    |
|-------------------------|-----------------------------------------|
| Camera drama            | `HOR` (−0.3·H strong … −1.2·H subtle)   |
| Player screen position  | `FOCUS`                                 |
| Visible world width     | `zoom` base divisors (1150 / 1500)      |
| DoF aggressiveness      | `cocT` constants + zoom-strength lerp   |
| Fog distance            | `hazeT` constants (keep it saturating!) |
| Hill drama              | heightfield amplitudes                  |
| Day length              | `dayT % 90`                             |
