# Pixel world follow-up review

Reviewed September 9, 2026 in the isolated Diablo study worktree.

- `live-3440x1440.png`: supported PrintWindow capture after launching
  `native/tools/play-native.ps1 -Local`; inspected at original resolution.
- `scene-1366x768.png`: production renderer capture of the same initial route.
- `raster-motion.mp4`: 40 production captures, 60 ms apart, encoded at 50/3 fps.
  The scenario consumes held diagonal movement through the real 50 ms game
  tick, advances presentation at 15 ms, then releases input for 600 ms.
- `raster-motion.csv`: actual positions, smoothing value, gait phase and drawn
  pose. Travel covers all four SE walking frames; release settles to `hero_se`.

The live scene now has quieter brown earth, warm stone stairs, a small brazier,
consistent clothed hero idles and closer prop/actor pixel scale. The initial
hero stands over the exit landmark; the scene still needs intentional paths,
ground transitions and less accidental scenery overlap. Current small props
and enemy bodies are not all equally resolved. HUD clipping and flat orb
liquids remain visible. The walking sources retain body/palette variation;
this movement trace proves integration and stopping, not polished animation.

Color review uncovered three cooperating errors: the orb mask packed red and
blue into opposite bytes, its test sampled the same reversed order, and the
PNG exporter swapped the whole image. All now follow BGRA DIB storage. The
new swatch round trip uses a known GDI RGB color and GDI+ PNG decoding as an
independent check. Older reference PNGs are unsuitable for palette judgment;
the supported live capture was the reliable comparison.

The dense-effect test initially measured 50.6 ms, with its 128-effect cap
correct. Its bitmap was inadvertently monochrome because it was created
compatible with a fresh memory DC. The corrected display-compatible color
surface retains the production floor cache and measures 8.6 ms. The separate
20-frame 3440x1440 gate measures 18.7 ms; neither 40 ms limit was changed.

Final verification:

- `native/build.ps1 -RunTests`: native core, networking, camera, session,
  presentation and audio suites passed before the final capture/tint fixes.
- `native/build.ps1 -RunClientScenarios -CaptureRoot .ci-artifacts/raster-accepted`:
  final build and all 64 scenarios pass, including color, movement and timing.
- `npm run playtest`: 32/32 pass; no browser code changed.
- Actual Pixel Respecter importer: six checks pass; catalog verifies 75 RGBA,
  binary-alpha assets with at most 32 opaque colors and matching hashes.
- `native/tools/raster/test_equipment.ps1`: grip/occlusion/cache tests and
  fractional sampling tests pass. Compiled source is retained beside the runner.

This is a verified integration milestone within the ongoing visual goal.
Full directional motion, weapon-specific attacks, enemy animation and scene
composition remain open. No push or merge.
