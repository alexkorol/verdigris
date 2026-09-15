# Native reconstitution handoff

## Starter preview integration — 2026-09-15

Active branch: `codex/starter-slice-20260915`, PR #61 into `master`.
The owner requested merge and publication of the Windows preview. Normal
branch pushes alone must not be reported as a shipped release.

The release candidate at `Z:/Code/.packages/verdigris-starter-release-final-c8cff89ac`
is built from clean source `c8cff89ac2158ebb16b80f152f9f044f40d19ade`.
All packaged scenarios, two launcher lifecycles, source/resource checks, and
same-profile settings restart passed. Its production starter capture was
inspected. The archive contains only manifest resources, with no QA profiles
or saves; every archived resource hash was checked. Enemy art remains
provisional. The starter uses seeded woodland scenery instead of house sprites.

Integration uncovered CMake-only defects: missing audio linkage, wrong runtime
asset directory, a single-config preset without a release build type, and an
unnormalized capture-root ancestor. The hidden inventory fixture now allows
its requested test viewport beyond a small desktop's default tracking limits
and asserts that input and painted dimensions agree. The nested-directory
capture/frame-budget regression passed. Hosted CI must be checked again after
these repairs; do not describe its earlier failed run as passing.

Publication is still pending at this record. Verify PR merge, tag identity,
and public release assets before reporting shipment. The release tag must
identify the exact packaged source above, even if later integration-only fixes
are present on master. Preserve all original commits when merging.

## Current handover — gameplay particles installed

Normal entry: `C:/Users/Alex/Documents/Verdigris Native 2026-09-13/Verdigris.exe`.
Installed clean source: `30ed52c842cc0500db2b3cf0a6397454c52410c8`.
Checkout: `C:/Users/Alex/Documents/ChatGPT/verdigris-consolidated-20260913`.
Working branch: `codex/native-consolidated-20260913`; shared baseline:
`codex/native-reconstitution`. Implementation pushed to both, remote SHAs verified.
Later handover commits contain only this record/evidence, not new game code.

Existing playable War Cry, dash, critical contact and ground pickup now emit
native particles from confirmed events. World-space rings expand/gather using
the existing tiny atlas and fixed 32-emitter/384-particle budgets. Inventory
refreshes and rejected actions cannot fabricate these effects. Existing level-up,
melee/death, menu, inventory and Crossroads persistence improvements remain.
The renderer now copies directly to the native DIB, materializing a separate
owned diagnostic snapshot only on request; exact pixel checks cover that path.

Eight native suites and **82 exact packaged scenarios passed**. Same executable
and same isolated QA profile saved/reloaded Effects 70%. Final package timing:
258 particles 37.449 ms, sustained inventory drag 36.519 ms (39.433 ms peak),
moving fullscreen 24.3 ms; all unchanged 40 ms average gates passed. Final
production particle captures and the actual normal 3440x1440 title were inspected.
Normal launch passed source/child paths/cwd/profile/perspective/menu Quit/cleanup.
The existing save is byte-identical before/after launch; normal settings stayed
absent. No owner game was killed and no QA profile was promoted.

Rollback: `native/build/normal-launch-rollback-before-30ed52c84/`.
Package: `native/build/player-package-30ed52c84/`.
Report: `orchestration/recoveries/2026-09-14-gameplay-particles/REPORT.md`.
The first candidate failed drag performance and was never promoted; the report
records the cause, code fix and successful replacement checks. No failed check
or verification task remains. Further content scope remains playable fire-vessel
tags/sockets, Burning Touch and authoritative flying projectiles. Their recipes
are preserved; none are falsely presented as existing gameplay. Continue
implementation and verified commit/push under the standing owner authorization.

## Previous handover — illustrated menus and native particles installed

Normal entry: `C:/Users/Alex/Documents/Verdigris Native 2026-09-13/Verdigris.exe`.
Installed clean source: `3cee73d2667dfcf7e77b4cdd910399bcc20b933f`.
Checkout: `C:/Users/Alex/Documents/ChatGPT/verdigris-consolidated-20260913`.
Working branch: `codex/native-consolidated-20260913`; shared baseline:
`codex/native-reconstitution`. Both receive the same verified implementation
and handover through ordinary fast-forward pushes. No competing launch.

The title/settings/pause/quit menu now has original illustrated cedar/bronze
art and physical enamel controls. The real button-based House/Scion creation,
name fields, appearance choices and Continue remain intact. The bounded native
particle module has external recipes and a tiny atlas: layered level-up column,
descending streaks, sparks, dust, embers, contact flare and reusable trails.
Confirmed server XP crossings trigger level-up; login does not replay it.

All eight native suites and **81 exact packaged scenarios passed**. The same
executable saved/reloaded Effects 70% in one isolated QA profile. Production
captures, including the actual normal 3440x1440 title, were inspected. Sustained
drag averaged 34.582 ms and moving fullscreen 25.3 ms under unchanged 40 ms gates.
Normal-entry smoke verified source, child images, cwd/profile, perspective
startup, confirmed menu quit and process cleanup. The owner's save remained
byte-identical even after normal launch; normal settings remained absent.
Crossroads items/coins/equipment preservation and bank recovery remain included.

Full prior installation/save rollback:
`native/build/normal-launch-rollback-before-3cee73d26/`.
No owner game process was killed and no QA profile was promoted. Build/verification
used app-owned native diagnostics without Computer Use or desktop input.

Remaining content work: a playable fire-bowl item/tag and per-frame sockets,
Burning Touch gameplay and authoritative flying-projectile gameplay. Their
recipes/API are shipped; they are not invented new combat actions. No failed
check or verification task remains. Continue authorized implementation and
commit/push completed verified work without adding an approval checkpoint.
Only handover/evidence changes follow the packaged source.

Report and captures: `orchestration/recoveries/2026-09-14-menu-particles/REPORT.md`.
Module/trigger details: `native/client/vfx/README.md`.

## Previous handover — Crossroads return fixed and installed

Normal entry: `C:\Users\Alex\Documents\Verdigris Native 2026-09-13\Verdigris.exe`.
Installed clean source: `34e855c1a4124cba42fd8c008863094454578c42`.
Safe return retains the living Scion's items, coins, equipped seats and ratings.
The owner's existing 370 coins were recovered from legacy extraction storage
into Rhea's Countinghouse bank; use its single-click **withdraw all** row.
No equipment ownership or seat was guessed, and no QA profile was promoted.

All 80 final packaged scenarios, eight native suites, same-executable/same-QA-
profile settings restart, copied-save migration/restart and the installed
normal-entry smoke passed. Actual window, bank and return captures inspected.
Final drag average 39.880 ms; moving fullscreen average 25.4 ms, under the
unchanged 40 ms average gate. Earlier failed candidates were never installed;
their failures and corrections are retained in the report.

The owner game closed naturally before promotion; no owner process was killed.
Full rollback including the prior save:
`native/build/normal-launch-rollback-before-34e855c1a/`.
Only documentation/evidence commits follow the packaged source. Publish the
handover to both `codex/native-consolidated-20260913` and the preserved shared
baseline `codex/native-reconstitution` by normal fast-forward push.
See `orchestration/recoveries/2026-09-14-crossroads-return/REPORT.md`.

## Previous handover — inventory extensions baseline

Normal entry: `C:\Users\Alex\Documents\Verdigris Native 2026-09-13\Verdigris.exe`.
Installed source: `2651bd4985e698fade744b8aeea5c84f9c251d57` (clean).
Both `codex/native-consolidated-20260913` and `codex/native-reconstitution` now
contain that implementation; subsequent handover commits contain only evidence
and documentation. Shared baseline advanced by fast-forward, without lost work.
Checkout: `C:\Users\Alex\Documents\ChatGPT\verdigris-consolidated-20260913`.

All 80 final packaged scenarios, eight native suites, same-QA-profile Effects
70% save/reload, and the installed normal-entry smoke passed. Actual source,
assets, child paths, working directory, normal profile and clean owned-process
shutdown were checked. Production inventory/drawer/drag captures were inspected,
including 960x600 and 3440x1440. Final sustained drag average 37.140 ms; moving
fullscreen average 25.3 ms, both below the unchanged 40 ms average gate.

Owner save values and quantities were preserved. First normal startup added
compartment/passive-tree schema fields and removed currency's obsolete cell;
the raw save hash therefore changed. Normal settings remained absent/unchanged.
Previous full installation and save retained at
`native/build/normal-launch-rollback-before-2651bd498/`.

Builds and passes no longer depend on Computer Use: use
`native/tools/verify-player-package.ps1` for a package, followed by
`native/tools/test-normal-launch.ps1` for the installed entry. Both run app-owned
hidden diagnostics without desktop input. Computer Use stayed stopped; the
interrupted human walkthrough is not relabeled complete or owner acceptance.
No build/test/push is still running or blocked. Evidence, limits and exact
commands: `orchestration/recoveries/2026-09-14-final-handover/REPORT.md`.

## Historical checkpoints (superseded by the current handover above)

## Combined inventory / m5x7 implementation — final package gate in progress

The owner-selected m5x7 typography history is merged with all six inventory
extensions. Removed inventory controls remain removed. Inventory well pixels
are now cached by dimensions/focus/texture with bounded storage: sustained
3440x1440 dragging of actual weapon art passes at 36.290 ms average (unchanged
40 ms gate), down from 75–84 ms before the repair. Production captures inspected.
Eight native suites and the focused real-server inventory scenario passed.

Unattended launcher save/reload checks passed on the same QA profile; title-only
normal-profile smoke passed in a physical development fixture, including a copy
of the owner's existing save. Actual owner installation/profile remain unchanged
at this checkpoint. Exact clean packaging, package scenarios, installed normal
entry verification and shared baseline promotion are the remaining operations.
Continue them without requesting another approval or restarting Computer Use.
See orchestration/recoveries/2026-09-14-final-handover/REPORT.md.

## Inventory extensions implemented — final combined packaging remains

All six skill-gated left-edge drawers now exist, with server-owned unlocks,
per-Scion storage and real drag transfers. Eight native suites and 80 client
scenarios passed; the enhanced inventory run also verifies populated drawer
transfers/restarts and keyboard reveal/close behavior after the last focused
fixes. Evidence and exact verification limits:
orchestration/recoveries/2026-09-14-inventory-extensions/REPORT.md.

Merge the newer owner-selected m5x7 branch before the next clean package;
preserve its text metrics without restoring the removed inventory action strip.
Sustained drag verification, the exact final package gate, and authorized normal
launch/shared baseline promotion remain. Normal cefd238b2 and owner saves are
untouched by this milestone. The goal remains active; no new approval gate.

## Current verification and installation — unattended checks available

The clean inventory-drag package at native/build/player-package-1d3db8ab7/
Verdigris.exe passed all 80 packaged scenarios, package hashes/resources and two
same-profile launcher lifecycles, including Effects 70% persistence and clean
owned-process shutdown. All eight native suites passed. Computer Use is not
required: run native/tools/verify-player-package.ps1 with explicit package and
contained evidence directories. It retains production captures for inspection.

The normal Documents/Verdigris Native 2026-09-13/Verdigris.exe was independently
updated by the typography task to source cefd238b234c0c489abb2718e6b86ba001672c6c
(m5x7). Preserve that work; the inventory QA package has the earlier font and
must not replace it wholesale. Owner save hash remains unchanged. Six gated
inventory drawers remain unfinished, with a local core-tested storage foundation
preserved outside the package. Shared baseline still d0179c8b3. No Computer Use
or owner-approval gate blocks implementation. Exact evidence and limitations:
orchestration/recoveries/2026-09-14-inventory-drag/REPORT.md.

## 2026-09-14 — Native typography implementation verified

The consolidated client uses the bundled CC0 Verdigris Novel family through
shared roles, crisp integer raster sizes and matching Unicode draw/measurement
paths. Name fields have measured editing/selection/scrolling; the existing log
wraps. The reference font is unidentified and Novel is a finer, more condensed
approximation. Equipment/stat fixes and the normal-entry handover below remain
intact. Full native acceptance passed eight suites and 80 client scenarios; the
final DPI-aware build also passed typography checks. Clean packaging and final
live installation verification follow this milestone. Details and inspected
evidence: `orchestration/recoveries/2026-09-14-typography/REPORT.md`.


## 2026-09-14 - Owner-selected pixel sans typography

Normal installation now contains source cefd238b234c0c489abb2718e6b86ba001672c6c
at C:\Users\Alex\Documents\Verdigris Native 2026-09-13\Verdigris.exe.
All 80 packaged scenarios and 1,270 installed hashes pass; 349 required resources
and embedded clean source identity pass. Existing save bytes and settings are
preserved. Final normal-window inspection awaits the owner's manual launch,
because desktop input was stopped with Escape. Packaged captures were inspected.
Rollback is native/build/typography-sans/rollback-20260913213044/installation.


The owner selected m5x7 at 32px em from actual native captures. Bundled as
Verdigris Sans (CC0), it supersedes Novel and the interim Pixel Operator trials.
Preserve the 2px authored steps and 14px capitals for ordinary text when
integrating concurrent inventory work. Fix layout using actual metrics rather
than changing the selected font to hide clipping. Small numeric counters use 16px em.
The exact Nox font remains unidentified; unsupported glyphs explicitly use '?'.

The typography work is isolated from c00ade08e on
codex/native-typography-sans-20260914, preserving unrelated inventory edits in
verdigris-consolidated-20260913. Existing equipment/stat repairs remain included.
Standing authorization includes committing verified work and pushing the working
branch. Validation, evidence and installation identity are recorded in
orchestration/recoveries/2026-09-14-typography/REPORT.md.

## 2026-09-13 - Normal-launch handover complete

Normal entry: C:\Users\Alex\Documents\Verdigris Native 2026-09-13\Verdigris.exe.
It now launches packaged source 43c104c5acca73805ebb6184079a7fb0d9c7be17 with
its installed assets and normal profile. The previous installation and original
save remain in native/build/normal-launch-rollback-8731c3646/installation.
Every preexisting owner-save field is unchanged; startup adds inventory schema
fields. Normal settings remain untouched. Confirmed menu quit closed the owned
client/server cleanly, with zero remaining Verdigris processes.

The final package passed 79/79 scenarios and eight native suites. Resumed live
checks proved same-profile Effects persistence, moved and equipped loadouts
across separate process restarts without duplicate starter grants, perspective
movement, bounded route thrust feedback, Return to title and Continue. Installed
verification passed all 1266 hashes and 346 required resources. Authoritative
stats/moves, UUID selection, pending/rejection handling and saved loadout fixes
preserve the accepted inventory composition and consolidated renderer history.

Both codex/native-consolidated-20260913 and codex/native-reconstitution carry
the verified implementation; later handover commits change only evidence/docs.
Concurrent uncommitted source edits are preserved and excluded from this package.
Build and verify those changes before replacing the normal installation again.

Evidence and exact limitations: orchestration/recoveries/2026-09-13-handover/REPORT.md.
Explorer double-click automation failed; Windows launch of that same top-level
normal launcher proved its real client/server/profile lifecycle. Bounded live
thrust feedback does not prove enemy damage; packaged melee/animation scenarios
provide separate coverage. Seat-to-backpack drag did not transfer the dagger;
Unequip worked. Back-item factory support and reserved-seat mechanics remain
outside this handover. No new acceptance or planning approval is required.

## 2026-09-13 - Native UI repair verified package

Inventory and character presentation now follow the owner WIZARD composition.
Native capacity/seats, perspective, authored animation and gameplay authority
are preserved. Packaged source f8770d4e7ff493c49ee06122ca2c100f8e9aba26 passed
79/79 packaged scenarios and all eight native test executables passed. The
actual launcher completed button-based House/female Scion creation, movement,
Return to title and Continue. Effects 90% persisted across a fresh process on
the same package and isolated profile. A separately timed live attack-frame
capture remains incomplete after a clean client exit; do not call it passed.

Exact executable and committed before/after evidence:
orchestration/recoveries/2026-09-13-ui-repair/REPORT.md
Normal launch and owner saves remain unchanged. Continue implementation on
codex/native-consolidated-20260913 with the standing commit/push authorization.
Later report/handoff commits do not change the packaged game code.


## 2026-09-13 - Inventory and equipment integration

Continued implementation on codex/native-consolidated-20260913 above 1d732241b,
without waiting for launch promotion. The preserved unfinished checkout's
footprint, seat and combat-field work has been integrated and completed:
12x7 backpack with actual item rectangles; exact seat drops and both rings;
remote U-to-unequip; authoritative acknowledgements and combat ratings; atomic
rejection of incompatible or full-backpack operations. The newer renderer,
animation, movement, melee and progression remain intact.

All native test executables pass, including the death/succession/relic/reconnect
journey. The new inventory-equipment scenario exercises real Win32 handlers
against a native server. Regressions found in the wider scenario suite were
fixed and their scenarios rerun. The exact package gate and evidence are in
orchestration/recoveries/2026-09-13-inventory/REPORT.md.

Owner saves, preserved checkouts and normal launch entries are unchanged.
Separately, the earlier package's fresh-process Effects 90% check passed using
the same QA profile; physical Escape left its other live walkthrough steps
unfinished. That does not block authorized inventory development.


## 2026-09-13 — Packaged verification result

The clean package from `3bd164d34f189d1438dc68990667312e8ecd721f` at
`C:/Users/Alex/Documents/Verdigris Native 2026-09-13/Verdigris.exe` passed all
78 packaged scenarios, resource/hash checks and embedded source identity.
Live title/settings/card screens were viewed; 90% Effects was saved through
the UI. The user stopped desktop control with Escape during live verification.
The full live walkthrough and fresh-process settings reload remain incomplete;
normal launch is unchanged. Exact hashes, evidence, preservation decisions and
remaining acceptance are in
`orchestration/recoveries/2026-09-13-consolidation/REPORT.md`.

## 2026-09-13 — Native application consolidation

Owner-authorized integration in `codex/native-consolidated-20260913`, based on
`5c388d9fd` with recovered history `443fb2503` merged in full. The perspective
renderer, actor assets/attachments, camera and gameplay fixes are preserved.
Title/settings/pause now connect to the existing character cards rather than
the old text ledger. Added names, House selection for creation, keyboard focus,
character management after admission, and returning-player Continue.

Build identity is embedded by both MSVC and CMake and checked against the
package manifest. Normal remote startup validates all 96 authored poses and
logs its actual asset root. Existing checkouts/saves and the normal launch
entry were not changed. Unfinished playable-checkout inventory/equipment work
remains preserved there; its useful build identity work was integrated.

The full development gate (native build, core/network/camera/session/events/
audio/settings tests, all client scenarios, existing performance bounds) passed.
The newly added `consolidated-flow` covers production Win32 button/name input
and real socket admission/reconnect. Final clean-package and live desktop
acceptance are recorded separately in the consolidation REPORT; a development
pass alone is not package acceptance.

## 2026-09-11 — Level, close melee, and starting movement

Owner-authorized narrow fixes in `verdigris-fable-renderer`:

- The remote client now mirrors authoritative level on login, scene admission,
  and state refresh. Partial movement/state updates retain the last valid level.
- Exact combat XP persists per House/Scion, including kills resolved by the
  server timer. Selecting another Scion restores that character's progression;
  a new Scion starts at level 1. Legacy recorded levels migrate at their XP floor.
  Older XP that was never recorded cannot be recovered.
