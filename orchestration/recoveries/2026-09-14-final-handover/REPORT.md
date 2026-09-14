# Combined inventory and unattended handover

## Delivered result

Source **2651bd4985e698fade744b8aeea5c84f9c251d57**, clean fresh build.
Normal entry **C:\Users\Alex\Documents\Verdigris Native 2026-09-13\Verdigris.exe**
now launches that package. The reproducible packaged copy is
`native/build/player-package-2651bd498/Verdigris.exe` in the consolidated checkout.
Both remote branches `codex/native-consolidated-20260913` and
`codex/native-reconstitution` were verified at this source after fast-forwarding
the shared branch from d0179c8b3. Later commits in this handover change only
reports/evidence, not game code or packaged assets.

Completed repairs: authoritative equipment/stat display; exact worn-seat to
backpack transfers and swaps; conserved UUIDs/quantities on rejected/full-pack
operations; pointer ownership/cancel/outside-drop; removed inventory action
strip/Character button/Equipment heading; non-backpack currency and coin count;
six server-gated per-Scion drawers; smaller centered close controls; textured
quiet wells/grid; bounded surface cache for responsive actual-item dragging;
minimum-viewport drawer clearance. Preserved accepted paperdoll arrangement,
m5x7 font, graphical House/Scion flow, perspective, authored poses and gameplay.

## Completed verification

- Eight native suites passed on the merged implementation (logs retained).
  The only subsequent game change was drawer screen geometry; the final package
  repeated the complete 80-scenario client suite including its new clearance test.
- Fresh native build/unchanged denylist; 1,270 package hashes, 349 required
  resources and embedded source identity matched before and after installation.
- All **80 final packaged scenarios passed**, exit 0. Real-server House/Scion
  buttons, appearance/names, perspective admission, return-to-title and Continue
  without old prompts; authoritative inventory/stats, full-pack rejection,
  exact equipment/compartment restart persistence, input/animation/combat and
  renderer regressions. Synthetic item/tree fixtures are explicitly QA data.
- Sustained actual-art dragging: 1,440 pointer events; 40 measured 3440x1440
  frames; **37.140 ms average, 50.528 peak**. Every painted ghost uses the latest
  pointer/grab offset, with no premature item changes or attacks. The gate
  remains average <40 ms. Moving fullscreen: **25.3 ms average, 35.2 peak**.
- Exact top-level launcher save/reload with **the same executable and the same
  isolated QA profile** restored Effects **70%**; both child processes exited
  cleanly. This is a volume setting, not completion percentage.
- Final physical candidate normal-profile smoke, then actual installed normal
  entry smoke passed. QueryFullProcessImageName proves both child paths in the
  installed folder, working directory matches, normal profile is installation
  `/profile`, normal settings override is absent, production title renders,
  menu Quit requires confirmation, and both owned processes exit 0.

Reproduce independently of Computer Use:

```powershell
powershell -NoProfile -File native/tools/verify-player-package.ps1 -PackageDirectory native/build/player-package-2651bd498 -EvidenceDirectory native/build/player-package-2651bd498/qa/verification
powershell -NoProfile -File native/tools/test-normal-launch.ps1 -InstallationDirectory 'C:/Users/Alex/Documents/Verdigris Native 2026-09-13' -EvidenceDirectory native/build/installed-check
```

Production captures actually inspected: inventory at 1280, before/after equip
with server ratings, rejected full-pack unequip, all drawer layout checks with
lowest drawer viewed at 960, sustained actual-weapon drag at original 3440,
long-name entry fixture, local pursuit walk/committed melee fixture, and exact
installed title showing Continue Scion and build 2651bd4985e6. Local pursuit
captures are local-session combat fixtures; hardware perspective admission is
verified separately by the remote consolidated flow in the same executable.

## Save preservation and rollback

