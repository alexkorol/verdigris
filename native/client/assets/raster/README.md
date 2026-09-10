# Pixel world art: current integration

The production Windows client now draws the imported player, enemy, NPC,
scenery, terrain and loot PNGs from `runtime/`. `raster_art.hpp` caches decoded
and nearest-neighbor scaled images with bounded memory. Static props use
visible content height; animation uses the shared full-canvas pivot and scale.

`source/` retains generated candidates, including rejected ones. `prompts/`
records the actual image requests, reference paths and observed failures.
`runtime/catalog.json` resolves import order and records the active source,
conversion settings, hashes and limitations for each of 139 PNGs. The importer
uses the owner's actual Pixel Respecter project at
`Z:/Code/Python/pixel-perfecter`; it is not a substitute pixelation filter.

Read [RESEARCH.md](RESEARCH.md) for firsthand Image 2.5 prompts, demonstrations
and critical feedback, and [PROMPTING.md](PROMPTING.md) for the resulting local
workflow. Model variant names are recorded only where the source actually
identifies them; our image tool has no model selector.

## Current milestone: reference-led attacks and readable warnings, September 10, 2026

Twenty-four manifests resolve 148 import records to 139 active PNGs from 49 sources.
All four hero strike directions have six poses; raider SW also has six. NE uses
the accepted idle palette and measured equipment grips. Existing drop art now
matches twelve item families, and compatible storehut variation reuses current
landmarks. Warning boundaries preserve visible actors; real elite attacks paint
contact on the confirmed damage event and then recover.

Supported build/all 68 scenarios, native suites, browser 32/32, importer 10/10 and
equipment 60×5×3 plus sampling pass. Fullscreen static/moving averages are
22.550/23.993 ms, moving peak 33.926 ms; unchanged 40 ms average gates. Root viewed
actual contact, warning, item and scenery captures, native equipment grids and
the supported live window. [Evidence and remaining limits](reviews/2026-09-10-actions/README.md).

NE follow-through width/reset and clip identity remain imperfect. Bow/staff
attachment is not weapon-specific animation. Raider art covers SW; elite event
proof does not claim ordinary native melee has attacker-owned animation.
Native pursuit, other monster motion, pixel effects/deaths and terrain repeat
remain unfinished. Exact active inputs and historical-source limits are recorded
in the two strike dependency manifests. This is provisional integration.

## Previous milestone: ground, motion registration and HUD, September 10, 2026

- Twenty-two manifests resolve 136 import records to 127 active PNGs from 41
  source files. The added quiet earth and eight SW raider poses use the actual
  Pixel Respecter pipeline. The raider retains its axe across the accepted cycle.
- World-aligned paths and planting use the existing village/town landmarks and
  solid footprints. The 512-tile ground cache is bounded and invalidates on
  source reload. Ground detail is quieter, but the generated earth is not
  certified seamless; repeat checks retain the measured edge mismatch.
- NW walking frames move down by exactly one pixel in the first row and five
  in the second, with matching equipment registration. No body pixels are
  rescaled or repainted. Actual movement starts the walk immediately; the
  existing stop smoothing still settles to idle.
- Route, audio and quickbar chrome uses the shared ornate skin, measured text
  and responsive placement. Equipment vital/stat rows no longer overlap, and
  the XP caption clears character/gear panes at 960x600. Fullscreen and smaller
  production captures, including both panes with audio muted, were reviewed.
- Native suites and all 66 final scenarios pass. Twenty 3440x1440 paints average
  23.258 ms stationary; twenty measured moving paints average 24.715 ms with a
  36.620 ms peak. Both retain the 40 ms average gate. Browser playtest is 32/32,
  importer 7/7, and equipment/ground/HUD probes pass.
- The raider review moves a monster through the production presentation path
  with scripted, collision-checked positions and verifies all eight phases and
  return to idle. Native enemy AI remains stationary; this is not pursuit proof.
- Reference-led NE contact repair improved the striking arm, but the next
  generated phase switched arms and its matte cleanup damaged the forearm.
  Both exact requests and diagnostic outputs are retained outside runtime.
  These are local built-in-tool observations; no model variant was selectable.
- [Viewed live window, phase captures, traces, clips, failures and final logs](reviews/2026-09-10-ground-hud/README.md).

## Previous integration milestone, September 10, 2026

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
  gate measures 18.7 ms over twenty 3440x1440 frames; the dense 128-effect frame
  measures 9.4 ms. Browser final rerun passes 32/32. Seven importer tests and
  the orb, equipment and color checks pass.
- [Retained live captures, actual motion traces, clips and verification](reviews/2026-09-10/README.md).

That milestone verified integration for further iteration. Its elevated NW
feet are corrected above; SW strike heads and cross-clip palette/outline changes
still need refinement.

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

Walking and the three integrated strikes still need more consistent identity,
palette and timing. NW foot registration is corrected, but its body/head bob
differs from idle. NE retains the legacy attack because the replacement action
is incomplete. Raider SW walking is available to moving presentation snapshots;
native enemy pursuit and other monster directions/actions remain unfinished.
Weapon-specific body mechanics, terrain repetition, some scenery overlap and
further HUD composition need refinement. Arbitrarily long owner names still
need a wrapping policy. Passing checks do not close these visual gaps.

Local evidence is under `.ci-artifacts/` in the worktree:
`raster-refined-live.png`, `raster-refined-scenarios.log`,
`raster-world-scenario.log`, `raster-browser-playtest.log` and
`raster-wave-build.log`. Native walk strips and the 140 ms/frame preview GIF
are in `native/tools/raster/`. These are review aids, not gameplay acceptance.
