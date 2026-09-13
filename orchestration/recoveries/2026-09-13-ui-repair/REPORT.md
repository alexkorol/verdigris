# Native UI repair verification — 2026-09-13

Implemented in the consolidated native game, with no renderer reset, save
migration, or replacement game. See [implementation](IMPLEMENTATION.md) and the
[binding UI contract](../../../docs/product/NATIVE_UI_ACCEPTANCE.md).

## Exact package and source

- Checkout: `C:/Users/Alex/Documents/ChatGPT/verdigris-consolidated-20260913`
- Working branch: `codex/native-consolidated-20260913`
- Implementation: `5ce492be5a69e435988a23578c59b0f11990e9ef`
- Packaged source: `f8770d4e7ff493c49ee06122ca2c100f8e9aba26` (clean).
  This adds portable artwork-provenance paths to the implementation commit.
- Executable: `C:/Users/Alex/Documents/ChatGPT/verdigris-consolidated-20260913/native/build/player-package-f8770d4e7/Verdigris.exe`
- Launcher SHA-256: `35d98fe7e2d623c5c2cabd3ac67162ea1061b57de11e38541b9adcbd942c48fb`
- Packaged client SHA-256: `530ef1cdb02945edf1ecace1e2ff294f2a7fe33ac81359ceba6025ec3979f934`

Later changes in this recovery are evidence/handoff documents only. The packaged
source remains the exact game code. Both `5c388d9fd` and `443fb2503` are ancestors.
The ordinary launch at `C:/Users/Alex/Documents/Verdigris Native 2026-09-13/Verdigris.exe`
and its saves have not been changed. Use the executable above to see this repair.

## Completed automated checks

The fresh package was built with `native/tools/package-native.ps1`, then tested
from its own working directory with its own assets and isolated settings.

| Check | Result |
| --- | --- |
| Embedded source ID and clean-build manifest | Pass |
| Package file SHA-256 checks | 1,266 / 1,266 |
| Required resources, artwork provenance, profile locking | 346 resources; pass |
| Packaged `--scenario all` | **79 / 79 scenarios; exit 0** |
| Native test executables | **8 / 8; exit 0** |
| Imported object transparency and handstone alpha | Pass; real transparent backgrounds |
| 3440x1440 moving frame budget | 28.9 ms average; 52.4 ms peak; existing <40 ms average gate passed |

The eight native test executables cover core, camera2d, Fable camera, networking,
session, presentation events, audio mixer, and user settings. The session test
includes death, succession, heirloom retention and reconnect. These ran before
the final clean rebuild; no C++ changes followed them.

The packaged suite retains movement, collision/entry clearance, authored
animation, strike contact, gameplay progression and memory/performance coverage.
It also exercises actual inventory handlers against a native WebSocket server:
full footprints; compatible destinations; wrong-seat/two-hand/full-pack rejects;
exact UUID conservation; visible Equip/Unequip; cancellation; authoritative
attack/defense; and no combat latch left by UI input.

Full [packaged scenario output](evidence/packaged-scenarios.txt) and
[integrity output](evidence/package-integrity.txt) are committed. The package's
`qa/` directory retains the complete capture set and launcher logs.

## Completed visual and live checks

Actual package captures at **960x600, 1280x800, 1366x768 and 3440x1440** show the
equipment-first arrangement, art rather than abbreviation buttons, square bag
cells, contextual comparison, compact character values and bounded feedback.
The populated fixture uses real server-owned items. It is not a novice loadout.

- [Before: previous native package, 960x600](evidence/before-inventory-960.png)
- [After: repaired native package, 960x600](evidence/inventory-960.png)
- [After: 1280x800](evidence/inventory-1280.png)
- [After: normal 3440x1440 display](evidence/inventory-3440.png)
- [Valid held drag](evidence/ui-valid-destination.png), [invalid destination](evidence/ui-invalid-destination.png), [rejection with retained item](evidence/ui-placement-rejected.png)
- [Equipment and authoritative values](evidence/ui-equipment-and-stats.png), [contextual tooltip](evidence/ui-contextual-tooltip.png)
- [Ordinary production gameplay, F3 off](evidence/ui-play-1280.png)

The actual `Verdigris.exe` launcher was also run separately from the scenario
process. The final live profile was `qa/live-profile-final` in this package.
Win32 messages targeted the process's verified `VerdigrisNativeClient` window;
PrintWindow captures used full-content rendering. This was an automated live
walkthrough, not an assertion that the owner operated or accepted it.

The live walkthrough completed Begin your House → Found your House → female
selection → Create female Scion → Set out, using the rendered buttons, with no
House/Scion command prompt. Movement changed the actual world view. Settings
opened using the on-screen button. Effects was changed to **90% volume**, saved
as `sfx=900`, followed by Return to title and a clean Quit. A fresh launcher/client
process on the **same executable and same profile** displayed Effects 90%.
Another launch used Continue to return to the existing Scion and enter a route.

Evidence: [House](evidence/live-final-house.png), [created Scion](evidence/live-final-created.png),
[movement](evidence/live-final-moved.png), [Continue](evidence/live-final-continue.png),
[settings after restart](evidence/live-final-restarted-settings.png).

## Remaining limits and failures

**No final automated check failed.** Two packaging attempts failed during work:
Windows separators in conversion provenance, then a linker file lock caused by
running the session test during a rebuild. Both were corrected; the successful
package was freshly rebuilt and passed integrity and all 79 scenarios afterward.

The separately timed live attack-frame capture remains incomplete. Its operation
failed because client PID 18076 had exited before the capture command; the launcher
recorded a clean exit 0, not a crash. A preceding live frame shows a Stone Lurker
kill, hit feedback and level 2, but that is [combat aftermath](evidence/live-final-combat-entry.png),
not proof of a captured, ordered attack animation. Packaged automated strike and
animation checks passed. Do not conflate these forms of evidence.

All 14 equipment regions are preserved. Ten have existing native item content
in the representative fixture. Cloak, Warhorn, Quick Rig and Attendant remain
empty without invented content. Unmapped item variants retain their real names;
this is not a claim that every possible item variant has new artwork.

The title and Settings retain their existing menu design; this repair changes
inventory/character composition and ordinary gameplay clutter. No owner visual
acceptance or normal-launch promotion is claimed. Further development continues
on this same consolidated lineage under the standing commit/push authorization.
