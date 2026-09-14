# Native equipment repair and handover — 2026-09-13

Implementation is published and the final packaged regression gates pass. **Normal-launch handover is not complete.** Physical Escape stopped Computer Use during the final live walkthrough. No further game input or launch promotion was performed after that stop.

## Identifiable implementation

- Checkout: `C:\Users\Alex\Documents\ChatGPT\verdigris-consolidated-20260913`.
- Working branch: `codex/native-consolidated-20260913`.
- Final packaged implementation: `43c104c5acca73805ebb6184079a7fb0d9c7be17`, clean.
- Executable: `C:\Users\Alex\Documents\ChatGPT\verdigris-consolidated-20260913\native\build\player-package-43c104c5a\Verdigris.exe`.
- Packaged client: the same package's `native\build\verdigris_client.exe`.
- Asset root: the same package's `native\client\assets\raster\runtime`.
- The final handoff/evidence commit follows the packaged source without changing game code. Both native development branches receive that documentation-only head by ordinary fast-forward pushes; verify their exact remote hashes in the delivery message.

The merged renderer/recovered-application histories and accepted inventory composition remain intact. No art, balance, campaign or equipment mechanics were added.

## Concrete repairs

Character values now come from authoritative session fields, including explicitly exposed attack and defense ratings. Missing values say unavailable; attack rating is no longer reused as a guessed critical-chance value.

Inventory selection follows the item's UUID through its whole footprint, equipment changes and separated acknowledgement packets. Buttons act on that visible selection, show pending state, disable unavailable operations, and clear pending state on rejection or connection loss. Keyboard selection survives stationary mouse hover. Drag cancellation, modal wheel ownership, route/talk bindings and text-entry isolation are covered.

Backpack rearrangements previously existed only in the client. They now use server-authorized moves/swaps, atomic footprint validation and explicit acceptance/rejection. Invalid equipment, two-handed restrictions, occupied-seat transitions and full-backpack unequip preserve items and show useful feedback.

Disk saves previously omitted inventory and equipment. They now preserve exact instances, rolls, UUIDs, quantities and positions, with separate Scion loadouts, House storage and bank storage. Persisted UUIDs reserve their serials before more items are created. Invalid saved loadouts are not overwritten.

The live walkthrough found two additional save defects that the original one-restart fixture missed:

1. **Existing saved entries did not update.** After loading a save, insertion retained the older snapshot instead of replacing it. `a317b898d` replaces the active entry with current authoritative state. The new second-restart regression failed before this change and passed after it.
2. **Re-admission granted another starter kit.** The one-time admission flag was not persisted; an equipped dagger was absent from the carried-item check. `43c104c5a` persists the admission flag and migrates previously admitted Scions. A graphical re-admission regression failed before this change and passed after it. Existing QA duplicates from the failed candidate were retained as evidence, not silently deleted.

Long selected-item names wrap within the viewport; full quantities remain available in tooltips while grid counts are bounded. Tooltips and panels draw above incidental HUD text. The launcher prevents a normal launch from inheriting QA settings overrides. Native exit logs identify confirmed menu quit and window-close messages.

Implementation files: `native/client/{main.cpp,inventory_ui.hpp,character_ui.hpp,inventory_scenarios.hpp,ui_acceptance_scenarios.hpp,client_model.hpp,remote_session.cpp,session.hpp,local_session.cpp,presentation_events.hpp}`, `native/include/verdigris/{core.hpp,networking.hpp}`, `native/src/{core.cpp,networking.cpp}`, `native/tests/networking_tests.cpp`, and `native/tools/{player-launcher.cs,PLAYER-README.txt}`.

## Completed final-package checks

- **79/79 packaged scenarios**, exit 0, on the exact `43c104c5a` client. [Complete log](evidence/packaged-scenarios.log).
- **Eight native suites passed:** core, networking, camera2d, Fable camera, session, presentation events, audio mixer and user settings. Logs are `evidence/final-*.log`.
- **1,266 package hashes, 346 required resources, clean embedded source identity and profile-lock alias checks passed.** [Package gate](evidence/package-check.log).
- Final frame budget at 3440×1440: **22.055 ms average**, moving **26.862 ms average / 36.123 ms peak**, with the existing 40 ms limit unchanged. [Metrics](evidence/frame-budget-report.txt).
- Production-window scenarios backed by a real native server exercise all ten destinations with supported factory items: main/off hand, body, head, feet, belt, neck, gloves and both rings. They cover buttons, tail-cell dragging, compatible/incompatible transitions, full backpack, item conservation, input isolation, exact persisted equipment/positions, graphical House/Scion/Continue, perspective, authored animation, movement, melee and progression.
- Final captured equipment/stats, rejection feedback and long-name/count views were inspected at 960×600, 1280×800 and the owner's 3440×1440 size. This is bounded presentation evidence, not blanket visual acceptance.

