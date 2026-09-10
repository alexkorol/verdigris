# Pixel world art: current integration

The production Windows client now draws the imported player, enemy, NPC,
scenery, terrain and loot PNGs from `runtime/`. `raster_art.hpp` caches decoded
and nearest-neighbor scaled images with bounded memory. Static props use
visible content height; animation uses the shared full-canvas pivot and scale.

`source/` retains generated candidates, including rejected ones. `prompts/`
records the actual image requests, reference paths and observed failures.
`runtime/catalog.json` resolves import order and records the active source,
conversion settings, hashes and limitations for each of 118 PNGs. The importer
uses the owner's actual Pixel Respecter project at
`Z:/Code/Python/pixel-perfecter`; it is not a substitute pixelation filter.

Read [RESEARCH.md](RESEARCH.md) for firsthand Image 2.5 prompts, demonstrations
and critical feedback, and [PROMPTING.md](PROMPTING.md) for the resulting local
workflow. Model variant names are recorded only where the source actually
identifies them; our image tool has no model selector.

## Verified integration milestone, September 10, 2026

- Hero walking now uses SE4, SW8, NW8 and NE8 frames. SE, SW and NW strikes
  each have six poses, with frame3 displayed at confirmed contact. Distance
  drives walking phase; completed strikes and released movement return to idle.
- The actual Pixel Respecter importer reduces one shared palette across each
  new cycle and preserves common 80x96 canvases. Twenty manifests resolve 127
  import records to 118 active PNGs from 39 source files. Binary alpha, at most
  32 colors, dimensions and source/output hashes are verified.
- Published workflows were applied, with exact prompts and failures retained.
  Kiki's preparation/contact/recovery structure guided strikes. SW's two bad
  hand poses were repaired as individual edits using Noel's fallback and
  Flixly's transition/identity reference roles. Corrected D2 motion references
  stay in the external study cache; none of their original pixels ship here.
- Equipment follows measured hands and changing body/finger occlusion. The
  probe passes 54 poses x five weapons x three scales, with grip error <=0.5px,
  stable GDI/cache usage and unchanged SE hand fingerprints. Bow/staff carry
  orientation remains a limitation during these melee body poses.
- A small pixel spark and a tint through the actual struck sprite replace
  contact disks at the feet. The 48x48 spark's measured contact origin is
  [21.5,27.5], used by `draw_contact_spark`; it fades as a static sprite.
  Damage text ends above its lift rather than extending down across life bars.
- Decorative trees frame the clearing, team rings are smaller, and life bars
  follow visible sprite bounds. `ui_skin.hpp` now composites the existing
  detailed orb art with textured liquid at 21 fill levels, preserving glass,
  stone hands and empty states. Its layer cache is capped at 64 layers/16MiB.
- Native suites and all 64 final scenarios pass. The unchanged fullscreen
  gate measures 18.7ms over twenty 3440x1440 frames; the dense 128-effect frame
  measures 9.4ms. Browser final rerun passes 32/32. Seven importer tests and
  the orb, equipment and color checks pass.
- [Retained live captures, actual motion traces, clips and verification](reviews/2026-09-10/README.md).

This verifies integration for further iteration. It does not certify finished
animation: NW walking has visibly elevated feet in some production frames,
SW strike heads broaden, and palette/outline changes remain across clips.

## Verified milestone, September 9, 2026

- Follow-up: three individual referenced hero directions replace the old
  costume-changing idles; larger props were reconstructed at a finer grid to
  match the hero's display pixel scale. Quiet packed earth and physical exit
  stairs replace the cracked lattice and giant exit plate. A brazier now
  accompanies the restrained gate light. These changes were inspected in the
  actual 3440x1440 window and at smaller production capture sizes.
- Equipment uses measured grips and body/finger occlusion for the sprite
  actually drawn, including idle fallbacks. Walking settles to idle after
  movement stops; the former bool conversion treated the smoothing tail as
  permanent movement. The separate equipment checks pass for 12 poses, five
  weapons, three sizes, and eight additional sampling sizes.
- Current final build: all 64 native scenarios pass, including the new actual
  movement/stop capture. Frame budget is 18.7 ms over twenty 3440x1440 frames;
  the dense 128-effect frame is 8.6 ms. Both retain the 40 ms limit.
- Corrected reversed color bytes in the orb masks, PNG exporter and orb test.
  An independent GDI swatch/export round trip verifies RGB preservation. The
  scenario painter now uses the display's color format; its former memory-DC
  bitmap was monochrome and cost 50.6 ms under the dense-effect fixture.
- Browser playtest passes 32/32. Native core/network/session/presentation/audio
  suites pass. Six importer checks and the equipment probes pass.
- Retained [live capture, motion clip, trace and review](reviews/2026-09-09/README.md).

The preceding initial milestone established these foundations:

- Six actor families have four named static direction assets. Hero/raider
  also have attack poses; a first SE hero walk has four separate referenced
  images. Facing and identity remain subject to the caveats below; this is
  not full animation coverage.
- The active catalog verifies real RGBA, binary alpha, at most 32 colors per
  asset, native dimensions and source/output hashes.
- Six importer checks pass, including preservation of common animation scale
  and limiting enclosed-background cleanup to reviewed rectangles.
- All 62 existing client scenarios pass; frame budget is 16.6 ms over twenty
  3440x1440 frames, below the unchanged 40 ms bound. The added `raster-world`
  scenario separately passes directional asset decoding, four distinct SE
  phase paints through the production helper, and a production scene capture.
- `npm run playtest` passes 32/32. The new native build passes. Full native
  core/network/session suites passed during the initial renderer integration;
  this refinement changes presentation and the XP pixel-sampling bounds.
- Viewed the supported live-window capture at its actual 3440x1440 resolution.
  Houses now have useful scale relative to people, and contact shadows retain
  the ground texture. Native and 4x walk strips were inspected.

## Still unfinished

Walking and the three integrated strikes still need smoother identity, ground
contact and timing. NE retains the legacy attack; its new preparation image
is only a candidate. The raider walk remains outside runtime because it loses
the idle's axe. Enemy motion, weapon-specific actions, motion onset smoothing,
ground transitions and path composition remain unfinished. Uniform ground
microtexture, some scenery overlap and HUD clipping/type/chrome inconsistency
are visible in the live capture. Passing checks do not close these visual gaps.

Local evidence is under `.ci-artifacts/` in the worktree:
`raster-refined-live.png`, `raster-refined-scenarios.log`,
`raster-world-scenario.log`, `raster-browser-playtest.log` and
`raster-wave-build.log`. Native walk strips and the 140 ms/frame preview GIF
are in `native/tools/raster/`. These are review aids, not gameplay acceptance.
