# Native equipment repair and handover — 2026-09-13

**Normal-launch handover is complete.** The established launcher now runs verified packaged source `43c104c5acca73805ebb6184079a7fb0d9c7be17`. Final live settings and loadout restart checks passed, the installed package passed its hash/resource gate, and the normal-profile launcher completed a title-only smoke check and confirmed menu quit with clean client/server shutdown. See the completion evidence below for the precise scope and tool limitations.

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

## Earlier live walkthrough and repaired failures

Before the stop, candidate `071f87f53` completed graphical House/female Scion creation, Set Out, perspective movement, Equip/Unequip buttons, tail-cell drag equip, Return to title and Continue. Effects 90% survived restarting the same executable with the same QA profile. Its live equipment restart failed; [the captured failure](evidence/071-restart-wear-failure.png) is not a passed check.

On `a317b898d`, live equipment and authoritative stats survived restart, and Effects 80% survived restarting that same executable/profile. Exact loadout comparison then exposed the extra starter dagger; [that failure](evidence/a317-restart-duplicate-found.png) prompted the final admission repair.

The first final-package reload attempt paused when Escape intended to close a game pane was intercepted by Computer Use's stop shortcut. This was not a deliberate request to abandon the work. No further game input followed that stop. The owner subsequently requested “finish the handover”; the completed final-package checks below were performed after that resumption and do not borrow earlier candidates' live results.

QA profile used throughout: `C:\Users\Alex\Documents\ChatGPT\verdigris-consolidated-20260913\native\build\player-package-071f87f53\qa\live-profile`. Its folder name identifies its origin, not the executable currently using it. The final launcher explicitly used this isolated profile; it did not use the normal save. Both restart sides of any resumed check must use the exact final executable and the same chosen QA profile.

The bounded live attack attempt on `071f87f53` produced an out-of-range Thrust effect and continued running: [before](evidence/071-attack-before.png), [swing](evidence/071-attack-swing.png), [after](evidence/071-attack-after.png). These frames establish a swing, **not damage to a foe**. Final-package automated melee/animation scenarios passed. The earlier historical clean exit cannot be attributed retrospectively because that binary lacked exit-source logging; this run recorded expected menu quits and Alt+F4 closes explicitly, with clean client/server exits. No crash was inferred.

## Normal launch and preservation

The actual established entry is the direct executable in the already-open Explorer folder:

`C:\Users\Alex\Documents\Verdigris Native 2026-09-13\Verdigris.exe`

It now contains verified packaged source `43c104c5acca73805ebb6184079a7fb0d9c7be17`. The existing executable remains the one normal entry; no additional normal shortcuts were created. Its child client and server run from this installation, and the launcher sets both child working directories to this installation root.

The previous installation and original save are recoverably copied to `native\build\normal-launch-rollback-8731c3646\installation`. Promotion preserved the original save bytes (SHA-256 `C5CFAC37532FE1A6B7413B4BC8AD1E4A8B8602C5C899CAE9D410E5BAD722045C`). Normal startup then added `bankItems`, `houseStore`, `kittedScions` and `scionLoadouts`; every preexisting save field was compared and remained unchanged. The migrated file SHA-256 is `50EC9C97FBB9DE9CC172FDBA4BEAB27155A984971334FE9C72031249FB847FFF`. No QA items or settings were copied to the normal profile. Normal `%LOCALAPPDATA%\Verdigris\settings.ini` was absent and remains absent.

The executed `native\build\promote-verified-handover.ps1` validated source/hash/path containment and copied only 1,266 manifest-listed package files into the existing installation, preserving its profile and the rollback copy. The installed gate passed all hashes, 346 required resources, embedded source identity and profile-lock checks. [Promotion](evidence/completion/promotion.txt), [installed gate](evidence/completion/installed-package-check.log).

## Completed final live handover

- Exact final launcher `43c104c5a`, original QA profile: Effects changed from 80% to 70%, saved, client/server closed, the same executable restarted with the same profile, and Settings visibly remained at 70%. [Saved](evidence/completion/03-effects-70.png), [restarted](evidence/completion/08-restart-effects-70.png). The original QA profile had since been played to level 9 with items in House storage; its state was preserved.
- Equipment fixture: a separate isolated `native/build/handover-evidence/resumed-live/loadout-profile` was seeded from the existing `final-live/accepted-save.json` fixture. It was not an owner-save reset. In the final executable, Unequip moved the worn dagger into the backpack; dragging its second footprint cell moved it to another position. After process restart with that same fixture profile, the exact complete saved loadouts matched and the new position was visible. [Move](evidence/completion/14-move-accepted.png), [restart](evidence/completion/17-restored-position.png), [comparison](evidence/completion/restart-check.txt).
- The restored dagger was drag-equipped from its second cell, followed by another complete process restart and Continue. It stayed equipped, with no extra starter item; full loadout JSON matched, including UUIDs, quantities and positions. [Equipped](evidence/completion/18-reequip.png), [restarted attachment](evidence/completion/21-wear-gameplay.png), [comparison](evidence/completion/wear-restart-check.txt).
- Final-package live movement changed the perspective scene, and entering the route then pressing Q produced a captured whiff effect while the client continued running. This establishes input/effect continuity, not enemy damage or a complete authored attack pose sequence. The final packaged animation/melee regression scenarios provide that separate coverage. [Movement](evidence/completion/22-movement.png), [route effect](evidence/completion/route-thrust.png), [after](evidence/completion/25-route-after-thrust.png).
- Return to title and Continue returned to the same equipped route session. [Title](evidence/completion/29-return-title.png), [Continue](evidence/completion/30-title-continue.png). All fixture sessions shut down cleanly; completed launcher logs are retained alongside these captures.
- Normal-launch smoke: Explorer identified the exact `Verdigris.exe` file in the established folder. Its double-click automation failed twice with `coordinate input geometry is unavailable`; screenshot capture also reported the existing `SetIsBorderRequired` interface failure. The supported Windows `launch_app` API then started that **same top-level normal launcher**, with no QA arguments. Its lack of its own visible window made the API report no targetable launcher window, but fresh process/window selection confirmed its child game and owned server. This was not a direct launch of the inner client. [Process chain](evidence/completion/normal-processes.json), [visible normal title](evidence/completion/31-normal-title.png).
- The normal launcher log confirms the installation asset root, source `43c104c5a`, Fable perspective renderer, 96 authored poses, normal installation `profile`, and `isolatedSettings=False`. A title-menu Quit and confirmation produced `confirmed menu quit`, client exit 0 and owned server exit 0; no Verdigris processes remained. No normal-save gameplay was performed. [Launcher log](evidence/completion/normal-launch.log), [preservation and shutdown](evidence/completion/preservation.txt).

Remaining limits: no factory item exists for the back seat; warhorn/quick-rig/attendant equipment mechanics remain unimplemented. A seat-to-backpack drag attempt did not transfer the dagger; the visible Unequip button did. The successful drag checks cover backpack movement and drag-to-equip. Explorer double-click automation itself remains unverified, although the actual normal launcher entry and its full lifecycle were verified through Windows launch. Historical attack-exit attribution remains unavailable; no crash or enemy-hit claim is made. These limits are separate from the completed normal-launch handover.

Concurrent source edits appeared in the checkout during the final handover. They were preserved, excluded from this evidence-only commit, and are not in the `43c104c5a` package. Both native branches share the verified committed implementation and this handover record; future changes must be built and verified before changing the normal installation again.