- Remote short melee now uses continuous circular reach (1.25 tiles), a forward
  aiming cone, line of sight, and a 100 ms initial wind-up. Leaving contact
  cancels damage; recovery remains 350 ms. Ordinary enemy contact uses the same
  close boundary. Boss area attacks retain their separate geometry.
- Starting remote movement is four tiles/second (40% below the previous
  baseline); 50 ms input sampling and enemy pursuit speed are preserved. Dash
  distance follows the reduced movement distance. No new movement bonuses yet.

An isolated save earned level 2 through real combat, was reconstructed from
disk, and displayed level 2 in both the live HUD and character panel. The owner
played that review session; its normal exit was followed by review-server cleanup.
Evidence is under `.ci-artifacts/level-contact-20260912/` (UTC date).

Validation: supported MSVC build, core/network/camera/presentation/audio suites,
all 76 client scenarios (one repaired contact fixture rerun), and browser
playtest 32/32 pass. The complete session suite also passes, including ordinary
combat/death/succession, exact heirloom recovery and reconnect. Its test driver
now aims at observed enemies and fights bosses at close range. No art changed.
Larger priorities and the
sprite-generation hold are recorded in [narrow-passes.md](narrow-passes.md).

## 2026-09-10 — Fable renderer and selectable eight-direction Scions

The resumed owner work is implemented in the isolated
`verdigris-fable-renderer` worktree on `codex/fable-renderer-20260910`.
The actual Fable demo archive and recent character-prompt chats were read.
The normal client now uses the reference camera/terrain/lighting pipeline,
bounded asynchronous terrain rebasing, nearest-sampled pixel actors, fresh
mouse aim and responsive movement. Server authority retains collision,
dash, loot, extraction and House/Scion state.

The owner's two character references now supply selectable male/female
appearances: eight directions, two walk contacts and three attack entries
(anticipation/contact plus idle recovery),96 runtime PNGs and96 equipment
sockets. Seven parallel workers handled generation, import and integration;
exact prompts, selected/rejected sources, actual Pixel Respecter provenance
and playback previews are retained. Appearance persists per Scion, survives
restart, and is restored when selecting a saved roster entry. Creation IDs
now avoid persisted living/crypt collisions after server restart. Town return
clears stale upstairs and refreshes stored items.

Final supported build and76/76 client scenarios pass, including fresh-aim
prediction, all96 authored asset selectors and the real female creation/
male successor path. Core/network/session/presentation/audio/camera suites
and browser32/32 also passed. GTX1660SUPER Fable3440×1440 averages28.555ms;
streamed travel averages24.530ms with44.126ms peak, below the unchanged40ms
average gate. Root viewed all equipped poses and native-scale loops, created
and played the female, restarted and played the saved male, and captured a
live attack following a new opposite-facing click.

The normal server/client is left at the1280×800 picker for owner testing;
saved Secondborn(male) and Thirdborn(female) can set out directly. The
animation review is open at `http://127.0.0.1:8873/global-playback.html`.
Relaunch with `native/tools/play-native.ps1`; F11 toggles window mode.
WASD moves, LMB strikes, Space dashes. Owner art acceptance remains pending:
side/diagonal gait consistency, female NW hair variation, idle recovery and
the visual mismatch with older scenery remain visible. This wave does not
claim a manual completed expedition or full Fable art/UI parity.

[Exact lineage evidence and captures](../../native/client/assets/raster/reviews/2026-09-10-lineage/README.md),
[source/import record](hero-lineage-20260910.md), and
[Fable implementation](fable-integration.md).

## 2026-09-12 — Standing commit/push preference

The owner reported recurring friction from blanket push bans. `AGENTS.md`
now makes normal working-branch pushes part of authorized implementation,
unless the user explicitly requests local-only work. PROTOCOL, BUS, the lane
map, old lease rules, the draft governance decision, and the reusable
coordinator prompt now defer to that policy. Preserve it in future sprint
prompts and handoffs. Historical reports are evidence, not new restrictions.

This milestone changes instructions only. Validation: reviewed the complete
diff, checked whitespace and lease JSON, and searched active guidance for
conflicting push restrictions. No gameplay behavior changed or was tested.

## 2026-09-10 — Terrain/wall integration; owner-requested ship and pause

Quiet earth now serves dungeons; the new quiet stone serves crypts. Raster
stone walls use the authoritative blocked grid, actor depth ordering and
player cutaway. No collision/simulation changes. Source generation failures,
published MIT edge processing, explicit wall-plane normalization, selected
native art and production evidence are retained. Catalog: 176 PNGs / 33
manifests / 62 sources / 185 import records; all 174 older PNGs unchanged.

Native core/headless suites passed in the first run, whose client portion
then crashed on a backend-free wall fixture's pickup lookup. The null guard
and explicit fixture assertion resolve it. Final supported build and all 73
client scenarios pass; browser 32/32, importer 10/10 and ground cache pass.
Static/moving 3440x1440 averages: 22.429/24.785 ms; moving peak 34.795 ms,
under unchanged 40 ms average gates. The normal remote game entered town
and dungeon, loaded PNG art and showed 18.9 ms live paint at observation;
Escape exited 0 and launcher/window checks found no owned orphan processes.

Root viewed production captures and the actual live window. Slab repetition,
masonry joins, abrupt cutaway transitions and held-equipment bounds remain
limitations. No manual fight or completed expedition was demonstrated.
The owner explicitly requested wrap-up, sync/ship/push and pause. Ship this
green milestone to the configured origin upstream; do not continue the
broader pixel-world work until resumed. Goal completion is not claimed.
[Review and exact evidence](../../native/client/assets/raster/reviews/2026-09-10-terrain-walls/README.md).

## 2026-09-10 — Firsthand Image 2.5 source audit

Reopened creator prompts and critical replies for combat sheets, walking,
separate-frame edits and game integration. Research now clarifies that PURESO
shows stills rather than walking playback and Noel reports unwanted design
changes rather than necessarily broken anatomy. The prompt workflow retains
Higgsfield's concrete preserve/replace instructions for each reference.
An existing public terrain-seam implementation is linked and explicitly
identified as GPT Image 2/Gemini tooling, not an Image 2.5 result. It has not
been run or adopted here. No verified 2.5-only seamless-terrain recipe emerged.

This is a research/documentation change; production code and all 174 runtime
PNGs are unchanged. The quieter stone experiment remains a candidate after
root viewed its native actor comparison and 3x3 repeat: square repetition and
inconsistent joints remain visible. Wall generation is held at the geometry
proposal. Native/gameplay gates were not rerun for documentation changes.
The broader pixel-world goal remains active.

## 2026-09-10 — Normal native server pursuit and published-reference motion workflow

The earlier pursuit work covered the small local `Simulation`; normal native
`WorldSimulation` enemies were stationary. Ordinary melee now follows a
monotonic server movement clock with 50 ms substeps and a 150 ms catchup cap,
visibility acquisition, bounded leash, deterministic grid navigation and body
separation. Warning/recovery feet stay locked; damage/range/attack timing is
unchanged. Changed `monster:state` endpoints arrive ahead of contact events;
the remote client interpolates display only, rejects stale movement and retains
authoritative facing. Common-rarity parsing and incoming-hit upsert no longer
temporarily enlarge ordinary enemies.

Published Image 2.5 source research now includes PURESO's actual walking prompt,
Higgsfield's per-frame geometry guides, original failures and a distinction
between creator inputs and aggregator reconstructions. Applying separate pose
and identity guides produced SE4 raider walking. Cleaned SW8 wight walking
preserves pale hand pixels that broad background removal erased. Actual Pixel
Respecter imports total 174 PNGs/31 manifests/60 sources/183 import records;
all 162 prior PNGs remain unchanged.

Supported native build/all suites/all72 client scenarios pass. Browser32/32,
importer10/10 and equipment/sampling pass. Static/moving fullscreen averages
22.510/24.684 ms and moving peak33.976 ms pass unchanged40 ms average gates.
Root viewed production SE4, seven live-server SW wight phases, attributed
contact, stop and scene replacement. Missing phase4 is covered statically in
raster-world; no extended/scripted chase was used to force its capture.
The normal live UI completed House/Scion creation and expedition entry, with
F3 at18.3 ms paint and a clean exit. A short D tap showed no visible displacement;
no manual fight or completed interactive loop is claimed.

Remaining: actor identity/gait polish, unanimated directions/actions, broad
remote terrain noise, flat wall placeholders and interactive-session feel.
No push or goal completion is claimed.
[Retained review](../../native/client/assets/raster/reviews/2026-09-10-monster-motion/README.md).

## 2026-09-10 — Real pursuit, directional walks and visible entrances

Native enemies now pursue through bounded circle navigation shared with player
collision. Input batches resolve in order once per 50 ms tick; mouse/polling
bursts cannot advance time. Immutable event poses preserve strike direction,
hit feedback, corpse facing and ordinary drop positions. Local sessions now
mirror world coordinates and actor identity correctly and install/retire scene
collision through entry, extraction and re-entry.

Fourteen new Pixel Respecter outputs add NE6/NW8 raider walking, alongside SW8.
All 148 old PNGs remain unchanged; the catalog has 162 sprites, 29 manifests
and 55 sources. SE remains rejected after three same-leading-leg attempts.
The new production pursuit scenario covers directional phases, collision,
tree detour, contact, input bursts and session lifecycle. Entry construction
clears physical anchors and the initial hero/stair artwork. Visual inspection
rejected a collision-only Tin2 repair, then verified the single-tree move that
exposes the hero and exit. Other props and normal travel occlusion remain.

Final supported build/all 71 scenarios pass. The preceding supported run passes
every native suite; its two remaining main-fixture failures are corrected in
the final run. Browser 32/32, importer 10/10 and equipment/sampling pass.
Fullscreen static/moving averages are 23.283/25.402 ms; moving peak 34.124 ms,
under unchanged 40 ms average gates. Root viewed native and production phase
images, actual detour/contact/entry and the live 3440x1440 window. Both live
local testbed sessions lost the idle character during observation; gear input
and clean closure were verified, but no completed manual fight is claimed.
Harness results prove real fight, loot, equipment and extraction. Opening pace
and full interactive-session acceptance remain open.

Smaller axe/body differences, missing SE and other monster motion, bow/staff
actions, terrain repetition and remote wall presentation remain active work.
Previously recovered items without authored positions retain legacy placement.
No goal completion, push or merge is claimed.

[Evidence, original failures and exact limits](../../native/client/assets/raster/reviews/2026-09-10-pursuit/README.md).

## 2026-09-10 — Pixel contact, dust and retained bodies

Nine new pixel sprites replace geometric dust/swing feedback and add a four-pose SW
raider collapse. The catalog now contains 148 assets across 27 manifests and 52
sources. Each new output exactly matches its reviewed Pixel Respecter candidate;
all 139 previous runtime PNGs remain unchanged.

Ordinary native melee now identifies its attacker before damage. Identified
remote melee/thrust/sweep does the same; ranged and unknown attacks retain their
existing damage feedback. Live ticks age old effects before ingesting contact,
so the first paint keeps the contact pose. Session deaths use a prior actor
snapshot when polling/painting has already removed the living enemy.

Falls last 160 ticks (8 seconds), with four poses over 8 ticks and a final
20-tick fade. Settled bodies draw beneath standing actors. Up to 32 bodies share
the 128-effect cap; transient bursts preserve them. Death never creates a live
actor or a second reward, and scene/loss transitions clear retained bodies.

Final supported build/all 69 scenarios pass. Native core/networking/session/
presentation/audio suites, browser 32/32, importer 10/10 and equipment 60x5x3 plus
sampling pass. Twenty 3440x1440 stationary frames average 25.426 ms; moving
frames average 25.467 ms with 35.561 ms peak. Both average gates remain
40 ms. Root viewed the final supported live window and closed it with exit0.

The fall clip covers only SW raiders. Other death directions/families retain
dust, and deaths without a known prior snapshot cannot fabricate a body. Slash
trails are generic combat feedback, not measured weapon-specific paths. Native
pursuit, other monster motion, bow/staff actions, cross-clip identity, terrain
repetition and remote wall presentation remain unfinished. WarCry, spawn/loss
effects, semantic boundaries, team rings and shadows still use procedural forms.
The broad pixel-world goal remains active.

[Evidence and review limits](../../native/client/assets/raster/reviews/2026-09-10-feedback/README.md). Committed locally; no push or merge.

## 2026-09-10 — Reference-led attacks and readable warnings

The catalog has 139 active pixel assets: NE hero strikes and SW raider strikes
add six poses each. Actual Pixel Respecter palette snapping pins NE colors to
the accepted idle, whose ready pose stays pixel-identical. Measured equipment
sockets pass 60 poses×5 weapons×3 scales. Twelve dropped-item families now select
appropriate existing art; compatible storehut art replaces an existing village
dwelling and Mara's existing town stall without changing objects or collision.

Real elite Sweep events now drive preparation during the warning, phase 3 on
the first confirmed damage frame, and follow-through/recovery afterward. Root
caught and removed an opaque warning fill that hid both actors. Supported final
build/all 68 native scenarios, native suites, browser 32/32, importer 10/10 and
equipment/sampling pass. Fullscreen static average 22.550 ms; moving average
23.993 ms and peak 33.926 ms across 20 frames, unchanged 40 ms gates. Root viewed the
live 3440×1440 window and closed it with exit 0/no orphan.

[Captures, final logs, failures and precise review scope](../../native/client/assets/raster/reviews/2026-09-10-actions/README.md)
are retained. NE body width/reset, bow/staff-specific actions, other monster
directions and locomotion, pixel effects/deaths and terrain repetition remain.
The elite contact proof does not establish ordinary native melee animation,
whose existing damage event lacks attacker identity. This milestone preserves
the active visual objective. Committed locally; no push or merge.

## 2026-09-10 — Quiet ground, registered walking and responsive HUD

The catalog now has 127 active pixel assets, including quieter earth and an
eight-pose SW raider walk that retains its axe. World-aligned paths use existing
landmarks and collision footprints. NW feet and equipment move by exact integer
offsets without repainting; the first actual position update now begins walking.
Route/audio/quickbar chrome shares the existing skin, labels use measured
placement, and equipment stats/XP clear both panes at 960x600.

Final supported native build/all 66 scenarios pass: twenty fullscreen static
frames average 23.258 ms, and twenty moving frames average 24.715 ms with a
36.620 ms peak. The 40 ms average gates are unchanged. Native suites, browser
32/32, importer 7/7 and equipment/ground/HUD probes pass. Root viewed the live
3440x1440 window and final small dual-pane capture, then closed the local client
with exit 0 and no orphan process. [Evidence, clips, traces and original failures](../../native/client/assets/raster/reviews/2026-09-10-ground-hud/README.md)
are retained. The ground probe uses the native build's default compiler flags;
an earlier optimized helper result is explicitly identified as such.

Raider motion is verified by scripted, collision-checked positions through
production presentation, not native pursuit AI. Native enemies still stand
and face/attack. The reference-led NE contact candidate improved the arm, but
the next phase switched limbs and its matte cleanup removed foreground. The
exact source/reference/prompt packages remain outside runtime as bounded review
history. The source-first guide records both stages of that failure.

NE actions, native monster locomotion, other monster directions, identity across
clips and terrain repetition remain open. This closes a tested integration
milestone, not the active visual goal. Committed locally; no push or merge.

## 2026-09-10 — Directional pixel motion and readable contact HUD

The runtime library now has 118 assets: four hero walk directions and six-pose
SE/SW/NW strikes, plus a small pixel impact. Equipment follows measured hands
and changing occlusion; damage tints the actual struck silhouette. Tree
dressing clears the gate, life bars use visible sprite bounds, damage text
clears the bars, and existing detailed orb art retains textured liquid/glass.
Published Image 2.5 workflows are applied with exact prompts and explicit local
adaptations. Original D2 reference pixels stay in the external study cache.

Final native build/all 64 scenarios pass: 18.7 ms fullscreen over 20 frames and
9.4 ms dense 128 effects, unchanged 40 ms limits. Native suites, browser final 32/32,
importer 7/7 and equipment 54 poses x 5 weapons x 3 scales pass. The SW motion fixture
now validates a clear corridor and paints all 15 ms presentation steps; it no
longer mistakes a blocked route/60 ms sampling gap for a missing sprite phase.
One initial browser final-death timeout did not recur in focused/full reruns;
bounded failure evidence was added without changing server behavior or gates.
[Viewed live capture, clips, traces, failures and final logs](../../native/client/assets/raster/reviews/2026-09-10/README.md)
are retained.

This closes the integration milestone, not the visual goal. NW walking feet
float in some frames; clip palette/proportions, NE attacks, enemy weapon/motion
continuity, ground texture/paths and HUD coherence still need work. Pending
NE/raider candidates are preserved outside runtime. No push or merge.

## 2026-09-09 — Game-room feedback on edits and texture

The original room-asset developer comment adds a practical positive result
(more stable repeated edits) and an unresolved weakness (noisy large surfaces).
`RESEARCH.md` preserves its ChatGPT-only access and missing prompt/settings;
`PROMPTING.md` adds scenery-scale review without claiming a proven corrective
prompt. The sorted combat-sheet discussion was reopened to verify the actual
limb, anticipation and timing criticism. Native implementation and candidate
art remain a separate pending milestone; this checkpoint changes guidance only.

## 2026-09-09 — Prompt fidelity, reference roles and reader feedback

Direct browser review of Kiki's original combat prompt/sheet and a reader's
GIF attempt adds user response beyond the creator's own examples. Palette
and registration critiques are recorded as review criteria, without claiming
they measured that output. `PROMPTING.md` now preserves the relevant published
recipe's level of detail and Flixly's previous-frame/opening-frame reference
roles; the universal preference for short prompts is removed. Source-only
generation was paused during this audit. This documentation checkpoint does
not include or validate the pending native motion and asset changes.

## 2026-09-09 — Original Image 2.5 user feedback and reusable recipes

`native/client/assets/raster/RESEARCH.md` rechecks primary creator posts and
critical comments, adds Kiki's combat beats/variable holds, reported 36/99-frame
editing sequences and an exact pose-sheet-to-H3 prompt, and distinguishes still
generation from video motion. Noel's successful example is explicitly scoped
to nearly stationary portrait motion, rather than walking.
`PROMPTING.md` now starts with selection of an actual published example and
records the adaptation before generation. No reliable seamless-terrain or
complete adult isometric-walk recipe was found. This is a documentation
milestone; unfinished native directional motion and strike integration are
separate pending work. The browser harness rerun passes 32/32 and does not
establish acceptance of the current native changes.

## 2026-09-09 — Consistent hero references, readable ground and reliable captures

- Runtime now has 75 assets. Corrected hero idles preserve trousers/wraps and
  equipment sides; finer large props match the hero's apparent pixel scale.
  Packed earth, physical exit stairs and a restrained brazier light improve
  the viewed live scene. Equipment follows measured grips with body/finger
  occlusion. Motion smoothing now returns the actor to idle after stopping.
- Added a real input-tick/presentation-pump motion capture and RGB swatch
  regression. Fixed swapped channels in orb masks, the PNG exporter and its
  old color sampler. Corrected monochrome test targets to display-compatible
  color surfaces, retaining the floor cache and unchanged 40 ms limits.
