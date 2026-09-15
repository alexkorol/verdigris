# Fantasy main-menu title

The main screen uses generated natural-gold fantasy lettering over the existing
illustrated gateway. Small pixel text, buttons, backdrop, settings and menu
navigation are unchanged. Both local-review and online title screens use the art.

## Direction and asset

The owner asked for title artwork and explicitly rejected name-derived patina.
`native/client/assets/raster/AGENTS.md:21` already states: “Use natural material
colors. The project name is not an art direction.” The first generation prompt
missed that guidance. Its green-patina result was rejected and never added to
runtime; the existing accepted menu artwork was not recolored.

Selected asset: `native/client/assets/menu/title-gold-v1.png`, generated with the
built-in image_gen tool. Exact final prompt, source file and rejected-candidate
record: `native/client/assets/menu/title-gold-v1.provenance.json`.
SHA-256: `0c74c1efca9512b4af20540431213c95415eb024288559ff665d9b0a389dae5f`.
The original 2083x755 PNG is copied unchanged. Read-only alpha/color inspection
found 1,228,486 fully transparent pixels, 282,648 pixels with alpha >128 and zero
green-dominant pixels under the recorded criterion.

The first rendered candidate was undersized because distant alpha values below
16 inflated the measured bounds. The loader now measures readable ink once, with
four pixels of padding, and the renderer fits that region into the title space
without distorting the aspect ratio. A regression checks that invisible canvas
specks do not shrink the wordmark. No per-frame pixel scan was added.

## Source and package

Runtime source: `13d127e5a20fb91664c23a0b3e310d211f03c217 clean`.
Branch: `codex/native-service-coop-20260914`; origin:
`https://github.com/alexkorol/verdigris.git`. Runtime push verified with ls-remote.
Package: `C:/Users/Alex/Documents/Verdigris Service QA 2026-09-14-r5/Verdigris.exe`.
ZIP: `C:/Users/Alex/Documents/Verdigris Service QA 2026-09-14-r5.zip`.
Manifest includes 1,288 files and the new required title asset.
Client SHA-256: `cf92042f72cca378f3922577722d29147297e4747eeff0048c64f0f8938eceba`.
Launcher SHA-256: `bb3eb8d248559e41b44547bae163026b90364c02261a6434774b1efb159975ad`.
ZIP SHA-256: `7de1b1a3bee2f00f30967197ae8815e27314bc17f3c5f255d153691ff999013f`.

## Verification

- Focused native menu/particle scenario passed, including generated title loading,
  actual art rendering, visible ink bounds and control containment.
- Viewed 640x480 and 1920x1080 production captures after correcting the alpha bounds.
- Fresh clean-source `package-native.ps1` build passed; archive executable hashes
  were verified against the manifest. No private profiles or credentials enter ZIP.
- Exact packaged online launcher passed, including actual child image/arguments,
  independent service health and owned-process shutdown. Viewed its 3440x1440
  `online-account-start.png` showing the gold title; the later capture shows the
  credential page. Evidence: `native/build/title-r5-online-final/launcher`.
- All 84 exact packaged scenarios passed, exit 0. Both local-review launcher
  lifecycles and same-executable/same-profile settings restart passed. Viewed the
  packaged 1280x800 title capture. Wide captures are preview-scaled by the viewer;
  the 1280x800 image was viewed without preview downscaling.
- Unchanged 40 ms average gates passed at 3440x1440: 258 particles 33.833 ms;
  inventory drag 27.455 ms average/33.966 peak; stationary scene 25.5 ms;
  moving scene 32.7 ms average/47.0 peak. This is not a 60 fps claim.
- Existing background/button hashes match their original provenance.

The first disposable launcher helper queried health before service readiness and
PowerShell treated stderr as a terminating native-command error. Its service was
explicitly stopped; the corrected readiness loop and a fresh evidence directory
passed. This was a test-wrapper correction, with no service or game code change.

Evidence: `native/build/title-gold-captures-final`, `native/build/title-r5-online-final`,
`native/build/title-menu-check-final.log`, `native/build/package-player-r5.log`,
and package `qa/verification`. The earlier small-title captures are retained in
`native/build/title-gold-captures` as rejected sizing evidence.

The normal installation and owner profiles were not targeted. R4 remains a
recoverable baseline. This title update does not deploy the service, promote the
normal installation or claim owner visual acceptance. The service/external-host
limitations in the co-op report remain unchanged.
