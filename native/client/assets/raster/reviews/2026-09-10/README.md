# Directional pixel motion and contact review

Reviewed September 10, 2026 in the isolated Diablo study worktree. This is an
integration checkpoint within the ongoing visual goal, not final art approval.

`live-3440x1440.png` is the supported PrintWindow capture of the final build
launched with `native/tools/play-native.ps1 -Local`. Root viewed the live scene
and the preceding identical presentation at original resolution. Both sessions
closed normally with no orphan client. The gate opening is clear and the orbs
retain textured red/blue liquid, glass shading and the surrounding stone art.

`walk-{se,sw,nw,ne}.csv` records 160 actual presentation steps at 15ms, driven
by real movement input through 50ms simulation ticks. The actor travels 180
units on each requested axis, then releases input for 600 ms and settles to the
matching idle with moving=0.0017214. Every authored gait pose is actually
painted. `captured_frame` links the 40 saved 60 ms screenshots to the full trace.
The matching MP4s encode those 40 screenshots at 50/3 fps without interpolation.
They use H.264/yuv420p for playback; PNGs remain authoritative for color review.
Root viewed production walking frames and source strips; this is not a claim
to have watched every continuous clip through a video-analysis tool.

`strike-*.png` captures real simulation contact, preparation/recovery sampling,
and axe acquisition through kill/drop/pickup/equip before SE/SW/NW confirmed
hits. The target's actual life decreases in each directional test. Root viewed
the contact captures. Damage text now clears the target bar. The near-contact
fixture overlaps bodies; simultaneous real hits legitimately tint both actors.
The equipped SW capture also demonstrates foreground column occlusion.

`equipped-strike-*.gif` is a separate, slower equipment inspection, with 90 ms
action slots and 360 ms idle holds. It uses the production attachment renderer
but does not establish simulation cadence. `equipment-review.json` preserves
the completed 54-pose probe's terminal provenance, reported results and limits.

`orb-comparison.png` shows old/new 0/25/50/100 states using existing raster
plates. `orb-probe.log` records monotonic liquid, preserved neutral detail,
repeated-draw stability and bounded cache usage. No new orb images were needed.

## Verification and corrected failures

- `native/build.ps1 -RunTests`: exit 0; core, networking, camera, session,
  presentation and audio suites passed. The later code change only corrects
  the motion scenario's route and sampling. Full log: `native-suites.log`.
- `native/build.ps1 -RunClientScenarios -CaptureRoot .ci-artifacts/raster-directional-verified`:
  final build exit 0, all 64 scenarios pass, fullscreen 18.7 ms / 20 frames at 3440x1440,
  dense 128 effects 9.4 ms. Both 40 ms bounds unchanged. `native-final-64-of-64.log`.
- First native run 63/64 is retained. Its SW review walked into a solid shrine
  and counted only 60 ms screenshots, missing phase 6. The fixture now verifies
  an unobstructed corridor against unchanged scenery and paints/counts every
  15ms step. The movement-distance assertion is stronger; no collision was
  removed and no art frame was dropped to make the test pass.
- Browser initial 31/32 failed only at the session-arc final-death wait. The
  focused scenario passed 1/1, and the final full rerun passed 32/32, exit 0.
  All three logs are retained. Failure-only diagnostics preserve the first
  and last 16 existing state observations and recent hit/message data. Server
  behavior, assertions and deadlines are unchanged. The initial timeout is
  still unexplained and intermittent; the rerun does not prove it fixed.
- Actual Pixel Respecter importer 7/7; catalog 118 assets; equipment 54 poses x
  five weapons x three scales; all eight extra sampling sizes pass. Color
  checks verify GDI DIB/display surfaces and PNG channels rather than trusting
  the old byte-swap workaround.

## Remaining visible gaps

NW walking feet sit above the ground ellipse in some frames. SW strikes have
14–15px heads versus 12 px idle heads, and NW strike bodies are 57–59 px versus 63 px
idle height. Broader bodies, palette/outline jumps, motion onset smoothing and
registration need another art/motion iteration. These are recorded gaps, not
hidden by fitting every pose to a separate bounding box.

NE's new strike is incomplete and remains outside runtime. The raider walk
candidate loses its idle axe and is rejected for integration. Other enemy
actions, bow/staff-specific attacks, quieter terrain, intentional paths and
coherent HUD type/chrome remain active work. Ground microtexture and HUD
clipping are visible in the retained live scene.