- Final build/all 64 scenarios pass: full-screen frame budget 18.7 ms and
  dense 128-effect paint 8.6 ms. Native suites, browser 32/32, six importer
  checks and the preserved equipment probes pass. Live 3440x1440 capture
  inspected at original resolution; [review evidence](../../native/client/assets/raster/reviews/2026-09-09/README.md)
  includes actual motion trace and clip.
- Visual goal remains active. SE gait quality, other walking directions,
  legacy attack identity/weapon action, enemy motion, ground transitions and
  scene composition are still unfinished. No push or merge.

## 2026-09-09 — Pixel world runtime and source-backed image workflow

- Added the bounded PNG renderer and actual Pixel Respecter import pipeline;
  73 native RGBA assets now serve player/enemy/NPC, scenery, ground and loot
  presentation. Source images, exact prompts, references, rejected attempts,
  conversion settings and hashes are retained. Static props use visible
  height; animation retains a common canvas/pivot.
- Firsthand Image 2.5 prompts, published outputs and critical feedback changed
  the process: accepted references, distinct reference roles, useful motion
  sheets where they work, separate images when they fail, measured alpha/grid
  cleanup, and a separate motion review. See
  [the asset record](../../native/client/assets/raster/README.md) and its research.
- Existing 62 native scenarios pass (16.6 ms frame budget), the new
  `raster-world` scenario passes separately, and browser playtest passes 32/32.
  Six importer checks and the padded-sprite renderer probe pass. Viewed the
  real 3440x1440 window capture and native/enlarged walk strips.
- This is an integration milestone, not completion of the active visual goal.
  Only SE has four walking candidates; full motion, directional identity,
  weapon attachment, consistent pixel pitch, terrain repetition and world
  marker treatment remain. Continue in the isolated Diablo study worktree.
  No push or merge.

## 2026-09-09 — D2R play reference and coherent native strike contact

- Resolved the D2R screenshot blocker, created offline Barbarian `verdigris`,
  and played camp/Blood Moor combat, death/corpse recovery, healing and gold
  pickup. Reference frames and observation-to-implementation mapping are
  recorded in [the Diablo study](diablo-reference-study/README.md).
- Native presentation now owns swings by actor, reconciles speculation into
  Active contact, and uses one strike for player pose and lunge. Repeated input
  cannot restart preparation; enemy swings cannot pose the player. Core damage,
  cadence and wire format are unchanged.
- Native suites, real-network session checks and all 62 scenarios pass;
  frame budget 27.3 ms at 3440x1440. Browser goal harness passes 32/32.
  Viewed production before/input/contact/recovery frames and a live native
  capture. Broad material, silhouette and actor-overlap defects remain.
- Continues on `codex/diablo-reference-study-20260908` in the isolated Codex
  worktree. No push or merge. See the study REPORT for evidence and limits.

## 2026-09-08 — Owner-directed Diablo reference study and cadence correction

- [Reference kit](diablo-reference-study/README.md): inspected the owner's
  installed D2R 3.2.92777 archives, extracted four gameplay tables and seven UI
  layouts into an external local cache, and retained a reproducible extractor,
  SHA256 manifest, selected observations, and cited D1/D2 architecture research.
- Corrected native remote combat: retriggers and target changes no longer reset
  the existing 350 ms attack deadline. A deterministic regression fails against
  the original code and passes with the fix. The network loot fixture now
  advances the server clock; the build script propagates all test failures.
- [Verification and integration report](diablo-reference-study/REPORT.md):
  browser 32/32; native suites and 61 client scenarios pass; frame budget
  30.7 ms at 3440×1440. D2R live capture failed, so timing/feel/audio observations
  remain explicitly unmeasured. No visual overhaul is claimed.
- Local integration branch: `codex/diablo-reference-study-20260908` in
  `C:/Users/Alex/Documents/ChatGPT/verdigris-diablo-study`. No push or merge.
## 2026-09-12 — Native title, Settings, and return flow (Codex Lane A)

- `native/client/main.cpp` connects the owner launch to a title with House &
  Scion, Settings, and Quit. The existing authoritative Chronicles actions now
  have clickable rows, arrow/Tab focus, Enter/Space confirmation, and Back.
  Escape dismisses gameplay panes before opening the session menu. Return to
  title retains the current session; Continue Scion resumes it without a second
  admission or purse request. Online menus explicitly say the world continues.
- Settings uses Lane B's per-user persistence API for mute and separate effects
  and music volumes. Minus/plus buttons and arrow keys apply and save the real
  audio preferences, including visible load/save errors. Existing WIZARD splash
  texture, Framekit chrome, and skin controls are reused; no art was invented.
- `--scenario frontend-flow` passes 13 checks through a real hidden Win32 input
  target, including mouse, keyboard, controller, quit cancellation, and blocked
  gameplay input. The full 62-scenario run had one resource-envelope timing
  failure under concurrent live/build/session load (69.6 ms); isolated rerun
  passed at 15.1 ms. Frame-budget passed at 26.2 ms / 3440x1440.
- MSVC compilation, denylist, core, networking, camera, presentation-event,
  audio-mixer, and settings tests pass. The first concurrent session suite ended
  with one failed check; the isolated complete rerun passed. Its initial label
  was lost in truncated console output, so its cause is not established.
- Live `play-native.ps1 -Port 6537` used disposable save/preferences paths under
  `native/build/frontend-evidence`, completed title -> House -> Scion -> game ->
  session menu -> title -> Quit, and exited 0 with no orphan process PIDs.
  Captures `title-live.png`, `settings-live.png`, `chronicles-live.png`,
  `house-live.png`, `scion-live.png`, `game-live.png`, `pause-live.png`, and
  `returned-title-live.png` were viewed at 3440x1440. Logs are
  `scenarios-all.txt`, `resource-envelope-isolated.txt`, and
  `session-isolated.txt` in that evidence directory. The clicked Effects minus
  button persisted `sfx=900` in its isolated `settings.ini`.
- Checkout began 15 commits behind origin with an unrelated dirty loop journal;
  the coordinator explicitly directed scoped work without reset/fast-forward.
  No owner-save edits or push were performed. Parent coordinator owns the
  browser playtest and overall integration decision.

## 2026-09-07 — Owner playtest persistence/combat/UI fix pass (Codex)

- Remote skill envelopes now use the authoritative `skillId`; War Cry spends
  resource, grants a bounded attack buff, expires on the native tick, and
  reports its result. Enemy damage/death and player targeting now honor the
  tile-grid line of sight, so attacks cannot pass through walls.
- The native owner HUD uses a compact READY/OFFLINE connection chip, a
  centered XP meter with a visible skill-point plus badge, radial quickbar
  cooldown hands, and a capped combat log for kill/level messages. Clicking an
  NPC now submits its first available interaction directly. The gear pane now
  has a WIZARD-style fourteen-seat paper doll (including conditional slots)
  beside the backpack, populated from authoritative `wearDetails`.
- Native server sessions checkpoint House/Scion identity, chronicle, username,
  and House treasury into an atomic per-guest JSON file beside the server
  binary (or `VERDIGRIS_SAVE_DIR`) and reload it before login, so a server
  restart no longer resets the account shell.
- Evidence: the native build/client scenario gate passes 0 failures, including
  incoming combat hit/death, extraction, remote render-list, paper-doll pane
  readability, frame budget, and persistence journeys. Browser playtest has a
  targeted combat rerun at 1/1; the last full run was 31/32 because the
  healer-race combat scenario timed out once, so a clean full rerun is still
  required before claiming the browser gate.

## 2026-09-06 — Family combat off WASD and Tin village (Cursor)

- VG-SOUND-002 / VG-UI-007: Family combat / Anticipate CC0 parks off WASD,
  the objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Unlicensed still rejected. VG-TOOLS-003 stays Kimi.
  Owner Demo journeys not duplicated. Provenance txt not recaptured.
- Scenario `legal-sounds` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Restore off WASD and Tin village (Cursor)

- VG-GPU-008 / VG-UI-007: Restore / Live buffers 1 parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Leak still rejected. Owner Demo journeys not
  duplicated. Quad BMP and report txt not recaptured.
- Scenario `gpu-recover` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Adapter software off WASD and Tin village (Cursor)

- VG-SOUND-001 / VG-UI-007: Adapter software / Tone 440 Hz parks off WASD,
  the objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. 0 ms cue still rejected. Owner Demo journeys not
  duplicated. Tone report txt not recaptured.
- Scenario `sound-adapter` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Voices 8 off WASD and Tin village (Cursor)

- VG-SOUND-004 / VG-UI-007: Voices 8 / Warning held parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Cosmetic x12 still rejected. Owner Demo journeys not
  duplicated. Does not edit native/audio or VG-PERF-002.
- Scenario `combat-audio` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Pixel capture off WASD and Tin village (Cursor)

- VG-GPU-007 / VG-UI-007: Pixel capture / BMP + provenance parks off WASD,
  the objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Packet log still rejected. Owner Demo journeys not
  duplicated.
- Scenario `gpu-capture` PASS. Capture viewed. Scene provenance txt not
  recaptured. Not Owner Demo. Not TASK-0108.


## 2026-09-06 — Lantern pool off WASD and Tin village (Cursor)

- VG-GPU-006 / VG-UI-007: Lantern pool / Bronze light parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Wash white still rejected. Owner Demo journeys not
  duplicated.
- Scenario `material-light` PASS. Capture viewed. Quad BMP not recaptured.
  Not Owner Demo. Not TASK-0108.


## 2026-09-06 — Y-sort off WASD and Tin village (Cursor)

- VG-GPU-005 / VG-UI-007: Y-sort / Sweep disc parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Wall hide still rejected. VG-WORLD-001 stays Kimi.
  Owner Demo journeys not duplicated.
- Scenario `grounding` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Live packets off WASD and Tin village (Cursor)

- VG-GPU-004 / VG-UI-007: Live packets / Session present parks off WASD,
  the objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Quad demo still rejected. VG-WORLD-001 stays Kimi.
  Owner Demo journeys not duplicated.
- Scenario `gpu-reference` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Theme Combat off WASD and Tin village (Cursor)

- VG-SOUND-008 / VG-UI-007: Theme Combat / Music none parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Leftover loop still rejected. STORY phase stays Kimi.
  Owner Demo journeys not duplicated.
- Scenario `music-phase` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Handle-free off WASD and Tin village (Cursor)

- VG-GPU-002 / VG-UI-007: Handle-free / Telegraph class parks off WASD,
  the objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Backend handle still rejected. VG-CORE-006 stays Kimi.
  Owner Demo journeys not duplicated.
- Scenario `gpu-packets` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Layout v1 off WASD and Tin village (Cursor)

- VG-GPU-003 / VG-UI-007: Layout v1 / No source parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Stale HLSL still rejected. VG-TOOLS-002 cook stays
  Kimi. Owner Demo journeys not duplicated.
- Scenario `shader-bindings` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Type floor off WASD and Tin village (Cursor)

- VG-UI-007: Type floor / Ink contrast parks off WASD, the objective, Tin
  village, and Life. Covering those combat surfaces cannot certify. Shrink
  type still rejected. Owner Demo journeys not duplicated.
- Scenario `hud-scale-floor` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Slay wardens off WASD and Tin village (Cursor)

- VG-GOV-003 / VG-UI-007: Slay wardens / Dash hint parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Walk-on still rejected. STORY copy stays Kimi. Owner
  Demo journeys not duplicated.
- Scenario `first-session-clarity` PASS. Capture viewed. Not Owner Demo.
  Not TASK-0108.


## 2026-09-06 — Software quad off WASD and Tin village (Cursor)

- VG-GPU-001 / VG-UI-007: Software quad / No D3D parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Unknown GPU still rejected. Owner Demo journeys not
  duplicated.
- Scenario `gpu-sample` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Zone loop off WASD and Tin village (Cursor)

- VG-SOUND-005 / VG-UI-007: Zone loop / Loop Tin village wind parks off
  WASD, the objective, Tin village, and Life. Covering those combat
  surfaces cannot certify. ambience x3 still rejected. VG-WORLD-007
  stays Kimi. Owner Demo journeys not duplicated.
- Scenario `ambience-layer` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Strike poses off WASD and Tin village (Cursor)

- VG-ART-003 / VG-UI-007: Strike poses / Windup parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Idle still still rejected. TASK-0173 models stay
  Kimi. Owner Demo journeys not duplicated.
- Scenario `attack-poses` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — Life left off WASD and Tin village (Cursor)

- VG-UI-007: Life left / Mana right parks off WASD, the objective, Tin
  village, and Life. Covering those combat surfaces cannot certify. X on
  mana still rejected. Owner Demo journeys not duplicated.
- Scenario `vital-orbs` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.


## 2026-09-06 — War Cry weave off WASD and Tin village (Cursor)

- VG-ART-006 / VG-UI-007: War Cry weave / Travel parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Screen fill still rejected. TASK-0173 models stay
  Kimi. Owner Demo journeys not duplicated.
- Scenario `weave-vfx` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Village kit off WASD and Tin village (Cursor)

- VG-ART-004 / VG-UI-007: Village kit / Solid proxy parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Lollipop still rejected. Owner Demo journeys not
  duplicated.
- Scenario `kit-chunk` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Warning windows off WASD and Tin village (Cursor)

- VG-ACT-005 / VG-UI-007: Warning windows parks off WASD, the objective,
  Tin village, and Life. Covering those combat surfaces cannot certify.
  ms/50 still rejected. Core ACT stays Kimi. Owner Demo journeys not
  duplicated.
- Scenario `telegraph-spec` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Dodge clear off WASD and Tin village (Cursor)

- VG-ACT-005 / VG-UI-007: Dodge clear / Life holds parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Ghost hit still rejected. Core ACT stays Kimi. Owner
  Demo journeys not duplicated.
- Scenario `telegraph-dodge` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Pad glyphs off WASD and Tin village (Cursor)

- VG-UI-008 / VG-UI-007: Pad glyphs / A strike parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Mouse pad still rejected. Owner Demo journeys not
  duplicated.
- Scenario `pad-path` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Kit lock off WASD and Tin village (Cursor)

- VG-ART-001 / VG-UI-007: Kit lock / Same delta parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Sliding kit still rejected. Owner Demo journeys not
  duplicated.
- Scenario `move-and-camera` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Uniform pan off WASD and Tin village (Cursor)

- VG-ART-001 / VG-UI-007: Uniform pan / Zoom lock parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Free tile still rejected. Owner Demo journeys not
  duplicated.
- Scenario `zoom-invariance` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Adult camera off WASD and Tin village (Cursor)

- VG-ART-001 / VG-UI-007: Adult camera / Bronze palette parks off WASD,
  the objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Chibi head still rejected. Owner Demo journeys not
  duplicated.
- Scenario `visual-target` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Jointed warden off WASD and Tin village (Cursor)

- VG-ART-001 / VG-UI-007: Jointed warden / Snout claws parks off WASD,
  the objective, Tin village, and Life. Covering those combat surfaces
  cannot certify. Crate foe still rejected. Owner Demo journeys not
  duplicated.
- Scenario `first-fight` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Risk wardens off WASD and Tin village (Cursor)

- VG-UI-005 / VG-UI-007: Risk wardens parks off WASD, the objective, the
  production Tin village card, and Life. Covering those combat surfaces
  cannot certify. route:tin still cannot certify. WORLD topology stays
  Kimi. Owner Demo journeys not duplicated.
- Scenario `route-map` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Hit flash off WASD and Tin village (Cursor)

- VG-ART-003 / VG-UI-007: Hit flash / Number fade parks off WASD, the
  objective, Tin village, and Life. Covering those combat surfaces cannot
  certify. Silent hit still cannot certify. TASK-0173 models stay Kimi.
- Scenario `combat-juice` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — World hold off WASD and Tin village (Cursor)

- VG-ART-005 / VG-UI-007: World hold and Unarmed first park below the
  minimap, off WASD, the objective, Tin village, and Life. Covering those
  combat surfaces cannot certify. Paper-doll seat alone still cannot
  pass. ITEM algebra stays Kimi.
- Scenarios `held-item` / `loot-to-bank` PASS. Captures viewed. Not Owner
  Demo. Not TASK-0108.

## 2026-09-06 — Stack 2 between the two panes (Cursor)

- VG-UI-001 / VG-UI-007: Stack 2 is a tall card in the world lane between
  First Scion and gear. Covering either pane, WASD, the objective, or
  Life cannot certify. Escape stack, tree absence, and WASD keep-out stay
  unchanged. Kimi's merge candidate remains a proposal on stale parent
  `193b7c9f`.
- Scenario `pane-stack` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Base Gear off the sheet and combat HUD (Cursor)

- VG-UI-004 / VG-UI-007: the Base Gear review strip parks in the world
  lane right of the C-key sheet. Covering First Scion, WASD, the
  objective, or Life cannot certify. Expanded Conditional-once, compact
  Sources Base | Gear, and dormant ATK exclusion stay unchanged. Core
  STAT algebra stays Kimi.
- Scenarios `stat-explain` / `pack-drag` / `equipment` PASS. Capture
  viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Expanded sheet paints Conditional once (Cursor)

- VG-UI-004 / VG-UI-001 / VG-UI-007: the expanded C-key sheet paints
  Conditional once. Compact Cond plus src cond as a second Conditional
  cannot certify. Compact Sources Base | Gear, dormant ATK exclusion, and
  C or Esc closes stay unchanged. Core STAT algebra stays Kimi.
- Scenario `stat-explain` PASS. Capture viewed. Not Owner Demo. Not
  TASK-0108.

## 2026-09-06 — Pack place off WASD and LIFE (Cursor)

- VG-UI-002 / VG-UI-003 / VG-UI-007: Pack place and Ack only park below
  the minimap, left of the I-key pane. Covering WASD or gear LIFE/ATK
  cannot certify. Compare hint is Enter equips | U unequips; a semicolon
  cannot certify. Pack wrap, reject occupancy, and compare-plate keep-out
  are unchanged. Core inventory-move stays Kimi.
- Scenarios `pack-drag` / `equipment` / `loot-to-bank` PASS. Captures
  viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Pack cells wrap Ember-edged axe (Cursor)

- VG-UI-002 / VG-UI-003: pack cell captions wrap at the type floor so
  Ember-edged axe stays two owner words. A 12-char period clip
  (Ember-edged.) cannot certify. Shrinking type cannot certify overflow.
  Reject occupancy, compare-plate keep-out, and gear footer are unchanged.
  Core inventory-move stays Kimi.
- Scenarios `pack-drag` / `equipment` / `loot-to-bank` PASS. Captures
  viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Compact Sources names Base and Gear (Cursor)

- VG-UI-004 / VG-UI-001 / VG-UI-007: compact C-key Sources paints
  Base 12 | Gear +0. Lowercase base/gear and src jargon cannot certify.
  Pixelmix cannot paint a middle-dot, so the sheet uses ASCII | like the
  gear footer. Expanded Base/Gear rows and the review Base Gear strip are
  unchanged. Core STAT algebra stays Kimi.
- Scenarios `hud-pane-readability` / `pane-stack` / `stat-explain` /
  `loot-to-bank` PASS. Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Compare plate left of gear stats (Cursor)

