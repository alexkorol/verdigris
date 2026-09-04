# Native WIZARD title milestone — 2026-09-04

## Implemented

- Owner remote startup opens a native, depth-tested 3D floating world instead
  of the ledger. It loads WIZARD's actual tapered GLB (119,259 triangles),
  4K terrain atlas, underside raster and illumination map. Source bytes and
  commit are recorded in `client/assets/wizard/splash/world/manifest.json`;
  the build verifies all four hashes. WIZARD itself was not modified.
- D3D11 presentation adapter, bounded 1600×900 scene resolution, GPU bilinear
  upscale, existing Framekit/GDI+ menu composition. Animated lighting/water,
  subtle orbit, procedural sky; drag orbit, wheel zoom, arrows, +/- and Home.
  Hardware device with WARP fallback; explicit visible error if graphics or
  assets fail, while the admission menu remains available.
- Continue selects the saved living Scion and preserves its mortality flag.
  This deliberately avoids the older `set-out` command, which forces the
  hard lifecycle. Create opens the real naming flow, and House & chronicle
  opens the existing ledger. Fresh accounts are routed to House founding.
- Menu readiness requires an actual Chronicle snapshot plus an established
  connection, not world admission. The first live inspection caught that
  circular readiness condition; disconnected/retrying states stay disabled.
- Name fields accept bounded Unicode clipboard text through the same ASCII
  name policy as physical typing. Embedded paste newlines cannot submit the
  dialog. Long House headings wrap/clip inside their own pane.

## Verification

- `native/build.ps1 -RunTests -RunClientScenarios -BuildSubdirectory title-screen
  -CaptureRoot native/build/title-screen/captures` built successfully and
  passed core/network/camera tests, but its session recovery leg failed once:
  after killing the Warden, the automated heir died while securing the room.
  Original evidence is retained in `native/build/title-screen/acceptance.log`.
- Separate unchanged `verdigris_session_tests.exe` rerun passed the complete
  death/relic/reconnect journey. See `session-recheck.log`. This is a remaining
  intermittent recovery-driver/gameplay investigation, **not a claimed fix**.
- All 34 native client scenarios pass (`client-scenarios.log`), including
  three title sizes, actual mesh count, orbit pixel changes, zoom limits,
  clipboard bounds, admission identity/oath, and missing-asset negative control.
- Original CPU HALFTONE upscaling failed at 48.0 ms. Moving upscale to the GPU
  reduced the 20-frame 3440×1440 title test to 9.0 ms; gameplay averaged 10.3 ms.
  The 40 ms bound was not changed.
- Presentation-event/audio suites pass. Required `npm run playtest`: 32/32.
- Production window launched through `play-native.ps1`, captured with
  `capture-window.ps1`, and viewed. Initial capture `live-first.png` exposed
  the disabled-menu defect described above. The final `live-title.png` capture
  was viewed after rebuilding: the scene renders and the menu is enabled.
  Computer Use was interrupted by the owner's physical Escape before final
  live click/paste verification. Those interactions remain automation-tested,
  not manually verified. The test client/server were left running, untouched.

## Still open — do not call the full splash/startup backlog done

- This is the native 3D foundation, not complete visual parity with the
  WIZARD prototype. Volumetric/rim clouds, waterfalls, regional weather,
  Crownlands alternate scene and adaptive quality remain to port.
- House-to-Scion creation still uses the existing intermediate ledger.
  Compact guided creation and the single explicit Hardcore creation toggle
  remain separate work. The title's Continue action preserves an existing oath.
- Full Unicode/IME composition, text selection and dictation integration are
  not claimed; clipboard support follows the existing ASCII name contract.
- First-launch shader/asset loading is synchronous. Device-loss recovery
  currently shows an explicit error rather than rebuilding the device.
- This does not complete the inventory/tree/portal/hotbar/notification backlog.

Launch this milestone: `powershell -NoProfile -ExecutionPolicy Bypass -File
native/tools/play-native.ps1 -BuildSubdirectory title-screen` from this clone.
F11 switches to the normal movable/resizable window. Saves use the persistent
account store introduced in the previous milestone.
