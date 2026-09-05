# Direct town portals — 2026-09-04

The Crossroads now exposes four authoritative portals in the town scene.
Each portal names its road and next unfinished, unlocked destination. A
nearby click sends `world:portal:use`; the server validates the portal id and
proximity, then enters that exact road node directly. Walking over a gate no
longer opens a chart, and the old `N` shortcut is debug-only behind F3.

Town portals render on top of the authored road-gate scenery. Hovering the
whole visible arch turns it white and shows a Framekit destination chip. The
Wayfinder chart remains available as the deliberate route-selection surface.
Returning through the in-instance exit still uses the forgiving visible
waymark and restores the four town gates at the Crossroads fountain.

## Verification

- `native/build.ps1 -RunTests -BuildSubdirectory direct-portals`: exit 0;
  core, networking, camera, session, presentation and audio suites pass.
  Log: `native/build/direct-portals-tests.log`.
- `native/build.ps1 -RunClientScenarios -BuildSubdirectory direct-portals
  -CaptureRoot native/build/direct-portals/captures`: exit 0; all 36 client
  scenarios pass, including compact/wide portal hover captures and the
  click-enter/return loop. Fullscreen frame budget remains under 40 ms.
  Log: `native/build/direct-portals-scenarios.log`.
- `npm run playtest`: 32/32 gameplay scenarios pass. Log:
  `native/build/direct-portals-playtest.log`.
- Viewed the generated portal hover capture:
  `native/build/direct-portals/captures/portal-hover-1366x768.png`.
  This is production painter evidence; a live owner-window interaction pass
  is still separate.

## Still open

Selectable LMB/RMB/Q/E/R/T bindings and Warcry unlock acquisition remain a
separate progression decision. Inventory/tree composition and the remaining
WIZARD atmosphere work are still tracked in `owner-2026-09-03.md`.