- VG-UI-003 / VG-UI-002: the Ember-edged compare plate parks in the world
  lane left of the I-key pane. Covering DEF/LVL or I or Esc closes cannot
  certify. Ack-only / pending gold strip is review overlay. TREE absence
  copy is unchanged.
- Scenarios `equipment` / `pack-drag` PASS. Captures viewed. Not Owner
  Demo. Not TASK-0108.

## 2026-09-06 — DEF and LVL stay on the gear pane (Cursor)

- VG-UI-007 / VG-UI-002 / VG-UI-003: gear LIFE/RES wrap above ATK/DEF/LVL
  at the type floor. A one-line readout that clips DEF cannot certify.
  Footer I or Esc closes is unchanged. TREE absence copy is unchanged.
- Scenarios `hud-pane-readability` / `pack-drag` / `equipment` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — I or Esc closes stays on the gear pane (Cursor)

- VG-UI-007 / VG-UI-002 / VG-UI-003: the I-key gear footer is two lines at
  the type floor inside the pane. Compacting copy cannot clip
  Enter equips | I or Esc closes. Shrinking type cannot certify overflow.
  TREE absence copy is unchanged. Character C or Esc closes is unchanged.
- Scenarios `hud-pane-readability` / `pack-drag` / `equipment` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — C or Esc closes stays on the sheet (Cursor)

- VG-UI-007 / VG-UI-001 / VG-UI-004: the C-key close hint is pinned to the
  bottom of the combat-HUD-clamped slot. Compacting rows cannot clip
  C or Esc closes. Covering Life or deleting the hint cannot certify.
  WASD stay off the pane. TREE absence copy is unchanged.
- Scenarios `hud-pane-readability` / `pane-stack` / `stat-explain` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Sheet below map, above Life (Cursor)

- VG-UI-007 / VG-UI-001 / VG-UI-004: the C-key First Scion sheet now sits
  below the minimap and above the life orb. Covering those combat surfaces
  cannot certify. Portrait and stat rows compact inside that slot. WASD
  stay off the pane. Owner Base Gear strip on `stat-explain` is review
  overlay. TREE absence copy is unchanged.
- Scenarios `hud-pane-readability` / `pane-stack` / `stat-explain` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Character keep-out, WASD off sheet (Cursor)

- VG-UI-007 / VG-UI-001 / VG-UI-004: C-key First Scion no longer takes the
  centered WASD fallback. The planner places the hint in the lane right of
  the sheet; deleting it cannot certify. Owner Base Gear strip on
  `stat-explain` is review overlay. Open tree keep-out from the prior
  packet is unchanged.
- Scenarios `hud-pane-readability` / `pane-stack` / `stat-explain` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Tree keep-out, WASD off pane (Cursor)

- VG-UI-007 / VG-UI-001: the top HUD no longer paints WASD on the open
  P-key skill tree. `plan_top_hud` keep-out covers tree and character
  panes, not only gear. Identity and controls land left of the pane.
  Overlaying the tree cannot certify. Gear pairwise captures at
  960/1366/3440 were unchanged. TASK-0193 slice still only paints after
  a payload.
- Scenarios `pane-stack` / `hud-pane-readability` PASS. Captures viewed.
  Not Owner Demo. Not TASK-0108.

## 2026-09-06 — No seats yet, invented origin (Cursor)

- VG-UI-001 / VG-UI-003: the P-key skill tree no longer paints a glowing
  origin seat while it says no data yet. Owner copy is No seats yet;
  invented origin cannot certify. PaneStat TREE string and TASK-0193
  geometric slice are unchanged and only paint after a payload. Stack 2 /
  Escape closes on `pane-stack` is unchanged.
- Scenario `pane-stack` PASS. Capture viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Skill tree, Spawn once (Cursor)

- VG-UI-003: `progression-surface` owner Skill tree strip paints No data yet
  beside the open gear pane. TREE jargon cannot certify absence. PaneStat
  still states `TREE no authoritative data`. Ack only / No pending on
  `equipment` is unchanged. TASK-0156 folder cannot certify. ITEM algebra
  stays Kimi.
- VG-ART-006: `animation-vfx-phase-a` owner Spawn once strip paints Fade ttl.
  Re-spawn cannot certify first sighting. War Cry weave / Travel on
  `weave-vfx` is unchanged. TASK-0122 folder cannot certify. TASK-0173
  models stay Kimi.
- Scenarios `progression-surface` / `animation-vfx-phase-a` PASS. Captures
  viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Slay wardens, Dash hint (Cursor)

- VG-GOV-003: scorecard First session dimension now names
  `first-session-clarity`. Owner strip paints Slay wardens / Dash hint
  below the measured top HUD so Space dash stays visible. Walk-on cannot
  certify local play. Kill fill / Gold pit on `xp-meter` is unchanged.
  STORY copy stays Kimi. Thresholds still need an owner stamp.
- Scenario `first-session-clarity` PASS. Capture viewed.
  Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Kit lock, hud-pane recapture (Cursor)

- VG-ART-001: `move-and-camera` owner Kit lock strip paints Same delta.
  Sliding kit cannot certify pan. Uniform pan / Zoom lock on
  `zoom-invariance` is unchanged.
- VG-UI-007: recaptured `hud-pane-readability` at 960/1366/3440 after later
  HUD chrome. No review strip (pairwise disjoint regions). Life left /
  Type floor strips unchanged. Owner Demo (VG-UI-006) not duplicated.
- Scenarios `move-and-camera` / `hud-pane-readability` PASS. Captures
  viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Dodge clear, Life holds (Cursor)

- VG-ACT-005: `telegraph-dodge` owner Dodge clear strip paints Life holds.
  Ghost hit cannot certify an avoided sweep. Warning windows / ms/50 on
  `telegraph-spec` is unchanged. Core ACT scheduling stays Kimi.
- Scenario `telegraph-dodge` PASS. Capture viewed.
  Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Unarmed first, Uniform pan (Cursor)

- VG-ART-005: `loot-to-bank` owner Unarmed first strip paints World hold.
  Paper doll cannot certify the pickup-to-equip journey. World hold /
  Ack equip on `held-item` is unchanged. ITEM algebra stays Kimi.
- VG-ART-001: `zoom-invariance` owner Uniform pan strip paints Zoom lock.
  A free tile cannot certify the camera contract. Adult camera /
  Bronze palette on `visual-target` and Jointed warden on `first-fight`
  are unchanged.
- Scenarios `loot-to-bank` / `zoom-invariance` PASS. Captures viewed.
  Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Jointed warden, Hit flash (Cursor)

- VG-ART-001: `first-fight` owner Jointed warden strip paints Snout claws.
  Crate foe cannot certify. Adult camera / Bronze palette on
  `visual-target` is unchanged. ART-007 mixed-pack review not this packet.
- VG-ART-003: `combat-juice` owner Hit flash strip paints Number fade.
  Silent hit cannot certify readable contact. Strike poses on
  `attack-poses` is unchanged. TASK-0173 models stay Kimi.
- Scenarios `first-fight` / `combat-juice` PASS. Captures viewed.
  Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Theme Combat unload, Type floor (Cursor)

- VG-SOUND-008: owner Theme Combat strip paints Music none. Leftover loop
  cannot certify. Unload still mutes the music bus. STORY phase stays Kimi.
- VG-UI-007: `hud-scale-floor` owner Type floor strip paints Ink contrast.
  Shrink type cannot certify overflow. Life left / Mana right on
  `vital-orbs` is unchanged. Owner Demo (VG-UI-006) not duplicated.
- Scenarios `music-phase` / `hud-scale-floor` PASS. Captures viewed.
  Not Owner Demo. Not TASK-0108.

## 2026-09-06 — layout v1, soak envelope, named machine (Cursor)

- VG-GPU-003: owner Layout v1 strip paints No source. Stale HLSL cannot
  certify. Software albedo/rim still loads without a runtime path. 64×64
  BMP hash is unchanged. TOOLS-002 stays Kimi.
- VG-PERF-007: owner 32 cycles strip paints Cap holds. Short scene cannot
  certify. Floor bitmaps stay 1 across 32 present/resize/effect cycles.
- VG-PERF-001: owner Named machine strip paints Paint fields. Unnamed HW
  cannot certify. Timed 20-frame 3440×1440 loop still averages under 40 ms
  (32.0 ms this run). Bound not raised. Owner strip paints after timing.
- Scenarios `shader-bindings` / `memory-soak` / `frame-budget` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — ack only, Base Gear, software quad (Cursor)

- VG-UI-003: owner Ack only strip paints No pending. Pending gold cannot
  certify. Compare still follows the acknowledged seat. ITEM algebra
  stays Kimi. TASK-0156 PaneStat string is unchanged.
- VG-UI-004: owner Base Gear strip paints Cond off. Dormant ATK cannot
  certify. Expanded Attack still excludes inactive conditional. Core
  STAT stays Kimi.
- VG-GPU-001: owner Software quad strip paints No D3D. Unknown GPU cannot
  certify. 64×64 BMP hash is unchanged. Not a Windows-only D3D proof.
- Scenarios `equipment` / `stat-explain` / `gpu-sample` PASS. Captures
  viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — adult camera, Tin village, life left (Cursor)

- VG-ART-001: owner Adult camera strip paints Bronze palette. Chibi head
  cannot certify. Live expedition still names camera/proportion/palette/
  contrast; concept-art and loader chips stay rejected.
- VG-UI-005: owner Tin village strip paints Risk wardens. route:tin
  cannot certify. Zoom still cannot leak an off-snapshot blip. WORLD/NET
  stay Kimi.
- VG-UI-007: owner Life left strip paints Mana right. X on mana cannot
  certify. Life stays the left red globe; mute stays a HUD chip. Owner
  Demo (VG-UI-006) not duplicated.
- Scenarios `visual-target` / `route-map` / `vital-orbs` / `first-fight`
  PASS. Shared visual-target PNG recaptured for ART-001 and UI-005.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — focus gear, pack place, pad glyphs (Cursor)

- VG-MOVE-005: owner Focus gear strip paints No buffer. Held fire cannot
  certify. Gear still swallows WASD and combat; closing the pane does not
  fire a buffered attack. TASK-0165 reducer stays presentation-side.
- VG-UI-002: owner Pack place strip paints Reject keeps. Silent equip
  cannot certify. Valid drop still lands at pack:2,1; reject keeps the
  item. Core inventory-move stays Kimi.
- VG-UI-008: owner Pad glyphs strip paints A strike. Mouse pad cannot
  certify. Injected XInput still drives stick, A, Y, and hotplug.
- Scenarios `pane-focus` / `pack-drag` / `pad-path` PASS. Captures viewed.
  Not Owner Demo. Not TASK-0108.

## 2026-09-06 — handle-free packets, kill fill, isolated remaps (Cursor)

- VG-GPU-002: owner Handle-free strip paints Telegraph class. Backend
  handle cannot certify. Semantic packets still copy Telegraph with no
  GPU state. CORE-006 stays Kimi.
- VG-GOV-003: owner Kill fill strip paints Gold pit. VG-ID count cannot
  certify. Empty pit still samples 0 gold; three kills fill ~49%.
  Scorecard thresholds still need an owner stamp.
- VG-MOVE-006: owner Isolated profile strip paints Dash remap. Documents
  cannot certify. Remap/restart/restore stay on a %TEMP% profile.
  SHIP-001 stays Kimi.
- Scenarios `gpu-packets` / `xp-meter` / `remap-binds` PASS. Captures
  viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — mixer prefs, dressing pass, loot labels (Cursor)

- VG-SOUND-006: owner Mixer prefs strip paints SFX persist. Mute reset
  cannot certify. Mute still cannot wipe SFX/music volumes; zero SFX
  stays silent.
- VG-WORLD-008: owner Dressing strip paints Not solid. Tree solid cannot
  certify. Dressing v1/v2 still change decoration hash only. WORLD-001–007
  stay Kimi.
- VG-PERF-005: owner Nearest 12 strip paints Drop stays. Cull pickup
  cannot certify. Dense pouches remain Drop sprites; nameplates cap at 12.
- Scenarios `audio-prefs` / `dressing-pass` / `loot-label-budget` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — reused pens, effect cap, cold trace (Cursor)

- VG-PERF-003: owner Reuse pens strip paints Keep warning. Drop FX cannot
  certify. Second paint still reuses GDI pens; Impact/Swing/Telegraph stay.
- VG-PERF-004: owner Cap 128 strip paints One floor. Grow FX cannot
  certify. Resize cycles keep one floor bitmap; 300 spawned effects stay
  at 128. core.cpp stays Kimi.
- VG-PERF-006: owner Warm glyphs strip paints Cold trace. Hide cold cannot
  certify. Prepared strike is not slower than the reported cold hit.
- Scenarios `effect-batch` / `resource-envelope` / `hitch-warmup` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — live packets, Y-sort Sweep, lantern pool (Cursor)

- VG-GPU-004: owner Live packets strip paints Session present. A quad demo
  cannot certify. Software present of live session packets, not D3D.
- VG-GPU-005: owner Y-sort strip paints Sweep disc. Wall hide cannot
  certify. Sweep stays a readable red disc on the village gate after the
  scenery pass. WORLD-001 stays Kimi.
- VG-GPU-006: owner Lantern pool strip paints Bronze light. Wash white
  cannot certify. Channel cap 220; damage chroma stays visible.
- Scenarios `gpu-reference` / `grounding` / `material-light` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — strike poses and War Cry weave (Cursor)

- VG-ART-003: owner Strike poses strip paints Windup/Active/Recover/Cancel
  silhouettes. Idle still cannot certify. Frame count cannot pass. TASK-0173
  models stay Kimi.
- VG-ART-006: owner War Cry weave strip paints Cast/Travel/Impact/Cancel.
  Screen fill cannot certify. Telegraph remains visible. TASK-0173/0174
  stay Kimi.
- Scenarios `attack-poses` / `weave-vfx` PASS. Captures viewed. Not Owner
  Demo. Does not re-spec TASK-0108.

## 2026-09-06 — bronze family, village kit, world hold (Cursor)

- VG-ART-002: owner Bronze stone strip paints Cooked CC0. Magenta cannot
  certify. Village kit still samples the SPDX CC0 albedo/rim family.
  VG-TOOLS-003 cook pipeline stays Kimi.
- VG-ART-004: owner Village kit strip paints Solid proxy. A lollipop tree
  cannot certify. Live tin village still ships hut, fountain shrine,
  road gate, ruin, and forked trees with published solids.
- VG-ART-005: owner World hold strip paints Ack equip. A paper-doll seat
  cannot certify world appearance. ITEM identity stays Kimi.
- Scenarios `bronze-stone` / `kit-chunk` / `held-item` PASS. Captures
  viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — three slice fixtures and headless contract (Cursor)

- VG-BUILD-001: owner Three slices strip paints Reach pike. Tint clones
  cannot certify. Character sheet still names reach/pressure/magic with
  tactics, weakness, gear, and encounter answers. Core STAT/BUILD stays
  Kimi.
- VG-QA-002: owner Sim event strip paints Intent swing. A mocked
  PresentationEvent cannot prove the journey. Live AttackStarted still
  maps through `presentation_from_sim` to swing intent and
  `attack-anticipate`. native/tests stays Kimi.
- VG-QA-001: traced screenshot hash updated to the recaptured
  `build-fixtures` PNG. Template-only still cannot certify.
- Scenarios `build-fixtures` / `headless-contract` PASS. Captures viewed.
  Not Owner Demo. Not TASK-0108.

## 2026-09-06 — aim hold, present-path latency, uncommitted extract (Cursor)

- VG-MOVE-002: owner Aim hold strip paints Face east. Move facing cannot
  certify. West walk still displaces; east aim stays. Core `resolve_move`
  stays Kimi.
- VG-MOVE-008: owner To present strip paints Input paint. Photon cannot
  certify. p50/p95 stay input-to-present, not dispatch time.
- VG-GOV-006: owner Carry open strip paints No extract. Extract ok cannot
  certify an uncommitted disconnect. Core D-106 stays Kimi.
- Scenarios `aim-hold` / `input-latency` / `death-disconnect` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — pane stack and eight-way (Cursor)

- VG-UI-001: owner Stack 2 strip paints Escape closes. Helper depth
  cannot certify. Escape still dismisses character then gear; bare
  Escape quits. Skill tree title stays Skill tree, not Geometric
  Passives. Capture viewed. Not Owner Demo.
- VG-MOVE-001: owner Eight-way strip paints Up-left. A vertical-only
  encoder cannot pass a diagonal. Capture viewed. Core movement stays
  Kimi.
- Scenarios `pane-stack` / `eight-way` PASS.

## 2026-09-06 — attack beat and mapped cues (Cursor)

- VG-ACT-007: owner Attack beat strip paints Anticipate. A fabricated
  swing cannot mint a beat. AttackStarted/DamageApplied/ActorDied still
  drive `attack-beat:*`; dash during anticipate is cancel.
- VG-SOUND-003: owner Beats mapped strip paints Hit once. Replaying the
  same event ID cannot double-play. Capture is `combat-beats`, not the
  stale shared spawn.
- Scenarios `attack-beat` / `combat-audio` PASS. Captures viewed. Core
  ACT stays Kimi. Does not re-spec TASK-0108. Not Owner Demo.

## 2026-09-06 — encounter mix on the packed fight (Cursor)

- VG-SOUND-007: owner Encounter mix strip paints Hit + warning. An
  isolated preview cannot certify. Mixer tape records hit, kill, and
  scion-lost with peak/floor range. Capture is the packed tin-village
  fight, not a silent schedule.
- Scenario `dense-mix` PASS. Capture viewed. Not WASAPI. Not Owner Demo.

## 2026-09-06 — hide trophies without mutating ground (Cursor)

- VG-ITEM-006: owner Loot filter strip paints Hide trophies. Mutate
  ground is the rejected control. Hiding a nameplate cannot delete the
  Drop sprite or change sim ownership/droprate.
- Scenario `loot-filter` PASS. Capture viewed. Not Owner Demo. ITEM
  sim stays Kimi.

## 2026-09-06 — licensed combat family on the fight (Cursor)

- VG-SOUND-002: owner Family combat strip paints Anticipate CC0. An
  unlicensed preview cannot certify even if the mixer would play it.
  Provenance table stays SPDX CC0 including swing windup
  `attack-anticipate`. The recapture is the tin-village fight, not a
  generic spawn.
- Scenario `legal-sounds` PASS. Capture viewed. Not a WAV bank. Not
  Owner Demo. VG-TOOLS-003 stays Kimi.

## 2026-09-06 — restore keeps one live buffer (Cursor)

- VG-GPU-008: recreate/resize/minimize-restore keep one pixel buffer.
  The restored BMP carries an L-bracket survival mark so it cannot
  certify as `gpu-sample`. Owner Restore strip paints Live buffers 1;
  leak is the rejected control. Failed `0x0` recreate surfaces
  `gpu-error:recreate` and releases pixels.
- Scenario `gpu-recover` PASS. Capture viewed. Hash diverges from
  `gpu-sample-quad.bmp`. Not DXGI device-removed. Not Owner Demo.

## 2026-09-06 — software 440 Hz adapter (Cursor)

