# Terrain and wall production trial, 2026-09-10

Accepted for provisional integration and the owner's requested ship/pause.
This milestone does not complete the broader pixel-world or game-feel work.

The client uses quieter earth in dungeons and a new 64x64 stone floor in
crypts. Raster wall modules now follow the authoritative blocked-cell grid,
share the actor depth pass, and fade to alpha 80 where a later wall overlaps
the player's visible sprite. The 64x96 module covers a 64x64 footprint with
32 pixels of lift and pivot [32,96]. Collision maps and simulation are unchanged.
Interior ground layout adds no village roads or procedural tree planting;
existing authoritative scenery remains, including trees visible in the live dungeon.

## Source and processing

Both new assets came from built-in image generation and the actual Pixel
Respecter project. The stone source initially failed visual tiling. A second
imagegen offset edit also failed. The retained width-2 repair runs the actual
unmodified `edge_match_blend` from Ian Lintner's MIT-licensed repository at
commit `19dbb4d5b655f8c4c0c8f979dd89baeb10237b0a`, using the exact 23-color source
palette. Code, license, pinned hashes and a standalone derivation command live
under `native/tools/raster/quiet-stone-derived/`. Width 8 was rejected for bands.
This is reuse of published processing code, not evidence of an Image 2.5
seamless-tile prompt. Native actor/repeat comparisons are in `stone-seams/`.

The wall model output painted its background and stretched the requested cap.
Actual Pixel Respecter reconstructs measured cap and front-face crops; explicit
64x64 and 64x32 normalization then assembles them at row 64. The manifest,
source crops, exact prompts/reference roles, assembly code, measurements and
review images are retained under `native/tools/raster/wall-stone-candidates/`.
The derived opaque module is intentional; source geometry/alpha failure is
recorded rather than described as prompt success.

Catalog: 176 active PNGs, 33 manifests, 62 active unique sources, 185 import
records including overrides. All 174 prior runtime PNGs and 60 prior sources
are unchanged. The two production PNGs are byte-identical to selected candidates.

## Verification and observed limits

The initial `native/build.ps1 -RunTests -RunClientScenarios` completed the native
core/headless suites but crashed in client scenarios (access violation,
-1073741819). Isolating `raster-world` passed; `raster-walls` reproduced it.
The wall fixture has no loot backend, so `nearest_pickup_id` now returns empty
when both session and simulation are absent. An explicit assertion covers that
condition. No fake session or rendering workaround was introduced.

After this presentation-only fix, the supported build and all **73** client
scenarios passed. The native core suites passed before the guard; they were not
rerun afterward. Browser playtest: **32/32**. Actual importer: **10/10**.
Ground-raster cache probe: **PASS**. Final metadata-only reimport preserved all
176 PNG bytes. Equipment checks were not rerun in this wave.

At 3440x1440, frame-budget average was **22.429 ms** static and **24.785 ms**
moving, with **34.795 ms** moving peak. The unchanged gate is 40 ms average.
The wall scenario checks pixel sampling, alpha, caches, missing/malformed
assets, fractional placement, four-neighbor exposure, corners/interiors,
elevated viewport culling, authoritative blocked movement, quiet materials,
actor depth and final warning order. Its 12 images are production paint
fixtures with WorldSimulation maps, not manual remote play.

Root inspected dungeon north and crypt south at 1366x768, the corner,
border/interior, elevated-culling probe, and crypt north at 3440x1440.
Player visibility and warning readability hold in those views. Root also
launched the supported normal remote game (port 6520), selected the existing
Scion, entered town, entered the dungeon, toggled F3 and inspected the actual
3440x1440 captures. Live F3 showed PNG assets loaded, READY, 20 monsters and
18.9 ms paint at the observed moment; it is not a sustained benchmark.
Windows.Graphics.Capture failed with `SetIsBorderRequired` / `0x80004002`;
the repository's supported capture-window script supplied the viewed images.
Escape closed the client with exit 0; the launcher verified no owned orphan
processes and a fresh window inventory found no game windows. No manual fight,
new character creation or completed expedition is claimed for this launch.

Stone slab repetition and interrupted joints remain visible. Wall cap motifs
and masonry joins repeat, with a broad module-wide cutaway that can affect
adjacent rows abruptly. Cutaway bounds exclude separately held equipment.
The live F3 debug text overlaps existing HUD text. These are retained limits,
not a final seamless-terrain, polished-wall or finished-game-feel claim.

The owner requested pause and ship. No further image generation or feature
work belongs to this wrap-up. Rejected candidates remain preserved locally.
Exact evidence copies and source hashes are in `retained-evidence.json`.
