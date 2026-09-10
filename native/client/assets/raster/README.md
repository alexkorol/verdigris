# Pixel world art: current integration

The production Windows client now draws the imported player, enemy, NPC,
scenery, terrain and loot PNGs from `runtime/`. `raster_art.hpp` caches decoded
and nearest-neighbor scaled images with bounded memory. Static props use
visible content height; animation uses the shared full-canvas pivot and scale.

`source/` retains generated candidates, including rejected ones. `prompts/`
records the actual image requests, reference paths and observed failures.
`runtime/catalog.json` resolves import order and records the active source,
conversion settings, hashes and limitations for each of 75 PNGs. The importer
uses the owner's actual Pixel Respecter project at
`Z:/Code/Python/pixel-perfecter`; it is not a substitute pixelation filter.

Read [RESEARCH.md](RESEARCH.md) for firsthand Image 2.5 prompts, demonstrations
and critical feedback, and [PROMPTING.md](PROMPTING.md) for the resulting local
workflow. Model variant names are recorded only where the source actually
identifies them; our image tool has no model selector.

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

The four-frame SE walk now runs through real travel and returns to idle, but
its color/body variation and motion quality still need refinement. Other walk
directions, attack/recovery cycles and enemy motion remain unfinished. The
legacy attack poses change the hero's design and striking arm; measured weapon
attachment does not make them correct weapon-specific animations. Item scale,
equipment changes in play, ground transitions, path composition, scenery
overlaps and HUD presentation also need further work. Passing checks do not
certify these visual gaps as complete.

Local evidence is under `.ci-artifacts/` in the worktree:
`raster-refined-live.png`, `raster-refined-scenarios.log`,
`raster-world-scenario.log`, `raster-browser-playtest.log` and
`raster-wave-build.log`. Native walk strips and the 140 ms/frame preview GIF
are in `native/tools/raster/`. These are review aids, not gameplay acceptance.