- VG-SOUND-001: owner Adapter software strip paints Tone 440 Hz. A 0 ms
  cue cannot certify. Unknown backends cannot pretend to be portable.
  PCM peak report written.
- Scenario `sound-adapter` PASS. Capture viewed. Not Owner Demo. Not
  WASAPI.

## 2026-09-06 — voice budget holds the warning (Cursor)

- VG-SOUND-004: owner Voices 8 strip paints Warning held. Twelve World
  cosmetics cannot starve scion-lost; cosmetic x12 is the rejected
  control. Mixer steal stays TASK-0157.
- Scenario `combat-audio` PASS. Capture viewed. Not Owner Demo.
  VG-PERF-002 stays Kimi.

## 2026-09-06 — painted-scene BMP readback (Cursor)

- VG-GPU-007: tin-village paint writes a 960x600 BMP plus provenance
  (`gdi-scene:tin-village`). A packet log or R/B-swapped PNG cannot
  certify. Pixel capture strip names BMP + provenance.
- Scenario `gpu-capture` PASS. Capture viewed. Hash diverges from
  `vital-orbs`. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — owner zone loop (Cursor)

- VG-SOUND-005: live mixer and Zone loop strip paint Loop Tin village
  wind. A protocol `ambience:route` token or stacked ambience x3 cannot
  certify. Salt reentry still voices one loop.
- Scenario `ambience-layer` PASS. Capture viewed. Hash diverges from
  `audio-prefs` / `music-phase`. Not Owner Demo. VG-WORLD-007 stays Kimi.

## 2026-09-06 — catalog warning windows (Cursor)

- VG-ACT-005: native Warning windows strip paints Thrust 3 ticks · reach
  and Sweep 3 ticks · melee. A protocol HUD token or an ms/50 guess cannot
  certify. `telegraph-spec` diverges from `gpu-packets`.
- Scenario `telegraph-spec` / `gpu-packets` PASS. Capture viewed. Not
  Owner Demo. Does not edit core or re-spec TASK-0108.

## 2026-09-06 — owner audio mixer (Cursor)

- VG-SOUND-006: mute keeps SFX 40 / Music 70 on a skin mixer panel. A mute
  chip without those numbers cannot certify.
- VG-SOUND-008: the same panel paints Theme Combat while foes live;
  unload still forces `music:none` and cannot leave a competing send.
- Scenarios `audio-prefs` / `music-phase` PASS. Captures viewed and
  hashes diverge. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — Sweep telegraph over village scenery (Cursor)

- VG-GPU-005: Sweep paints a readable red disc on the village gate after
  the Y-sorted scenery pass. A HUD token or a capture-black fill cannot
  certify. `grounding` capture diverges from `gpu-packets` / `telegraph-spec`.
- Scenario `grounding` PASS. Capture viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — moving bronze lantern pool (Cursor)

- VG-GPU-006: the village gate now paints a bronze lantern ellipse that
  moves with `light_from_tick`, plus a red damage disc that cannot wash
  white. A HUD token without that pool cannot certify. `material-light`
  and `bronze-stone` captures diverge.
- Scenario `material-light` / `bronze-stone` PASS. Capture viewed. Not
  Owner Demo. Not TASK-0108.

## 2026-09-06 — bronze War Cry weave family (Cursor)

- VG-ART-006: cast motes, travel orbit, impact ticks, and cancel implode
  share bronze identity. Radius stays inside a screen sixth so spectacle
  cannot hide a telegraph. DIB effect rings use `dc_color` so gold is not
  capture-black. Native `War Cry weave` strip paints all four beats.
- Scenario `weave-vfx` PASS. Capture viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — readable strike family at game scale (Cursor)

- VG-ART-003: windup cocks the blade; active lunges it forward. Limb pixel
  mins keep the Scion from collapsing to a stick. `attack-poses` captures
  the armed active strike off the EXIT pad and paints a native Windup /
  Active / Recover / Cancel strip. Idle or HUD labels alone cannot certify.
- Scenarios `attack-poses` / `visual-target` / `held-item` PASS. Capture
  viewed. Not Owner Demo. TASK-0173 animation models stay Kimi.

## 2026-09-06 — skill tree owner language + pack glyphs (Cursor)

- VG-UI-001: tree pane title is `Skill tree`; absence is `Skill tree: no data yet`.
  HUD ops `tree-pane` / `tree:owner-title` / `tree:owner-absent`. Scenario
  `pane-stack` opens the pane and writes `tree-pane-960x600.png`.
- VG-UI-002 / VG-UI-003: pack cells paint a bronze weapon glyph when
  billboard art is missing. A grey square cannot certify. `PaneStat` still
  carries TREE absence for TASK-0156.
- Scenarios `pane-stack` / `equipment` / `pack-drag` / `loot-to-bank` PASS.
  Captures viewed. Not Owner Demo. Not TASK-0108.

## 2026-09-06 — owner-readable character sheet (Cursor)

- VG-UI-004: expanded ATK sources paint Base / Gear / Passive /
  Conditional. HUD ops keep `char:src *`. Slice builds on the sheet are
  `role · gear` chips; tactics/weakness stay on HUD ops for VG-BUILD-001.
- Scenarios `stat-explain` / `build-fixtures` / `loot-to-bank` PASS.
  Capture viewed: current HUD, compact builds, owner source labels.
  Not Owner Demo. STAT/BUILD sim stays Kimi.

## 2026-09-06 — owner gear pane without TREE jargon (Cursor)

- VG-UI-003: gear overlay paints `Skill tree: no data yet` when the tree
  payload is absent. `PaneStat` still carries `TREE no authoritative data`
  for TASK-0156. Weapon seat uses `ui_skin::slot`.
- Scenarios `equipment` / `pack-drag` / `attack-poses` PASS. Capture
  viewed: current HUD, bronze Scion, village gate, owner skill-tree line.
  Not Owner Demo. ITEM sim stays Kimi.

## 2026-09-06 — shrine and gate inside the spawn capture (Cursor)

- VG-ART-004: tin village shrine and dressing gate sit in the spawn
  frustum so `kit-chunk` actually shows them. Fountain has a basin,
  column, and water; the gate has two pillars, a lintel, and an opening.
  A stone blob or a solid slab cannot certify. Spawn stays outside the
  solid shrine radius; the gate remains non-solid.
- Capture viewed: hut lower-left, blue fountain near spawn, gold
  post-and-lintel gate to the right, ruin on the far right. Not Owner Demo.

## 2026-09-06 — tin village ruins as collapsed walls (Cursor)

- VG-ART-004: town `SceneryKind::Ruin` paints a one-sided broken wall,
  fallen timber, and rubble, not a covered wagon. Collision radii and
  dressing-gate non-solidity are unchanged. `wagon` remains in
  `vector_art.hpp` unused by the village kit.
- Scenario `kit-chunk` / `visual-target` PASS. Capture viewed:
  `docs/execution/captures/art-wave/kit-chunk-960x600.png`. Not Owner Demo.

## 2026-09-06 — tin village dwellings as huts (Cursor)

- VG-ART-004: `SceneryKind::Dwelling` paints a mudbrick/thatch hut (walls,
  pitched roof, door), not a scalloped market stall. Collision radii and
  dressing-gate non-solidity are unchanged. `market_stall` remains in
  `vector_art.hpp` unused by the village kit.
- Scenario `kit-chunk` / `visual-target` PASS. Capture viewed:
  `docs/execution/captures/art-wave/kit-chunk-960x600.png` — hut in the
  lower-left, forked trees, bronze Scion, current owner HUD. Not Owner
  Demo.

## 2026-09-06 — jointed bronze wardens (Cursor)

- Town lurker is no longer a hip-to-foot crate: jointed legs, snout, filled
  bronze claws, taller than the Scion. A crate-shaped foe cannot certify
  `visual-target` / `first-fight`. Not VG-ART-007 Owner Demo mixed-pack
  review. ENEMY identity stays Kimi.
- Captures viewed. Scenarios PASS. Not TASK-0108.

## 2026-09-06 — forked village trees (Cursor)

- VG-ART-004: tin village trees are a forked bole with root flare and
  clustered canopy. A circle-on-stick lollipop cannot certify the kit.
  Collision radii and dressing-gate non-solidity are unchanged.
- Scenarios `kit-chunk` / `visual-target` / `held-item` PASS. Captures
  viewed: cloud-like layered canopies, current owner HUD on kit-chunk
  (Tin village, Space dash, no skeleton art chip). Not Owner Demo.

## 2026-09-06 — bronze held weapon on the composition sheet (Cursor)

- VG-ART-001 / VG-ART-005: `visual-target` now pickups and equips before
  present so the sheet is an armed adult Scion, not an unarmed crate.
  Sword is a filled bronze blade with guard; `player_style` is warm
  bronze (not steel grey).
- 32bpp scenario DIBs write GDI RGB; PNG save swaps as BGRA. `dc_color`
  corrects world fills on DIB destinations only. Floor cache is skipped
  on those DIBs so town stone is not blue-grey in the sheet. Live
  `CreateCompatibleBitmap` path is unchanged.
- Scenarios `visual-target` and `held-item` PASS. Captures
  `docs/execution/captures/art-wave/visual-target-960x600.png` and
  `held-item-960x600.png`. Viewed: bronze tunic, bronze blade, tan floor,
  red HP orb. Not TASK-0108, not Owner Demo.

## 2026-09-06 — owner objective strip + dash (Cursor)

- Owner HUD paints `Slay the wardens (1 remain)`, not `objective: ...`.
  HUD ops keep the protocol prefix. Compact controls restore `Space dash`
  (TASK-0153 first-session-clarity). Extract strip drops the `12u` dump.
- Scenario `first-session-clarity` PASS. Capture `visual-target-960x600.png`.

## 2026-09-06 — adult Scion rig (Cursor)

- Vector `humanoid` now uses adult proportions (head ~1/8, jointed legs,
  tapered torso). A 1/3 chibi head cannot pass. VG-ART-001 / VG-ART-003.
- Captures `visual-target-960x600.png` and `attack-poses-960x600.png`.
  TASK-0173 models untouched. Not Owner Demo.

## 2026-09-06 — composition sheet XP + owner risk/return (Cursor)

- `visual-target` seeds three level-1 kill XP so the sheet shows a filled
  meter, not a black hairline. Route card paints `Risk: wardens` and
  `Return: press F at the pad`. HUD op labels stay protocol-stable.
- Captures `visual-target-960x600.png` and `route-map-960x600.png`.

## 2026-09-06 — owner route card names (Cursor)

- Route card paints `Tin village` / `Town road`, not `route:tin:1:0`.
  A protocol colon-id cannot be the owner title. F3 still shows the raw
  id. Compact controls: `WASD | LMB strike | I gear | F3 binds`.
- Scenario `route-map`. Captures `route-map-960x600.png` and
  `visual-target-960x600.png`. Not TASK-0108, not Owner Demo.

## 2026-09-06 — hide skeleton art loader chip (Cursor)

- Owner HUD no longer paints `art: PNG billboards loaded`. Loaded art is
  silent; missing plates still warn. F3 keeps the diagnostic line.
- `first-fight` / `visual-target` reject a loader chip as the composition
  sheet. Mute chip stays. Capture
  `docs/execution/captures/art-wave/visual-target-960x600.png`.
- Not TASK-0108, not Owner Demo.

## 2026-09-06 — local XP meter fill (Cursor)

- Live local HUD showed `XP lv 1` over an empty black strip because
  `sync_world_from_simulation` hard-coded `xp_fraction = 0`.
- Local kill XP now uses the same RS curve as snapshot `state.xp`
  (12 per monster level). Scenario `xp-meter`: empty gold=0, filled
  gold=805 at fraction 0.493. Capture
  `docs/execution/captures/art-wave/xp-meter-960x600.png`.
- Did not touch `remote_session.cpp`, `native/src/core.cpp`, or the
  networking snapshot writer. Not TASK-0108.

## 2026-09-06 — VG-UI-007 pane vs HUD at owner 3440×1440 (Cursor)

- Extends TASK-0159: `hud-pane-readability` now presents 960×600, 1366×768,
  and 3440×1440. Open gear pane stays disjoint from identity, controls,
  objective, art chip, minimap, quickbar, and orbs. Captures write to
  `docs/execution/captures/art-wave/` — a TASK-0159 folder PNG cannot
  certify this wave.
- Viewed open/closed 3440×1440 plus open 960×600. Life red left, mana blue
  right, gear pane on the right, HUD chips clear of the pane.
- Evidence `docs/execution/evidence/VG-UI-007.json`. Not Owner Demo.
  `remote_session.cpp` remains narrow-released for Kimi.

## 2026-09-06 — live HUD window + VG-UI-007 scale/cues (Cursor)

- Presentation gate: launched `verdigris_client.exe`, captured the live
  3440×1440 window with `native/tools/capture-window.ps1`, viewed
  `docs/execution/captures/art-wave/live-hud-owner.png`. Life 100/100 red
  left, mana 50/50 blue right, XP lv 1, skill chips, objective, warden
  grounded. PrintWindow DIB is BGR; the committed PNG is RGB-corrected.
- VG-UI-007: `hud-scale-floor` now writes
  `docs/execution/captures/art-wave/hud-scale-floor-960x600.png`. Scale 0
  rejected; 640×480 still floors type; low life has a chevron; foe tooltip
  contrast is ink-on-panel. Shrinking type cannot pass. Not VG-UI-006 /
  Owner Demo.
- `native/client/remote_session.cpp` narrow-released for Kimi's remaining
  TASK-0108 `world:projectile` parse. `main.cpp` lease stays ACTIVE.

## 2026-09-06 — remaining Cursor-lease SOUND/MOVE/WORLD/ITEM/PERF (Cursor)

- VG-SOUND-006: mute cannot reset SFX/music volumes (`audio-prefs`).
- VG-SOUND-007: mixed-pack mixer tape; owner Encounter mix / Hit + warning;
  isolated preview fails (`dense-mix`).
- VG-MOVE-005: focused panes swallow WASD/combat (`pane-focus`).
- VG-MOVE-006: isolated `bindings.v1`; Documents cannot be the test path
  (`remap-binds`).
- VG-WORLD-008: dressing-pass v1/v2 cannot change topology.
- VG-ITEM-006: loot nameplates; owner Hide trophies; hiding cannot mutate sim ground.
- VG-PERF-001: named Win32 machine + floor/world/hud/upload
  (`frame-budget`, 11.5 ms avg at 3440×1440, bound stays 40 ms).
- VG-PERF-003–007: effect-batch, resource-envelope, loot-label-budget,
  hitch-warmup, memory-soak.

## 2026-09-06 — kit, weave, pad, beats, combat audio (Cursor lease)

- VG-ART-004: tin village kit + collision proxies (`kit-chunk`).
- VG-ART-006: WarCry weave labels; spectacle cannot hide telegraph
  (`weave-vfx`). Does not take TASK-0173 models.
- VG-UI-008: XInput on the fixed tick (`pad-path`). Mouse cannot mint
  `pad:connected`.
- VG-ACT-007: AttackStarted/DamageApplied drive `attack-beat:*`. A
  fabricated swing cannot mint the beat.
- VG-SOUND-003/004/005: `combat-audio` + `ambience-layer`. Same event ID
  cannot double-play; cosmetics cannot starve `scion-lost`; rapid reentry
  cannot stack ambience.

## 2026-09-06 — GPU present path 003–006/008 (Cursor lease)

- VG-GPU-003: `software-albedo-rim-v1` bindings; stale/wrong backend fail
  closed. Scenario `shader-bindings`. Capture
  `docs/execution/captures/art-wave/shader-bindings-quad.bmp`.
- VG-GPU-004: live session packets present; disconnected demo rejected.
  Scenario `gpu-reference`. BMP + PNG
  `docs/execution/captures/art-wave/gpu-reference-*`.
- VG-GPU-005: Y-sort + telegraph overlay after scenery. Scenario
  `grounding`. Capture `docs/execution/captures/art-wave/grounding-960x600.png`.
- VG-GPU-006: moving light, channel cap 220, damage chroma not washed.
  Scenario `material-light`. Quad BMP + HUD PNG.
- VG-GPU-008: recreate/resize/minimize-restore keep one buffer; restored
  BMP stamped; owner Restore strip; `0x0` surfaces `gpu-error:recreate`.
  Scenario `gpu-recover`.

## 2026-09-06 — packets, bronze/stone, legal sounds, graph audit (Cursor)

- VG-GPU-002: Telegraph draw class copies to handle-free packets
  (`gpu-packets`). Poisoned `backend_handle` cannot snapshot. Capture
  `docs/execution/captures/art-wave/gpu-packets-960x600.png`. Snapshot
  `docs/execution/captures/art-wave/gpu-packets-snapshot.txt`.
- VG-ART-002: cooked bronze/stone albedo+rim, SPDX CC0. Magenta fill
  cannot pass. Scenario `bronze-stone`. Capture
  `docs/execution/captures/art-wave/bronze-stone-960x600.png`.
- VG-SOUND-002: combat family includes swing windup `attack-anticipate`.
  Owner Family combat / Anticipate CC0; unlicensed is the rejected control.
  `unlicensed-preview` cannot ship. Scenario `legal-sounds`. Capture
  `docs/execution/captures/art-wave/legal-sounds-960x600.png`.
- VG-GOV-008: pack `roadmap.py validate` (200/689) plus unittest overlap
  fixtures. Decision already at
  `docs/execution/decisions/audit-dependency-and-path-scheduling.md`.

## 2026-09-06 — first-wave P0 + mute-on-unload (Cursor lease)

- VG-GPU-001: isolated software 64×64 bronze/stone quad (`gpu-sample`).
  Capture `docs/execution/captures/art-wave/gpu-sample-quad.bmp`. Unknown
  backend cannot pass. Not a D3D presenter.
- VG-ART-001: in-game HUD names camera/proportion/palette/contrast
  (`visual-target`). External concept-art token cannot substitute.
- VG-SOUND-001: software 440 Hz adapter (`sound-adapter`). Zero-duration
  cue cannot pass as audible.
- VG-SOUND-008: `theme_for` + music-bus mute on `music:none` so unload
  cannot voice a leftover combat loop. Scenario `music-phase`. Capture
  `docs/execution/captures/art-wave/music-phase-960x600.png`. Device stays
  muted in the harness; STORY phase authority stays Kimi.
- VG-ART-003: idle cannot wear the active strike pose (`attack-poses`).
  Capture `docs/execution/captures/art-wave/attack-poses-960x600.png`.
  Does not take TASK-0173 models or re-spec TASK-0108.

## 2026-09-06 — TASK-0108 local Telegraph ingest (Cursor lease)

- Client stage of Kimi's ranged `world:projectile` windup: JS payload keys
  become the existing Telegraph op, then attributed Damage/Impact.
  Helper `native/client/ingest-ranged-projectile-warning.hpp`. Lock in
  `native/tests/presentation_events_tests.cpp`. Scenario `ranged-warning`.
  Does not edit `native/src/**`, `native/include/**`, or
  `native/client/remote_session.cpp`. Slam `monster:telegraph` is not this
  mapper. A hit without a preceding warning cannot mint a Telegraph.

## 2026-09-06 — ship Cursor pack wave (owner asked to push)