| Evidence | What it establishes |
| --- | --- |
| [Before equip](evidence/handover-before-equip.png), [after equip](evidence/handover-after-equip.png) | Real-server fixture items move from backpack to supported seats; attack/defense values change from 12/5 to 23/28. The after capture also shows the rejected two-handed attempt. |
| [Full backpack rejection](evidence/handover-full-pack-rejection.png) | Equipped armor remains seated with an 84/84 pack and explicit refusal. |
| [960](evidence/handover-long-name-960.png), [1280](evidence/handover-long-name-1280.png), [3440](evidence/handover-long-name-3440.png) | Deliberately labeled presentation stress: a frozen copy of real session data with a synthetic long name/count and large values. These synthetic values were not sent to authority or saved. |
| [Final title](evidence/final-package-title.png) | Exact final executable launched and visibly identifies build `43c104c5acca`. |
| `tonight-save-regression-{red,green}.log`, `tonight-kit-regression-{red,green}.log` | Both newly discovered restart failures reproduced before their repairs and passed afterward. |

## Live walkthrough: completed versus pending

Before the stop, candidate `071f87f53` completed graphical House/female Scion creation, Set Out, perspective movement, Equip/Unequip buttons, tail-cell drag equip, Return to title and Continue. Effects 90% survived restarting the same executable with the same QA profile. Its live equipment restart failed; [the captured failure](evidence/071-restart-wear-failure.png) is not a passed check.

On `a317b898d`, live equipment and authoritative stats survived restart, and Effects 80% survived restarting that same executable/profile. Exact loadout comparison then exposed the extra starter dagger; [that failure](evidence/a317-restart-duplicate-found.png) prompted the final admission repair.

On final `43c104c5a`, the launcher and source-identifying title were checked. **The final live equipment/settings reload repeat was interrupted by physical Escape while attempting to open Settings.** No subsequent game input was issued. The user-controlled QA client was left alone. Do not transfer the preceding candidates' live passes into claims that the complete final walkthrough passed.

QA profile used throughout: `C:\Users\Alex\Documents\ChatGPT\verdigris-consolidated-20260913\native\build\player-package-071f87f53\qa\live-profile`. Its folder name identifies its origin, not the executable currently using it. The final launcher explicitly used this isolated profile; it did not use the normal save. Both restart sides of any resumed check must use the exact final executable and the same chosen QA profile.

The bounded live attack attempt on `071f87f53` produced an out-of-range Thrust effect and continued running: [before](evidence/071-attack-before.png), [swing](evidence/071-attack-swing.png), [after](evidence/071-attack-after.png). These frames establish a swing, **not damage to a foe**. Final-package automated melee/animation scenarios passed. The earlier historical clean exit cannot be attributed retrospectively because that binary lacked exit-source logging; this run recorded expected menu quits and Alt+F4 closes explicitly, with clean client/server exits. No crash was inferred.

## Normal launch and preservation

The actual established entry is the direct executable in the already-open Explorer folder:

`C:\Users\Alex\Documents\Verdigris Native 2026-09-13\Verdigris.exe`

It remains the old installation at packaged source `3bd164d34f189d1438dc68990667312e8ecd721f`. **It has not been promoted.** No additional normal shortcuts were created.

The previous installation and save are recoverably copied to `native\build\normal-launch-rollback-8731c3646\installation`. The normal save SHA-256 remains `C5CFAC37532FE1A6B7413B4BC8AD1E4A8B8602C5C899CAE9D410E5BAD722045C`; no QA items or settings were written there. Normal `%LOCALAPPDATA%\Verdigris\settings.ini` was absent and remains absent.

The prepared, unexecuted `native\build\promote-verified-handover.ps1` validates source/hash/path containment and copies only manifest-listed package files into that existing installation, preserving its profile. After computer control is resumed and the final live reload succeeds, execute the in-place promotion, verify installed hashes, and smoke **the Explorer entry itself**, recording its real process/asset/profile identity and clean owned-server shutdown. Preserve the owner's live activity; do not close or replace the stopped QA session through another input mechanism.

Remaining limits: there is no factory item for the existing back seat, and warhorn/quick-rig/attendant seats do not have implemented equipment mechanics. This task did not invent those extensions. The remaining task blocker is the explicit Computer Use stop, not missing implementation approval. Completed verified work is published under the standing commit/push policy; resuming the live check does not require another planning approval.
