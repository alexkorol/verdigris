# Pixel world art: current integration

The production Windows client now draws the imported player, enemy, NPC,
scenery, terrain and loot PNGs from `runtime/`. `raster_art.hpp` caches decoded
and nearest-neighbor scaled images with bounded memory. Static props use
visible content height; animation uses the shared full-canvas pivot and scale.

`source/` retains generated candidates, including rejected ones. `prompts/`
records the actual image requests, reference paths and observed failures.
`runtime/catalog.json` resolves import order and records the active source,
conversion settings, hashes and limitations for each of 73 PNGs. The importer
uses the owner's actual Pixel Respecter project at
`Z:/Code/Python/pixel-perfecter`; it is not a substitute pixelation filter.

Read [RESEARCH.md](RESEARCH.md) for firsthand Image 2.5 prompts, demonstrations
and critical feedback, and [PROMPTING.md](PROMPTING.md) for the resulting local
workflow. Model variant names are recorded only where the source actually
identifies them; our image tool has no model selector.

## Verified milestone, September 9, 2026

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

The four-frame SE walk remains a candidate pending actual game motion review.
Other walk directions, attack/recovery cycles and enemy motion need work.
Generated color variation, provisional directional identity differences,
staff/weapon handedness and held-item attachment need further inspection.
The repeated ground, extraction marker, gate light shapes and overall scene
composition still need improvement. Static props and actors have differing
apparent pixel pitch at the current world scale. Passing checks do not certify
these visual gaps as complete.

Local evidence is under `.ci-artifacts/` in the worktree:
`raster-refined-live.png`, `raster-refined-scenarios.log`,
`raster-world-scenario.log`, `raster-browser-playtest.log` and
`raster-wave-build.log`. Native walk strips and the 140 ms/frame preview GIF
are in `native/tools/raster/`. These are review aids, not gameplay acceptance.