Promotion copied only manifested game resources, not QA profiles, and preserved
the existing save byte-for-byte before startup. Normal smoke then compared every
preexisting save field, array element, UUID and quantity. Its only non-additive
change was currency's old `slot: 0` becoming null (currency no longer occupies
backpack cells). Added fields: packEligibility, packId, per-Scion passiveTree
and treeQuestPoints. Existing save values were otherwise preserved. Normal
settings remained absent throughout; no normal-profile settings were changed.

Save SHA256 before: `696F462912374876476010B497FE9C394FCD473E0ACF60CB6CBFA9D01FF427C1`.
After migration: `887E9B01EC3A084EC43BB065998D5A6198EFC4FD7F22D156E27103F490BE1040`.
Original full installation/save retained at
`native/build/normal-launch-rollback-before-2651bd498/`.
Installed launcher SHA256: `34DC1993D07150D53CB65A885FCB48C18FA566AA4DB2E78CD04F9C3A4F004FCE`.
Installed client SHA256: `59EB475CACEE5DF96FB2D899CB156F5ECF3E18F449EE2B356184B553E457B791`.

## Remaining limits

No failed final automated checks, pending build, launch promotion or push.
Computer Use stayed stopped after Escape. No alternative desktop input tool
was used. The interrupted human walkthrough remains unperformed; automated
captures are not owner acceptance and do not establish the cause of that
earlier interruption. New auxiliary loot/art/abilities were not invented:
extension transfer tests use labeled QA category fixtures. The bounded average
frame gate passed; individual peaks above 40 ms are reported, not hidden.

## Development checkpoints (superseded by the delivered result)

Final layout correction: inspection of the 9e8e60741 package at 960x600 found
the lowest drawer overlapping the action bar. Drawer geometry now shares the
inventory/HUD vertical boundary. Added explicit nonintersection assertions for
all six drawers at all three viewport sizes. The focused scenario passed,
corrected minimum-size capture was inspected, and actual-art sustained drag
remained below the unchanged gate (31.036 ms average, 34.365 peak).

Foundation: b21054d63 inventory extensions, merged with typography branch
d4498d2af (owner-selected m5x7 implementation cefd238b2). Preserved accepted
paperdoll composition, authoritative equipment/stats, purse, six skill-gated
drawers, exact-cell transfers, outside drops, perspective and animation.

Sustained drag benchmark exercises the real window handler with 1,440 pointer
events and 40 measured 3440x1440 production frames. It verifies latest-pointer
ghost anchoring, item conservation and absence of attacks. Initial failure:
75.327 ms average, including 51.196 ms HUD cost (earlier run 83.538 ms).
Bounded per-asset well cache preserves gradient/texture pixels and avoids
recompositing unchanged surfaces. Actual authored-weapon drag now averages
36.290 ms (45.187 peak); the unchanged gate is average below 40 ms.

All eight native suites passed. The enhanced inventory scenario passed after
the cache and real-art benchmark changes. Captures inspected: before/after
equipment composition with authoritative ratings and sustained drag at 3440.
The dragged auxiliary test objects remain explicitly labeled synthetic QA
fixtures; no production auxiliary loot, art or combat abilities were invented.

New normal launcher smoke uses an app-owned hidden title window, actual normal
profile selection, confirmed menu Quit, and owned server shutdown. It records
actual child image paths with QueryFullProcessImageName (avoids a MainModule
enumeration startup race reproduced during development), and working directory.
Test script snapshots existing saves/settings, accepts only additive saved
fields or the authorized currency-coordinate migration, and verifies clean
exit of both owned children. Development physical-copy smoke passed, including
a copy of the owner save; actual owner installation was untouched at this point.
The development junction fixture correctly failed the stronger actual-image
path assertion; the physical fixture passed without weakening the check.
Same-executable, same-QA-profile Effects 70% save/reload also passed.

Computer Use remains stopped. No desktop input was resumed. These checks are
application-owned regression fixtures; they do not assert owner live acceptance
or retroactively complete the interrupted desktop walkthrough. Final package,
installed normal smoke and shared-baseline handover are pending this checkpoint.