Architect checkout `codex/native-reconstitution`. Lands `docs/execution/`
(pack ingest, GOV-001/004 baseline+crosswalk, GOV-002 draft, evidence)
plus the native client/GPU HUD wave. TASK-0108 stays Kimi's core+wire;
client Telegraph ingest stays on this lease. VG-GOV-002 is **not**
owner-stamped. Dual program heads: this branch vs
`origin/codex/goal-aaa-systems` @ `e7b65360`.

## 2026-09-06 — native pane Escape stack (Cursor, uncommitted)

- VG-UI-001: Escape dismisses character then gear; bare Escape quits.
  Helper depth without native paint/Escape cannot prove. Scenario
  `pane-stack`. Capture
  `docs/execution/captures/art-wave/pane-stack-960x600.png`.

## 2026-09-06 — pack-grid drag occupancy (Cursor, uncommitted)

- VG-UI-002: valid pack drop moves the cell; a rejected drop cannot lose,
  duplicate, or silently equip. Scenario `pack-drag`. Capture
  `docs/execution/captures/art-wave/pack-drag-960x600.png`. Sim
  `inventory_move` stays Kimi.

## 2026-09-06 — ack-only equip compare (Cursor, uncommitted)

- VG-UI-003: gear compare plate uses the acknowledged seat. A pending
  request paints `compare:pending`, not gold `currently equipped`.
  Scenario `equipment`. Capture
  `docs/execution/captures/art-wave/equipment-960x600.png`.

## 2026-09-06 — equipped hold on the actor (Cursor, uncommitted)

- VG-ART-005: world `held:*` attachment must follow the acknowledged equip.
  A paper-doll seat with `held:none` cannot pass. Scenario `held-item`.
  Capture `docs/execution/captures/art-wave/held-item-960x600.png`. Does
  not re-spec TASK-0108 or Owner Demo.

## 2026-09-06 — readable ATK sources (Cursor, uncommitted)

- VG-UI-004: character sheet Attack is base+gear+passive only while Cond
  is inactive. `B` expands four source rows. Folding dormant into Attack
  cannot pass. Scenario `stat-explain`. Capture
  `docs/execution/captures/art-wave/stat-explain-960x600.png`. Core STAT
  stays Kimi.

## 2026-09-06 — map/route overlay (Cursor, uncommitted)

- VG-UI-005: minimap zoom/opacity are overlay settings. Scenario `route-map`
  proves max zoom cannot paint `off-snapshot-warden`. Capture
  `docs/execution/captures/art-wave/route-map-960x600.png`. Owner Demo
  journeys not duplicated.

## 2026-09-06 — death/disconnect extract ack (Cursor, uncommitted)

- VG-GOV-006: disconnect cannot silently ack uncommitted extraction.
  HUD `extract:uncommitted` + chip; `extract:ok` only after sim bank.
  Scenario `death-disconnect`. Capture
  `docs/execution/captures/art-wave/death-disconnect-960x600.png`.
  Decision `docs/execution/decisions/rule-on-death-and-disconnect.md`.
  Does not edit `native/src/core.cpp`.

## 2026-09-06 — capture channels + renderer trial (Cursor, uncommitted)

- VG-GPU-007: GDI+ PNG save now swaps DIB B,G,R so COLORREF red/blue
  survive the file. A channel-swapped still cannot certify. Scenario
  `gpu-capture` and recaptured `vital-orbs`. Capture
  `docs/execution/captures/art-wave/vital-orbs-960x600.png` (life 208,69,69
  left; mana 91,146,239 right).
- VG-GOV-005: `docs/execution/decisions/choose-the-renderer-trial-boundary.md`.
  The software sample is the GPU trial; a green quad is not an engine port.
  Extends TASK-0114; does not pick sokol/SDL.

## 2026-09-06 — vital orbs + parity scorecard (Cursor, uncommitted)

- VG-UI-007: life stays the left vessel, mana the right. Mute is a HUD
  chip (`audio muted`), not an X on the mana globe. Scenario `vital-orbs`.
  Swapping the blue sheet crop onto life fails. Capture
  `docs/execution/captures/art-wave/vital-orbs-960x600.png`.
- VG-GOV-003: `docs/execution/decisions/freeze-the-parity-scorecard.md`.
  A feature or VG-ID count cannot pass. Does not mint TASK numbers.

## 2026-09-06 — eight-way move + held aim (Cursor, uncommitted)

- VG-MOVE-001: `encode_eight_way` keeps both axes on diagonals (`up-left`).
  A vertical-only encoder cannot pass. Scenario `eight-way`. Capture
  `docs/execution/captures/art-wave/eight-way-960x600.png`.
- VG-MOVE-002: remote `player:move` no longer overwrites held aim.
  Local tick re-aims after move because core `resolve_move` still turns
  facing. Scenario `aim-hold`. Capture
  `docs/execution/captures/art-wave/aim-hold-960x600.png`. Does not edit
  `native/src/core.cpp`.

## 2026-09-06 — input-to-present latency (Cursor, uncommitted)

- VG-MOVE-008: key/button QPC paired with `paint_scene` present QPC.
  Scenario `input-latency` reports p50/p95 on the named Win32 machine.
  `Simulation::dispatch` elapsed time is not `input-latency:photon`.
  Protocol `docs/execution/decisions/measure-native-input-response.md`.
  Capture `docs/execution/captures/art-wave/input-latency-960x600.png`.
  Report `docs/execution/captures/art-wave/input-latency-report.txt`.
  VG-MOVE-007 buffering stays Kimi.

## 2026-09-06 — headless presentation contract (Cursor, uncommitted)

- VG-QA-002: `AttackStarted` from the simulation maps to `intent:swing` and
  `attack-anticipate`. Removing that bridge fails the fixture. A mocked
  PresentationEvent with swing FX cannot prove the journey. Scenario
  `headless-contract`. Capture
  `docs/execution/captures/art-wave/headless-contract-960x600.png`. Does
  not take `native/tests/**` or mint TASK numbers.

## 2026-09-06 — telegraph timing and geometry (Cursor, uncommitted)

- VG-ACT-005: warning duration and reach come from
  `Simulation::presentation_catalog()`, not `event.value / 50`. Local ticks
  and a remote millisecond payload render the same window. AttackStarted
  cancels; expired entries cannot stay a silent damaging cone. Scenario
  `telegraph-spec`. Capture
  `docs/execution/captures/art-wave/telegraph-spec-960x600.png`. Does not
  edit `native/src/core.cpp`.

## 2026-09-06 — slice build fixtures + evidence schema (Cursor, uncommitted)

- VG-BUILD-001: character sheet names reach (thrust/pike), pressure
  (melee/close blade), and magic (war-cry/vessel). Each lists tactics,
  weakness, gear, and an encounter answer. Three tinted copies of melee
  fail `distinct_slice_loops`. Scenario `build-fixtures`. Capture
  `docs/execution/captures/art-wave/build-fixtures-960x600.png`. Does not
  edit `native/src/core.cpp`.
- VG-QA-001: `docs/execution/pack/tools/evidence_manifest.py` rejects
  template-only records and screenshots without sha256/`produced_by`.
  Does not mint TASK numbers or take `native/tests/**`.

## 2026-09-06 — loot filter facts (Cursor, uncommitted)

- VG-ITEM-006: ground drops publish `loot-fact:weapon|trophy|misc`. Owner
  strip paints Hide trophies; mutate ground cannot certify. Hiding
  trophies suppresses nameplates only; Drop sprites, `loot_positions`, and
  sim ground tables stay put. Scenario `loot-filter`. Capture
  `docs/execution/captures/art-wave/loot-filter-960x600.png`. Does not
  edit `native/src/core.cpp` or item definitions.

## 2026-09-06 — visual dressing vs topology (Cursor, uncommitted)

- VG-WORLD-008: versioned decoration pass on the tin village layout.
  Dressing trees are non-solid (`dressing:tree`). v2 changes the
  decoration hash only; spawn, scenery seed, and topology hash stay put.
  A solid dressing tree is an unreported obstacle. Scenario
  `dressing-pass`. Capture
  `docs/execution/captures/art-wave/dressing-pass-960x600.png`. Does not
  edit `native/src/core.cpp`.

## 2026-09-06 — attack presentation beat (Cursor, uncommitted)

- VG-ACT-007: `ingest_events` maps AttackStarted → anticipate (plus
  `attack-anticipate` cue), DamageApplied → impact, ActorDied →
  aftermath, dash during anticipate → cancel. Owner Attack beat /
  Anticipate; a fabricated swing cannot mint `attack-beat:*`. Scenario
  `attack-beat`. Capture
  `docs/execution/captures/art-wave/attack-beat-960x600.png`. Does not
  edit `native/src/core.cpp` or re-spec TASK-0108.

## 2026-09-06 — remapped controls (Cursor, uncommitted)

- VG-MOVE-006: versioned keyboard bindings persist under
  `%TEMP%\verdigris-isolated-profile`. Duplicate codes paint
  `bind:conflict`; unknown devices paint `bind:invalid-device`. Saving
  into a Documents path fails `bind:owner-profile`. Restart reloads the
  remapped dash; restore defaults returns Space. Scenario `remap-binds`.
  Capture `docs/execution/captures/art-wave/remap-binds-960x600.png`.
  VG-SHIP-001's packager in `native/tools/**` stays Kimi.

## 2026-09-06 — pane focus + 200-ID registry (Cursor, uncommitted)

- VG-MOVE-005: TASK-0165 `input_focus` now gates the production tick. WASD,
  strike/dash, pickup, and pack-drag do not leak through focused panes.
  A held attack cannot fire when the pane closes. Scenario `pane-focus`.
  Capture `docs/execution/captures/art-wave/pane-focus-960x600.png`.
- VG-GOV-004: 200-row registry `docs/execution/CROSSWALK_REGISTRY.md` (no
  TASK mint). VG-GOV-008: pack `roadmap.py` validate + unittest evidence
  in `docs/execution/decisions/audit-dependency-and-path-scheduling.md`.

## 2026-09-06 — dense mix + pane stack (Cursor, uncommitted)

- VG-SOUND-007: score the mixer tape from a mixed pack plus elite telegraph
  and a danger cue. Owner Encounter mix / Hit + warning; isolated preview
  cannot pass. Scenario `dense-mix`. Record
  `docs/execution/captures/art-wave/dense-mix-score.txt`.
- VG-UI-001: native Escape stack — character then gear then quit. Scenario
  `pane-stack` presents the gear pane; a depth helper alone is not the proof.

## 2026-09-06 — sound adapter, prefs, ambience, equip ack, soak (Cursor, uncommitted)

- VG-SOUND-001: software PCM tone adapter; unknown backend fails; shutdown
  releases the buffer. Scenario `sound-adapter`.
- VG-SOUND-006: prefs file keeps SFX/music through mute toggles; zero SFX
  volume drains silence. Scenario `audio-prefs`.
- VG-SOUND-005: one `ambience:<route>` loop; salt reentry cannot stack.
  Scenario `ambience-layer`.
- VG-UI-003: `EquipView` is ack-only. Live HUD `equip:pending:` cannot be
  `equip:ok`. Helmets cannot occupy main-hand. Scenario `equipment`.
- VG-PERF-007: 32 present/resize/effect cycles stay inside the resource
  envelope. A short scene cannot pass. Scenario `memory-soak`.

## 2026-09-06 — material light, pixel capture, GPU recover (Cursor, uncommitted)

- VG-GPU-006: moving light on bronze/stone (`shade_texel_lit`); channels
  cap at 220. Damage-zone chroma cannot be concealed by additive white.
  Live HUD `material-light:moving`. Scenario `material-light`.
- VG-GPU-007: software readback writes a BMP plus provenance
  (backend/content/platform). A semantic packet log cannot count as the
  capture. Scenario `gpu-capture`.
- VG-GPU-008: `RecoverablePresenter` resize/minimize-restore keeps one
  live buffer. Restored BMP carries an L-bracket mark. Failed recreate
  surfaces `gpu-error:recreate` and releases pixels. Scenario `gpu-recover`.

## 2026-09-06 — grounding / telegraph overlay (Cursor, uncommitted)

- VG-GPU-005: contact shadows stay at feet; painter sorts by world Y;
  threat telegraphs paint after scenery so a foreground wall cannot erase
  the warning. Scenario `grounding`.

## 2026-09-06 — GPU reference scene from live packets (Cursor, uncommitted)

- VG-GPU-004: `present_reference_scene` shades the software sample from
  session packets (Player/Monster, scenery, impact, HUD target sheet).
  A disconnected textured-quad demo fails. Scenario `gpu-reference`.
  BMP `docs/execution/captures/art-wave/gpu-reference-session.bmp`.

## 2026-09-06 — visual target + bronze/stone + shader bindings (Cursor, uncommitted)

- VG-ART-001: live HUD names the in-game composition target
  (`target:camera:top-down`, adult proportion, bronze-stone palette,
  ink-on-panel contrast). Concept-art HUD tokens are rejected. Scenario
  `visual-target`. Capture
  `docs/execution/captures/art-wave/visual-target-960x600.png`.
- VG-ART-002: cooked albedo/rim maps in `bronze_stone.hpp` (CC0). Village
  shrine/ruin/gate sample the family; magenta placeholder cannot pass.
  Scenario `bronze-stone`.
- VG-GPU-003: `cook-shaders-and-resource-bindings.hpp` layout v1. Software
  load has no runtime shader path. Stale layout and non-Software backends
  fail instead of drawing a silent fill. Scenario `shader-bindings`.

## 2026-09-06 — GPU sample + semantic packets (Cursor, uncommitted)

- VG-GPU-001: isolated `native/renderer/gpu` software sample draws a
  bronze/stone textured quad, writes a BMP, and shuts down. Unknown
  backends fail. Not a D3D-only window. Scenario `gpu-sample`.
- VG-GPU-002: `packets_from_render_list` copies Telegraph/etc with
  `backend_handle == 0`. Snapshot text has no HDC/D3D/pointer tokens.
  Scenario `gpu-packets`. Live HUD `gpu-backend:software`.

## 2026-09-06 — pad path + legal sounds + music phases (Cursor, uncommitted)

- VG-UI-008: XInput on the 20 Hz tick (injected `PadReport` for harness).
  Glyphs `pad-glyph:*`, hotplug in/out. Mouse coordinates cannot set
  `pad:connected`. Scenario `pad-path`.
- VG-SOUND-002: SPDX CC0 family in `sound_family.hpp`. Owner strip names
  Family combat / Anticipate CC0. Scenario `legal-sounds`.
- VG-SOUND-008: `music:explore|combat|recovery|none|muted`. Coalesced
  submit; unloaded session cannot keep a competing want. Scenario
  `music-phase`.

## 2026-09-06 — village kit + WarCry weave (Cursor, still uncommitted)

- VG-ART-004: tin village kit includes dwelling, shrine, tree, ruin, and a
  non-solid dressing gate. Solid pieces publish `collision-proxy:<kind>`
  on the production render list (same solids as movement). Scenario
  `kit-chunk`. Capture `docs/execution/captures/art-wave/kit-chunk-960x600.png`.
- VG-ART-006: WarCry aura/fade labeled `vfx-weave:cast|travel|impact|cancel`;
  radius capped to 1/6 of the short viewport edge; elite telegraph still
  draws. Extends TASK-0122; does not re-spec TASK-0108. Scenario
  `weave-vfx`. Capture `docs/execution/captures/art-wave/weave-vfx-960x600.png`.

## 2026-09-05 night — melee attack poses (Cursor, still uncommitted)

- VG-ART-003: Scion melee is four rig poses (windup / active / recovery /
  cancel) driven by swing lifetime, cooldown, and dash dust — not a single
  sine of frame count. Scenario `attack-poses`. Does not implement
  TASK-0108 or TASK-0173 model files.

## 2026-09-05 night — combat hitch warmup (Cursor, still uncommitted)

- VG-PERF-006: `warm_combat_glyphs` starts GDI+, Pixelmix, damage fonts,
  and combat pens/brushes, then draws a dummy numeral/ellipse before the
  first player strike. Live local and remote clients call it after
  billboards. Scenario `hitch-warmup` prints cold, warm, and prepared
  paint times; omitting the cold number fails. Swing and Damage ops stay.

## 2026-09-05 night — resource envelope (Cursor, still uncommitted)

- VG-PERF-004: floor cache shrinks when the view is less than half the
  bitmap; effects use `add_effect` with a 128 cap (oldest dropped).
  Scenario `resource-envelope` cycles 1920/640/960 eight times, then
  300 impacts. One floor bitmap; pens/brushes ≤ 128; fx = 128. A cheap
  frame cannot excuse growth.

## 2026-09-05 night — effect batch + tooltip contrast (Cursor, still uncommitted)

- VG-PERF-003: `fill_ellipse` / `ring_ellipse` / `draw_line` reuse cached
  GDI pens and brushes (128 cap). Damage numerals reuse fonts by height.
  Scenario `effect-batch`: 40 impacts + 40 swings still emit ops; a
  thrust telegraph cannot be dropped to pass; second paint reuses pens.
- VG-UI-007: hover tooltip titles and facts paint `kInk` on the panel
  (contrast ≥ 4.5 vs `kPanelMid`); accent is a triangle mark. Extended
  `hud-scale-floor`.

## 2026-09-05 night — loot nameplates + paint trace (Cursor, still uncommitted)

- VG-PERF-005: Z-key loot names are the 12 nearest pouches (X-target
  always included). Every drop still paints as `Drop`. Scenario
  `loot-label-budget` (120 pouches).
- VG-PERF-001: `frame-budget` prints display size, logical CPUs, and
  last-paint floor/world/hud/upload fields. F3 overlay matches. Live
  present times `BitBlt` as upload; headless scenarios report upload 0.0.

## 2026-09-05 night — route card + stat source (Cursor, still uncommitted)

- VG-UI-005: route card under minimap (return/risk, no foe names);
  client-only `[`/`]` zoom. Hidden while gear/character/tree panes own
  the left column. VG-UI-004: character sheet ATK src / Passive / Cond
  dormant. VG-UI-007: life chevron when low. VG-SOUND-006: mute flag
  next to the client exe plus a mute glyph on the resource orb.
- VG-UI-002: backpack drag uses `inventory_grid` occupancy. Valid drop
  moves the cell; rejected drop cannot lose, duplicate, or equip.
  Equip stays Enter / drop-on-weapon / `Command::equip`.
  Evidence: `loot-to-bank` and `hud-pane-readability` PASS (0 failures).
- VG-SOUND-003/004/005: local combat events voice through the mixer;
  duplicate event keys cannot double-play; Scion-lost outranks cosmetics;
  ambience does not stack on the same route. Scenario `combat-audio` PASS.
- VG-UI-007: type floor (`skin::kMinSmallPx` / `kMinBodyPx`); scale 0 is
  rejected; low-life chevron; hover tooltip stays in-frame. Scenario
  `hud-scale-floor`.

## 2026-09-05 — execution pack ingest + native HUD chrome (Cursor)

- Planning pack (200 DRAFT VG goals) lives at `docs/execution/pack/`.
  VG IDs are not TASK numbers. Lanes vs Kimi:
  `orchestration/CURSOR_KIMI_LANES.md`. Crosswalk:
  `docs/execution/CROSSWALK.md`. Baseline HEAD `486058f3`.
