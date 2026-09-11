# Handoff: Verdigris 2.5D Graphics Overhaul

You are working inside the Verdigris repository — an existing 2D RPG. Your task is
to overhaul its rendering to the 2.5D style demonstrated in the attached package
(`verdigris-25d-overhaul.zip`): a tilted-perspective camera over a GPU-rendered
heightmap terrain mesh, with pixel-art billboards, screen-space dynamic lighting,
day/night grading, depth of field, and atmospheric effects. Gameplay, entity logic,
maps, and data formats must keep working; this is a renderer replacement, not a
game rewrite.

## The package

- `dist/songs-of-the-mire.html` — playable single-file demo. **Open it first and
  play for two minutes.** This is the acceptance target for look and feel.
- `src/game_template.html` — the reference implementation (assets stripped to a
  placeholder). All rendering code lives in one `<script>`; it is written to be
  read and ported, not imported.
- `docs/ARCHITECTURE.md` — **read this in full before writing code.** It defines
  the coordinate model, the exact camera math, the WebGL terrain shader, the
  render-pass order, and — critically — §8, a list of real bugs already hit and
  solved in this design. Do not re-derive solutions to those; use the documented
  ones.
- `assets/*.png` — the demo's processed sprites/tiles, useful only for testing;
  Verdigris keeps its own art.
- `tools/build.py` — rebuilds `dist/` from `src/` + `assets/` (python3, stdlib).

## Ground rules

1. **Survey before you touch anything.** Identify Verdigris's engine/stack,
   current render loop, coordinate conventions, entity/draw architecture, camera,
   and map format. Write your findings and a concept mapping (world units, depth
   axis, sprite anchor points, existing z-ordering) into
   `docs/25d-overhaul-plan.md` in the repo before phase 1.
2. **The math is engine-agnostic; the code is not.** If Verdigris is web/canvas,
   you can port functions nearly verbatim. If it is another engine (Godot, Unity,
   LibGDX, custom), port the *equations and pass order* into that engine's idioms
   (e.g. the terrain mesh + shader translate directly to any engine's mesh +
   vertex/fragment shader; the lightmap multiply becomes a fullscreen post pass).
   The invariant that must survive any port: **one projection formula and one
   height function shared by terrain and billboards** (ARCHITECTURE §2, §8.8).
3. **Feature-flag everything.** Add a renderer toggle (config or debug key) that
   switches between the legacy renderer and the new one at runtime for as long as
   the overhaul is in progress. Never delete the legacy path until the final
   phase is accepted.
4. **Do not port demo gameplay.** Combat, AI, pickups, the specific map, and the
   HUD in the demo are throwaway scaffolding.
5. Commit per phase with the phase name; keep each phase shippable.

## Phases (each has its own acceptance check)

**Phase 0 — Survey & plan.** As above. Also decide where elevation data comes
from for Verdigris maps: an authored heightmap layer, values derived from
existing tile metadata, or (interim) a flat `h=0` everywhere. Flat-zero is a
legitimate first integration — every later phase works with it.

**Phase 1 — Perspective camera + billboards.** Implement the projection
(ARCHITECTURE §2) and route all world-space drawing through it: position by
`projT`, scale by `s`, y-sort by depth. Ground can remain the legacy flat render
temporarily, or the strip fallback. Accept when: walking around Verdigris shows
sprites growing toward the bottom of the screen (~3× scale ratio), mouse/touch
picking still lands correctly (`unproj`), and the legacy toggle still works.

**Phase 2 — Terrain mesh.** Pre-render the current map to a ground texture
(reuse Verdigris's existing tile renderer for this — do not re-implement tile
drawing), add the hillshade bake if elevation exists, build the grid mesh, port
the vertex/fragment shaders (§3), composite under the sprite layer. Include the
horizon: saturating fog + sky + treeline (§3, §6). Accept when: terrain shows
depth, a permanent horizon, no shimmer/pixel-crawl at distance (mipmaps +
anisotropy present), and sprites sit on the surface with zero visible drift while
walking (verify numerically like the demo does if in doubt).

**Phase 3 — Lighting & atmosphere.** Ambient day/night multiply grade, additive
projected light sources (hook up whatever light-emitting entities Verdigris has:
torches, spells, windows), screen-space cloud shadows (heed §8.2), vignette.
Accept when: a full day cycle looks good at dawn/noon/dusk/night and lights read
correctly against the grade.

**Phase 4 — DoF & polish.** Continuous DoF on terrain (in-shader) and sprites
(continuous blur radius — never discrete levels, §8.3), zoom control coupling to
DoF strength, terrain occlusion clipping for sprites behind ridges (skip if
Verdigris maps stay flat), god rays / mist / ambient particles where they fit the
game's tone. Accept when: zooming in visibly shallows focus with no banding or
stepping anywhere.

**Phase 5 — Performance & cleanup.** Profile on the weakest target Verdigris
supports; budget: terrain 1 draw call, lightmap at ≤ quarter res, sprite filter
changes minimized. Handle context loss and the zero-viewport startup case
(§8.1). Only then, with sign-off, remove or archive the legacy renderer.

## When something looks wrong

Check ARCHITECTURE §8 first — full-screen smearing gradients, banding, floating
sprites, NaN storms, and terrain shimmer are all documented failures with known
causes. Reproduce the demo side-by-side with the same camera constants before
concluding the design is at fault.