- Native HUD: web-token skin, Pixelmix, wizard orb plates, hover
  tooltips, authoritative XP bar (`state.xp` on the snapshot). Cursor
  claims `native/client/**` until released.
- Evidence: `hud-pane-readability` PASS (0 failures) with isolated
  captures under `docs/execution/captures/hud-wave/`.
- Uncommitted; owner pushes. Do not duplicate TASK-0108 / Owner Demo.

## 2026-09-01 — vector art era + four playable themed roads

- vector_art.hpp: procedural animated art replaces the raster world set.
  Humanoid rig (walk/breathe/attack, held tools), lurker/wight/beast/
  ghast/totem monster rigs dispatched by theme+role, swaying trees,
  fountain, stalls, wagon, gate arches, per-theme terrain tiles painted
  into the floor cache, themed masonry walls. Framekit pane chrome and
  item art remain raster (WIZARD deliverables). frame-budget ~10-13 ms.
- Server: per-theme named monster roster (melee/ranged/buffer ids), and
  'theme' rides dev:state.
- Chart pane over open:screen 'chart': town gate tiles open road charts;
  Enter/click sets out via world:zone:enter. Salt/chalk/copper roads and
  the marsh/grove/crypt/wilds themes are reachable in play for the first
  time. Fixed the open:screen parser (payload is top-level, not nested -
  shop/bank panes were silently dead too).
- Live-verified: salt gate -> Rushweir marsh (murk tiles, pools, Mire
  Ghast in elite gold). Owner should feel-check walk/attack animation.

## 2026-08-31 (night) — 55 fps, monsters fight back visibly, first-floor balance

- Perf: floor cache (BitBlt except on tile-boundary crossings), persistent
  back buffer (was a 19 MB alloc/free per frame), cached GDI+ HUD chrome
  (premultiplied layers; orb liquid at 21 levels). Live: paint 21.4 ->
  13.1 ms, fps 43 -> 55 at 3440x1440. F3 shows floor/world/hud section ms.
- Monster body language (presentation-only, event/position-derived):
  telegraph windup lean, landed-strike lunge, mirror toward the player.
- Core balance (owner ruling): pack first strikes arm a staggered
  400-1300 ms windup instead of a same-millisecond burst; contact damage
  2 + level (was 4 + level*2). Journey harness camps for its first hit.

## 2026-08-31 (later) — pacing rework, assets everywhere, audio voiced

- 20 FPS was structural: one 50 ms timer drove simulation AND rendering.
  Now a 15 ms frame timer with a 50 ms fixed-tick accumulator (wire
  cadence preserved), dt-correct camera smoothing, no input-driven
  repaints. Live F3 fps counter; ~30-50 fps at 3440x1440 measured.
- Walls ride the wire (dev:state includeMap, fetched once per scene) and
  draw as raised cut stone. Loot renders as category glyphs; NPCs are
  vector silhouettes with role rings; strike lunge animates the body.
- Asset-path escaping bug had silently disabled the whole WIZARD pack;
  fixed (forward slashes), F3 now reports framekit/item-art/scenery
  state. Town landmarks anchored on server contract positions.
- TASK-0157 audio finally has a device: waveOut synth sink (six-handle
  pool, fail-closed without a device), fed from the remote event stream
  at the fixed tick. M mutes. Owner has not yet confirmed feel/sound.

## 2026-08-31 — perf fix + panes: loot, inventory, character, tree

- Move+attack stutter fixed (`a9944523`): trivial input handlers (the
  WM_MOUSEMOVE per-event sync/invalidate starved lowest-priority
  WM_PAINT/WM_TIMER), viewport-clipped floor tiling, capped predicted
  swing effects. Reproduced as a 198k-event/3s message flood: 164 ms
  frames -> 21.7 ms. `--scenario all` now carries a `frame-budget` gate
  (20 real 32bpp frames at 3440x1440, <40 ms average); F3 shows live
  paint ms.
- `883d642e`: loot draws at authoritative groundItems positions (per-uuid
  fan for same-tile stacks) and X picks up the nearest real uuid (the
  server ignores empty uuids — pickup previously did nothing). The
  vendored WIZARD framekit pack is finally consumed: nine-slice
  panel/slot chrome + item art in the inventory pane (I); new character
  sheet (C) with server-derived attributes; clickable passive-tree pane
  (P) over the authoritative passiveTree mirror (allocation -> 
  player:skilltree:save; verified live, +2 INT round-trip); trade/bank
  panes over open:screen. Pane interiors scale with hud_scale.
- AGENTS.md now carries the binding native presentation gate; agents
  capture the live window with `native/tools/capture-window.ps1`.

## 2026-08-30 — owner-feedback pass 2: presentation leaves the skeleton

- LMB now routes through `dispatch_skill`, so the primary attack draws the
  same instant facing-oriented swing arc as Q/E/R (it previously had no
  animation at all).
- Camera snaps instead of panning the whole map on scene loads (follow lerp
  is unchanged in play; a gap over one arena half-extent snaps).
- The client starts borderless windowed-fullscreen (WS_POPUP at the primary
  monitor size); F11 toggles back to a 1280x800 movable window.
- New `native/client/ui_skin.hpp`: GDI+ skin layer (rounded gradient panels
  with shadows, glass vital orbs, sunken quickbar slots, chips, Segoe/Georgia
  type ramp). All HUD chrome + the Chronicles front door render through it.
- Resolution scaling: `hud_scale(height)` (integer; 1 at the shipped test
  resolutions, 2 at 1440p) sizes the shared HUD geometry, fonts, minimap,
  orbs, quickbar, connection chip; camera zoom grows with window height so
  the world keeps its on-screen scale. Toast anchors above the quickbar.
- All suites green (`native/build.ps1 -RunTests`, `--scenario all`,
  denylist). Verified live at 3440x1440 via window captures.

## 2026-08-22 — shipped for cloud/other harnesses

- Program tip `bb454c3c` on `codex/native-reconstitution` shipped via PR #58.
  Protected `master` is `2d3e92a5`.
- TASK-0101 and TASK-0161 are ACCEPTED/INTEGRATED. Combined native G6 with
  `-CaptureRoot` passed (`COMBINED-EXIT=0`).
- TASK-0108 is READY (readable ranged combat, ports 7280-7299). Exact base
  `76368466`. Do not own `session_tests.cpp` (TASK-0162).
- PC Codex Sol is retired. No OpenCode writer is assumed. Owner launches
  workers on other harnesses from this tip. Standalone orchestration `main`
  remains Mac-owned.

## 2026-08-22 — Cursor successor + TASK-0101/0161 accepted

- Codex Sol retired; Cursor successor acknowledged at `5c62c904`.
- TASK-0101 revision 1 (`a742355d`) ACCEPTED (`34ff3137`) and integrated
  (`bdecf037`).
- TASK-0161 (`9f004d2a`) independently ACCEPTED and integrated (`76368466`).
  Combined program G6 passed on that implementation tip.
- TASK-0108 is READY from W1 with `session_tests.cpp` excluded (TASK-0162).

## 2026-08-21 — PC single-lane Ox Alpha surge runway

- Program truth at sweep start: `d2423873`; `origin/master` and
  `origin/codex/native-reconstitution` matched, latest exact-SHA CI was green,
  and no PR, active claim, REVIEW_REQUESTED, or REVISE transition existed.
- D-126 registers only `ox-pc-a` (Windows, ports 6620-6639). The stopped b/c
  tabs shared one OpenCode project, made no claim/write, and are explicitly not
  Verdigris lanes or incidents.
- `RUN_STATUS.md` now exposes 30 effective pairwise path-disjoint READY packets
  plus 18 DRAFT successors. `PROGRAM_GRAPH.md` carries terminal T1-T8 proof and the deeper journey,
  presentation, renderer, campaign, combat, skill, monster, item, progression,
  persistence, replay, performance, tooling, packaging, and polish graph.
- Initial one-at-a-time route: TASK-0081 Gate B wire-contract freeze. The
  isolated worktree now exists at
  `Z:\Code\.worktrees\verdigris\ox-pc-a` on
  `codex/TASK-0081-gate-b-wire-contract-ox-pc-a` at base `7f271691`. Its local
  ignored `START_HERE_OX_PC_A.md` carries the complete claim/implementation/
  evidence/push/continuation packet; the architect did not claim or write
  STATUS/REPORT.
- Recurring supervision is active through Codex app heartbeat
  `verdigris-surge-supervisor` every 15 minutes. It resumes this architect
  task, scans before action, reviews/integrates/restocks transitions, and
  suppresses unchanged-state noise.
- Owner-only decisions are batched under `orchestration/owner-input/`; none
  blocks TASK-0081. This milestone changes coordination only, not gameplay.

## 2026-08-20 — TASK-0070 reference scenes Stage 1 (cursor, REVIEW_REQUESTED)

- Worker `codex/TASK-0070-reference-scenes-cursor` off `27d2be62`.
  `verdigris_client.exe --reference-scene all` writes 10 PNGs (1920x1080 and
  1366x768) and 5 render-list JSON dumps. Two-run JSON must match.
- Gates: `build.ps1 -RunTests` green. Architect eyeballs one scene per
  resolution.

## 2026-08-20 — TASK-0069 remote reconnect/retry (cursor, REVIEW_REQUESTED)

- Worker `codex/TASK-0069-remote-reconnect-cursor` off `1f45eb33`. Unexpected
  drop enters `Retrying` (1s/2s/4s, three attempts), re-logs the same guest,
  and resumes from the login snapshot. `player:session-replaced` stays
  terminal `Disconnected`.
- Gates: `build.ps1 -RunTests` green (reconnect resume + replaced no-retry).

## 2026-08-20 — TASK-0064 remote presentation unify (cursor, REVIEW_REQUESTED)

- Worker `codex/TASK-0064-remote-presentation-unify-cursor` off program tip
  `5c41a048`. `--remote` uses the local `paint_scene` pipeline (billboards,
  FX, HUD, camera2d); the 0061 debug painter is gone. No Simulation in
  remote mode.
- Gates: `build.ps1 -RunTests -RunClientScenarios` green, including new
  `remote-render-list` (Monster/Swing/Drop via paint_scene) and session
  `render-list` ops. Architect still needs to play `--remote` and rescore
  Gate A (no zeroes, ≥9/12).
- Play: N enters tin route (E is Sweep); X take-underfoot; walk stairs to
  extract. Monster/loot positions are inferred until 0063 snapshots.

## 2026-08-15 (latest) — Orchestration program active

- The program is now coordinated through `orchestration/` (protocol, state,
  decisions, task specs). Claude/Fable is architect+reviewer; the Codex
  coordinator with Luna workers implements. Read
  `orchestration/PROTOCOL.md` first.
- The current coordinator snapshot is indexed in
  [`RECONSTITUTION_STATUS.md`](RECONSTITUTION_STATUS.md), including the
  original checklist, WIZARD seams, review-ready tasks, blocked ownership
  questions, and current gate evidence.
- Focused WIZARD seam verification is recorded in
  [`WIZARD_INTEGRATION_VERIFICATION.md`](WIZARD_INTEGRATION_VERIFICATION.md):
  Orbs, inventory/Brands & Bonds, and Cartographer/map tests pass 73/73.
- Historical Delaford-to-Verdigris coverage is mapped in
  [`DELAFORD_COVERAGE_MATRIX.md`](DELAFORD_COVERAGE_MATRIX.md), with PvP and
  resource skills explicitly deferred pending product authority.
- The checklist gap audit is maintained in
  [`VERDIGRIS_GAP_AUDIT.md`](VERDIGRIS_GAP_AUDIT.md), including evidence,
  parity boundaries, and unresolved owner decisions.
- Wave 1 READY: TASK-0001 (native Legends records), TASK-0002 (build/CI
  hardening), TASK-0003 (slice verification harness). DRAFT: TASK-0004
  (client control pass per decision D-007), TASK-0005 (legacy audit).
- `prototypes/founding-slice/` is a committed, verified browser feel-lab
  ("Founding of a House"): serve the folder statically and open
  `index.html`, or rebuild via `node build.mjs`. It answers camera/combat/
  founding presentation questions and has no architectural authority.

## 2026-08-15 (later) — Milestone E first visual pass and a build fix

- Fixed a real Milestone D defect: `build.ps1` never defined
  `VERDIGRIS_NATIVE_WINDOWS`, so the "windowed" client silently compiled the
  console fallback and exited immediately. The define is now passed, the entry
  point is a standard `main()` (console subsystem keeps `--headless` stdout
  working), and `NOMINMAX` protects the std algorithms.
- `native/client/main.cpp` now carries the Milestone E visual foundation:
  an adjustable 2.5D camera (wheel zoom, PgUp/PgDn pitch, -/= perspective,
  Home reset), back-to-front depth sorting, contact shadows, a projected
  ground grid and extraction pad, billboard actors with enemy life bars,
  client-side loot scatter around kill sites, and procedural event-driven
  effects (swing arcs, impact flashes, death rings, dust, loot sparkles),
  all double-buffered.
- Verified end to end on Windows: MSVC build clean, the eleven core tests and
  the legacy denylist gate pass, `--headless` completes the extraction loop,
  and a driven input pass (PostMessage WASD/LMB/P/E/X against the live window,
  PrintWindow captures) shows the fight, drops, route unlocks, and extraction
  rendering correctly.

## 2026-08-15 — Milestones A–D first runnable slice

- Repository was synchronized to `origin/master` (`882dd81`) and work moved to
  `codex/native-reconstitution`.
- The browser game remains intact as historical reference.
- Product authority, open decisions, WIZARD Arcane Lattice evidence, legacy
  matrix/denylist, canonical `AGENTS.md`, sprint map, and C++ ADR are complete.
- `native/` is a dependency-free C++20 workspace with a fixed-step command/event
  simulation. It proves House and Scion creation, shared player/monster stats,
  movement, melee, damage/death, generated item and trophy drops, pickup/equip,
  extraction, loss and relic candidacy, House-owned route/branch progression,
  and an external seasonal objective/reward.
- `native/client/main.cpp` is a first runnable client: Win32 opens a native
  placeholder-shape window with WASD, mouse actions, Space dash, pickup/equip,
  and extraction controls; `--headless` provides a self-terminating smoke run.
- The core test executable covers the eleven requested architectural behaviors.
- Explicit `platform/`, `renderer/`, `networking/`, `persistence/`, and
  `content/` seams are documented without coupling them into the simulation.
- WIZARD integration intent is now explicit: orbs and Splash feed the renderer
  or menu presentation, Brands & Bonds/inventory feeds the item/UI path, and
  Cartographer is a candidate deterministic map-content adapter.
- The Claude demo source and 22 external PNG plates are now inventoried in
  [`CLAUDE_DEMO_ASSET_INTAKE.md`](CLAUDE_DEMO_ASSET_INTAKE.md). The binaries
  remain outside the checkout pending asset provenance, size, and packaging
  approval.
- The incomplete product checklist is recorded in
  [`VERDIGRIS_FEATURE_CHECKLIST.md`](../product/VERDIGRIS_FEATURE_CHECKLIST.md),
  with economy, Legends, branch-length, travel-risk, and UI-setting questions
  promoted into `OPEN_DECISIONS.md`.

## Next exact steps

1. Run a manual Win32 play pass and capture control/feel notes.
2. Decide whether the next renderer experiment uses the existing Canvas 2.5D
   reference ideas or a focused native billboard layer.
3. Keep networking, persistence, complete magic, and production art out of the
   core until the first playable loop has been evaluated.

## 2026-08-17 — Native parity wave N2 in progress

- TASK-0044 is claimed by Kimi Code in the external worktree
  `C:\Users\Alex\Documents\KimiWork\verdigris`; the WIP adds native world,
  movement, solo-zone, metadata, monster, and stair-return seams without
  changing the unchanged browser harness.
- Coordinator evidence is kept in
  [`TASK-0044 BASELINE.md`](../../orchestration/tasks/TASK-0044-native-protocol-n2/BASELINE.md)
  and the provisional
  [`REPORT.md`](../../orchestration/tasks/TASK-0044-native-protocol-n2/REPORT.md).
- The current worker WIP now passes the native denylist, core tests, and
  networking tests. A rebuilt server also passes unchanged `movement` and
  `zones` attach scenarios 2/2, including all six zones and saved-position
  restoration. Coordinator captures are preserved in the TASK-0044 folder;
  Kimi committed the six native files as `d476788`, so the task is now
  `REVIEW_REQUESTED` pending Fable's architect rerun and acceptance decision.

## 2026-08-17 — N3 combat parity boundary prepared

- The coordinator completed a read-only audit of the native/browser combat
  seams and recorded the executable handoff in
  [`N3_PARITY_IMPLEMENTATION_BRIEF.md`](N3_PARITY_IMPLEMENTATION_BRIEF.md).
  It maps the existing `player:move` and `player:skill:trigger` wire events
  into the deterministic core and lists the unchanged `combat` and
  `encounter-variety` acceptance matrix.
- The committed N2 server intentionally fails those two scenarios at the
  N2 boundary (18 monsters instead of the combat scenario's minimum 20, and
  no authored melee/ranged/buffer roles). The raw negative transcript is
  preserved in the TASK-0044 captures; no assertion was weakened and no N2
  source was changed.
- This is a handoff and evidence checkpoint, not an N3 claim. Native combat
  implementation must wait for Fable to issue a READY N3 task/spec.
- The requested authority choice and task issuance are tracked in
  [`QUESTION-0009`](../../orchestration/questions/QUESTION-0009-native-n3-authority-bridge.md);
  no source workaround is authorized while it remains open.

## 2026-08-17 — Current coordinator evidence refresh

- The disposable combined parity candidate `codex/integration-parity-candidate-v3`
  (`3636b729`) applies the complete TASK-0043 correction chain and TASK-0044
  `d476788` directly onto coordinator tip `9fea5668`.
- That candidate passes browser unit `122/779`, browser playtest `31/31`, the
  native denylist/core/networking/client gate, and the unchanged native attach
  matrix `quickstart`, `single-session`, `movement`, `zones` at `4/4`.
- TASK-0043 is now accepted and integrated at `1f470e3` on program tip
  `50ca60ad`; Fable's architect review `1f833081` records the default-mode
  31/31 gate and loopback-bind guidance. TASK-0044 is now accepted and
  integrated at `5b84f51e` on program tip `71b6b207`; review `5b2ee5b6`
  records the personal 4/4 rerun and accepts the documented N3 stubs.
- A dependency-complete rerun of that exact staged timing correction was
  recorded at coordinator commit `9e5d9fd8`: a fresh worktree installed
  dependencies with normal install scripts, then passed browser unit `122/779`
  and `PLAYTEST_PORT=6538 npm run playtest` at `31/31` (`loadMode:false`,
  p99/max event-loop lag `32.178/109.642 ms`). This strengthens the evidence
  package and is retained as coordinator provenance for the accepted
  correction.
- The WIZARD seam rerun is recorded at coordinator commit `6cb7b366`:
  Orbs, Brands & Bonds/inventory, and Cartographer/map tests remain `73/73`;
  Verdigris Splash remains intentionally presentation/reference-only.

## 2026-08-17 — Current-tip N2 candidate refresh

- The pre-integration coordinator tip `27db1611` is intentionally still N1:
  its unchanged native attach baseline is 2/4 (`quickstart` and
  `single-session` pass; `movement` and `zones` time out).
- Disposable candidate `f602dab4` cherry-picks N2 worker commit `d476788`
  onto that tip and passes the native denylist/core/networking/client gate plus
  unchanged `quickstart`, `single-session`, `movement`, and `zones` at 4/4.
- This supersedes the earlier candidate reference for handoff purposes but
  accepted implementation and loopback-bind correction are integrated.

## 2026-08-18 — N3 combat parity review handoff

TASK-0045 is `REVIEW_REQUESTED` on
`codex/TASK-0045-native-protocol-n3` at `6d39565c`. The worker owns only
`native/**`; `playtest/**`, `server/**`, and `src/**` remain read-only. Native
denylist/core/networking gates, unchanged seven-scenario attach, combat unit
coverage, and the authentic telegraph-radius negative are captured. The
architect must rebuild and rerun the attach before acceptance; no N3 source is
integrated into the program branch yet.

## 2026-08-18 — Current-master playability evaluation review handoff

TASK-0046 is `REVIEW_REQUESTED` on
`codex/TASK-0046-playability-reevaluation` at `1de6e45b`, based on current
program tip `45846af7`. It owns only task-folder evidence. The report records
two approximately ten-minute arcs, first-minute page-context `window.ws.url`
proofs on disposable ports, and a new disposition/ranking. Guest produces
readable melee kills/XP/gold; the mortal-oath Chronicles arc remains blocked
at a visually present but mechanically silent opener. Architect review is
pending.

## 2026-08-20 — Server/rules parity COMPLETE (32/32 attach)

D-122 axis 1 is done: the unchanged 32-scenario playtest harness passes
against the native C++ server, verified twice consecutively on fresh
servers (PR #45, hotfix PR #46). Native gates (build + unit + session +
client scenarios) all green; session tests survive 8/8 under heavy CPU
load after the reader-thread join fix.

Load-bearing findings for successors:

- Target selection: `start_player_attack` now does a true nearest scan
  with direction-aware tie-breaks. The old scan silently locked onto
  the first spawned monster; anything combat-adjacent that "worked"
  before 08-20 may have depended on that bug.
- Session semantics mirror JS exactly (proven by probing the live JS
  server): one shared anonymous guest account; concurrent anonymous
  login REPLACES the old session; adoption rebuilds a fresh Player
  while loot/levels/bank/tree/quest-record persist; permadeath
  survives reconnect; the commission chain resets on scion admission
  only. Two quest-point counters exist on purpose: quests.questPoints
  (persistent chain record) vs top-level questPoints (live tree
  budget, resets per login).
- WebSocketServer::stop() now joins per-connection reader threads.
  Never revert to detach: a reader waking after `delete server`
  dereferences the freed object (the 08-20 Native CI segfault).
- Ops: kill servers with PowerShell Stop-Process, never Git Bash
  pkill (MSYS pkill cannot kill native Windows exes; a stale server
  produced a bogus 26/32).

Remaining axes: presentation deltas #3/#4 (surface density TASK-0078,
panels/typography unspecced), Gate B Chronicles client (TASK-0077).


## 2026-09-14 — first-slice art milestone (Codex)

Branch `codex/first-slice-art-20260914`, isolated worktree. Published art scope: 128 reviewed player locomotion PNGs (male/female, walk/sprint, four cardinal directions, eight phases). Fixed 96x96 canvas, [48,80] rig anchor, 48px/metre. Undyed linen without trim; shorter practical female cut; female braid centered behind the head. Includes editable Blender sources, native references, original generated alpha PNGs and hash-checked reconstruction reports under `native/client/assets/first-slice/reviewed`.

This is NOT completion of the owner request for all village-defense prologue assets. Native game integration, idle/combat/equipment, NPCs, monsters and scenery remain unfinished. Rejected local generations are excluded from the versioned reviewed pack. Do not use old candidate files when their source hash no longer matches the report. The experimental attack renderer still has hand-pose and skin-mask defects and is not approved for painting or runtime.

Validation: package validator loaded all128 PNGs and checked RGBA/binary alpha,96x96 size, unique phases, source/reference/output hashes, fixed anchor, whole-cycle registration and per-frame silhouette gates. Both full64-frame contact sheets were visually inspected at2x; no gameplay change is claimed. Native baseline build/tests and fable-world scenario passed before asset work; no client source or game behavior was changed by this milestone. Standing owner policy permits committing and normal pushing verified implementation; do not add a blanket push ban.


### Starter exterior reference update

The user supplied and endorsed `native/client/assets/first-slice/concept/starter-player-exterior.png`. This exact image is now the appearance reference. It supersedes earlier short-hair, mandatory centered-back-braid, thin-belt and open-sandal-only assumptions. Preserve its long naturally draped female braid, male wavy hair and jaw beard, coarse untrimmed flax, substantial rope belts, asymmetric sporty female tunic, woven footwear and low wooden-club grips. The 128-frame milestone predates this concept and is not a completed match. No Blender or generated sprite change is claimed by this reference-only update.

## 2026-09-15 — first-slice nonplayer art milestone (Codex)

On `codex/first-slice-art-20260914`, the user requested parallel production of the complete Village Palisade first-slice asset pack. Accepted nonplayer output: 16 static NPC cardinal views, eight scenery sprites, four starter inventory icons, and 64 pack-wolf idle/walk frames. Self-contained accepted folders under `native/client/assets/first-slice/` preserve source/reference/output hashes, prompts, reconstruction reports, and editable Blender sources where applicable. Root independently viewed the final contacts and targeted corrections. World pixel density remains 48px/metre with declared pivots; inventory icons are not world-scale sprites.

The current starter exterior reference remains authoritative. New player and monster combat/death/boss art is still in production; partial animation families must not replace complete gameplay art or silently fill missing actions with idle/old sheets. Native integration and ground pixel-parity fixes are concurrent work and are not certified by this art-only milestone. A pre-change native fable-world timing run measured41.681ms versus the40ms bound while concurrent renders were running; full final native acceptance remains required without relaxing that limit.

Nine prior ChatGPT generation conversations were moved into the existing Pixel Art and Game Dev project and verified in its listing. Any future web generation must stay in that project. The user's latest instruction switches new generation to the built-in imagegen skill while web requests are rate-limited. Inspect decoded alpha and composites: RGB stored beneath alpha0 is not a visible halo and must not trigger needless regeneration.

## 2026-09-15 — native art integration milestone (Codex)

The native client now reads explicit accepted clip manifests with ordered frames, cardinal directions, per-clip canvas and ground pivots at 48 pixels per world tile. Source PNG bytes are installed unchanged. Gameplay actor activation requires every required action and direction; players additionally require both unarmed and club families for the same appearance. Imports replace the whole appearance cohort rather than retaining old actions or equipment variants. Different action padding is legitimate (96x96/[48,80] to 128x128/[64,96]) and does not resize the character. Static NPC identities are explicit scene bindings. Accepted inventory icons fit their actual item footprints independently of world sprite placement.

Terrain now samples 48 logical texels per tile on a rectangular 3840x3072 patch, preserving the existing 80x64 world extent, camera and asynchronous streaming. Point sampling avoids a second implied pixel grid. Resident-plus-bake CPU memory is approximately 98 MiB. Pre-rendered ground footprints use bounded depth bias to keep below-pivot bases visible without moving the sprite rectangle or UVs.

Validation: native build/tests passed, installer cohort-replacement tests passed, legacy denylist passed, and all 83 native client scenarios passed with exit 0 after final source changes. The first-slice scenario checks actual GPU art traces, anchor preservation, terrain overlap, incomplete-family gating and a below-pivot depth regression. Evidence is under `native/build/first-slice-integration`, with suite log `native/build/first-slice-all-scenarios.log`. On GTX 1660 SUPER at 3440x1440, the final suite measured 26.382 ms average native frames and 24.448 ms average streamed frames (50.078 ms peak), 1214.154 ms background bake, 7.417 ms upload, and 102559336 peak CPU terrain bytes. The existing 40 ms average gate was not relaxed. Fresh native hut, well, NPC and inventory captures were independently viewed by the root agent; the live-window/package gate is coordinated by root separately.

This is an integration milestone, not a complete first-slice deliverable: the local inspected runtime currently contains 92 accepted nonplayer PNGs, while new player and monster combat/boss families remain in production. Incomplete animated families stay out of gameplay. The inspection fixture uses the production renderer but does not implement the authoritative Village Palisade story or assign old Crossroads NPC names to prologue roles. Do not mistake the retained legacy player in these captures for the new approved exterior.

### Local launcher renderer correction

Root's live-window review caught a startup seam the fixtures missed: `play-native.ps1 -Local` left perspective and authored-actor rendering disabled, unlike remote startup, and therefore used the old GDI art path. Both launch modes now construct their state through the same product-renderer initializer. A targeted first-slice regression checks this startup configuration and an accepted tree draw from normally generated scenery without fixture bindings; it passes. The real local launcher was then driven through Enter and captured using `capture-window.ps1`; `native/build/first-slice-integration/live-native-game-routing-fixed.png` was viewed and confirms accepted trees/huts and the pixel-parity terrain. The launcher exited cleanly without orphan processes. Distinct storehut, gateway, shrine and column assets remain legacy where no accepted replacement exists. The local testbed's idle combat death is unchanged and is not an art-routing failure.

### 2026-09-15 — approved exterior walk milestone and live-path correction

Added 24 inspected frames under `native/client/assets/first-slice/starter-v2/accepted`: male front/back walk and female front walk, eight phases each. Exact imagegen calls, original RGBA, uploaded guides, native references, editable Blender scenes and source/reference/output hashes are preserved. These are partial clips; gameplay activation remains blocked until coherent unarmed and club families have all required actions/directions. Native canvas96x96, pivot48,80 and48px/metre remain unchanged. Recorded sheet/row registration does not recenter individual poses or discard foreground. Registration and final silhouette alpha cutoffs are now explicitly distinguished in reports. Five durable exterior/reconstruction scripts syntax-compile; packaging verifies copied references and hashes.

Root launched `native/tools/play-native.ps1 -Local`, captured and viewed the3440x1440 live title and game (`native/build/first-slice-integration/live-native-{window,game}.png`), then closed only that test process; launcher verified no orphan process. This caught a real integration gap: local startup used the old GDI renderer while remote startup and fixtures used the new product renderer. The native lane is correcting shared startup configuration; the old-scene capture is failure evidence, not visual acceptance. The83-scenario code milestone `ed5c062` passed but did not catch that startup-path difference. Re-run the real launcher after its correction.

### 2026-09-15 — boss locomotion and native runtime asset packaging

The accepted nonplayer collection now has156 sprites:128 pack-wolf/well-alpha idle/walk frames,16 NPC views,8 scenery sprites and4 inventory icons. The boss's initial softer right-walk paint was rejected and replaced to match the other directional contours. Root and world lane inspected revised outputs; the self-contained monster pack includes original generation sheets, exact guides/prompts, editable Blender sources, native refs and reconstruction/hash reports. Combat families remain incomplete and inactive in gameplay.

`native/client/assets/first-slice/runtime/manifest.tsv` and its156 PNGs are now versioned, with the scenery/monster import JSONs in runtime-imports. Every runtime PNG was independently checked byte-for-byte against accepted sources, including binary alpha and declared frame geometry; no stale/unreferenced runtime PNGs are present. Monster generation/package scripts syntax-compile. The prior92-sprite runtime passed all83 native scenarios; this addition installs the64 reviewed boss locomotion frames without enabling incomplete gameplay families.

Live startup correction5ead862 is verified: root viewed `native/build/first-slice-integration/live-native-game-routing-fixed.png`, showing accepted trees and generic hut through the production renderer. Distinct old landmarks and old actor families are still visible where no complete corresponding replacement exists; this is not completion of the village-defense gameplay or all first-slice animation coverage.

### Retained monster death routing

Death events now retain the exact monster art identity and committed facing after live snapshots remove the actor. Fable resolves that complete family's death clip, including its action-specific canvas/pivot, advances by clip FPS, and holds the last frame through corpse retention/fade. It suppresses duplicate live-dead drawing and never substitutes the old raider death for an active complete new family. Existing incomplete-family gating and gameplay timings are unchanged.

Full native build/tests, the first-slice-art scenario, and the existing raster-feedback lifecycle scenario pass. The focused test drives a real lethal hit through LocalCoreSession, removes the actor in a snapshot before draining its death event, and checks same-family GPU routing across death phases and final-frame hold. It temporarily reuses accepted wolf pixels as diagnostic clips and restores the registry; those pixels are not death-art acceptance or runtime promotion. Evidence logs: `native/build/first-slice-lifecycle-build.log`, `first-slice-lifecycle-scenario.log`, and `first-slice-legacy-lifecycle-scenario.log`.

Forthcoming 16-phase attack art must place authoritative contact at index8 (normalized phase0.5). Existing melee presentation lasts six50ms ticks and sweep eight; predicted input begins at phase0 and contact reconciles to phase0.5. Attack manifest FPS does not stretch gameplay timing. See `native/tools/README-first-slice-art.md` for the import and cadence contract.

### Player death equipment retention

Core death clears carried items before the next render, so deriving the dead player's art from current inventory selected unarmed instead of the equipped club. World synchronization now retains only the last living visual appearance, held variant and facing. Fable uses that snapshot during the same player's death while keeping complete-family gates. LocalCoreSession also retires the scene to `surface` in the same death snapshot; this single alive-to-dead transition preserves the visual identity, while later scene changes or a successor clear it and the death clock. Lost gameplay items remain lost.

The client recompiles and first-slice-art, death-disconnect and raster-feedback scenarios pass. The focused regression uses actual core equip, enemy lethal damage and inventory clearing through both direct local and session adapters, checks GPU selection of the club death rather than unarmed, and creates a successor to verify reset. Its temporary diagnostic pixel mappings are restored and never exported as player artwork. Evidence: `native/build/first-slice-player-death-scenario.log`, `first-slice-player-death-disconnect.log`, and `first-slice-player-death-feedback.log`.

### 2026-09-15 — keep art studies out of player packages

Package creation previously copied the entire first-slice directory recursively, including ignored failed generations and editable source scenes. It now copies only runtime/manifest.tsv and its referenced PNGs. The shared resource validator checks frame-name/content hash agreement and rejects missing/empty manifests; package validation rejects any additional first-slice source or stale PNG. Artist sources remain versioned separately for editing and provenance. PowerShell parse checks pass, the live156-PNG inventory selects157 files, full required native resources validate (518), and empty/path-escape negative probes fail as expected. A complete clean-commit player package build remains pending final actor assets; these checks do not claim that package exists yet.

### 2026-09-15 — monster attack/hit milestone

The frozen accepted monster pack now contains176 frames/28 clips: four-cardinal pack-wolf idle/walk/attack/hit and well-alpha idle/walk/hit. Root sampled the new native-scale contacts after the world-art lane reviewed each accepted clip. The short attack is the authored neck/torso lunge; no jaw animation is claimed. Boss side-attack and both death families remain outside this milestone, and incomplete actor families remain inactive in gameplay.

The native runtime now includes204 nonplayer PNGs/56 clips. Every PNG matches an accepted source hash, declared dimensions, binary alpha and48px/metre. No unreferenced runtime PNG remains. Four durable monster builders/transfer/package helpers syntax-compile. The accepted archive adds original CC0 Quaternius wolf source and source-credit metadata. The native death-equipment fix c464ced87 passed first-slice-art, death-disconnect and raster-feedback against this runtime; final complete-actor package and full native gates remain pending.

### 2026-09-15 — owner reduced scope and rejected environment/monster direction

Owner explicitly stopped excessive frame production and rejected current monsters, huts, trees and the gate/portal visible in screenshots. All parallel workers are stopped; do not resume their former frame targets. They also reached the account usage limit. Preserve their unfinished changes. Current local1036-frame runtime was an integration candidate, not a finished owner-approved package. No new player package was promoted.

Read the current hold in native/client/assets/first-slice/README.md. Resume with a minimal Village Palisade set and a small visual composition/guide review using actual owner/project references before generation. Do not interpret the word packs as approved wolves, or inherited Crossroads gateway art as prologue direction. Defer separate sprint/equipment/diagonal expansion and long animation cycles. Player projection still has a verified contrast-loss investigation; native locomotion-cadence fix and tests are local unfinished worker changes. Do not claim these final checks passed.

### 2026-09-15 — single village composition proposal

Owner said do it after the reduced-scope reset. Produced one local Blender layout and one built-in imagegen composition, stored in docs/art-review/2026-09-15-village-composition. Both original and768px display preview were viewed. The packed scene, exact prompt, source references and explicit drift findings are preserved. This proposal is not runtime art or owner approval; no animation workers restarted and no production imports were made.


### 2026-09-15 — native Village Palisade playable implementation

Working branch `codex/starter-slice-20260915`, isolated worktree `Z:/Code/.worktrees/verdigris-starter-slice`, based on a60c87c7. The old dirty art worktree and its unfinished larger frame batches are preserved. Standing owner authorization includes committing verified work and pushing this working branch; no extra push approval is needed.

New native Scions now enter Village Palisade: occupation, civilian branch, two packs, well boss, forgiving retry, first level/skill point, and explicit passage to Crossroads. The server owns phase/reward/persistence, with distinct per-Scion records and compatibility for previously admitted saves. The native client exposes the choices and uses the same progression for general interaction. Core authority tests include actual combat and lethal retry; the starter-slice client scenario covers the real socket/reducer/production painter. See native/STARTER-SLICE.md for controls and scope.

Installed minimal existing male/female unarmed/branch cardinal clips without generating another animation batch. Sprint shares walking. Four Blender-guided ImageGen village props replace the old hut/tree/portal composition in this scene. Their source guides, paint, true alpha and transfer provenance are retained. Live play was launched and captured with the repository tools, and reviewed; that caught overlapping/stale objective prompts and a delayed level-display update, corrected before packaging. Enemy art is the older human raider family and remains provisional. This implementation does not imply owner approval of that family, the earlier wolves, or all final art. Keep art expansion small.

The full native gate, focused starter regression, and packaged verification logs belong under native/build and the review package's qa folder. Consult the actual exit results before asserting acceptance; no browser gate substitutes for native acceptance. Native visual review is separate from owner art approval.


### 2026-09-15 — procedural starter scenery replaces whole-building sprites

Owner rejected single-sprite houses because their baked perspective does not fit the map. The Village Palisade now uses shared seeded generation (`native/include/verdigris/starter_layout.hpp`) for clustered trees/shrubs, rocks/grass and winding paths with a reserved defense clearing. Both native authority and client consume the same placement recipe. Removed both house billboards and their rectangular collision footprints; trunk and rock tiles now provide the actual obstacles. No new image generations or animation batches. Existing small cutouts retain 48 pixels/metre, independent of object bounds. The house source art remains preserved but is not placed in this scene. Crossroads and other historical route layouts are not redesigned by this focused starter-map change.

Tests cover seed replay/variation, bounds, the released house footprint, authoritative obstacle matching, and reachability between the start, well, encounters and exit. Inspect the procedural-village captures and the packaged verification results for this revision. Standing commit/push authorization remains in effect on the working branch.
