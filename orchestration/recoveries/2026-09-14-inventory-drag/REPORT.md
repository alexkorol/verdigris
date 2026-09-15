# Inventory drag and unattended verification milestone

The native inventory now begins a drag from a worn seat, follows the pointer
with the item image, and sends an exact backpack destination to authority.
The server performs an atomic transfer or compatible replacement and explicitly
acknowledges accepted/rejected operations. Stale identities, incompatible
replacements and footprint overflow preserve the previous items. Dragging onto
the world uses an acknowledged drop transaction, including worn equipment.

The Equip/Unequip and Character button strip and Equipment heading are removed.
The keyboard unequip binding remains. Coins retain their authoritative balance
and saved UUID/quantity but have no backpack cell; old saved purses migrate out
of the grid. The backpack heading shows that balance. Inventory and character
close controls are smaller and centered. The inventory reuses the existing
WIZARD slot_texture.png, with dimmer grid borders; its 1280x800 production
capture was inspected. The accepted paperdoll composition is preserved.

## Verification independent of Computer Use

`native/tools/verify-player-package.ps1 -PackageDirectory <package> -EvidenceDirectory <package>/qa/verification`
checks package hashes/source/resources, executes all native client scenarios,
then runs two top-level launcher lifecycles under one newly generated QA profile.
The application-owned `--verify-launch save|reload` mode creates a hidden test
window and invokes the actual menu/settings/rendering code; it never injects
desktop input or targets an existing player window. Effects 70% must persist
between the two processes; both launcher-owned children must exit cleanly.
The normal launcher rejects this mode without an explicit separate profile.

This makes builds and automated packaged acceptance independent of accidental
Computer Use stops. A stopped player window is left alone. Production captures
still require visual inspection, and automated scenarios are not called a live
owner walkthrough or owner acceptance.

The denylist walker was also made cycle-aware without changing its identifiers,
file types or exemptions. A temporary generated QA junction exposed recursive
enumeration before exemptions were checked. Its removal was blocked by automatic
approval review, so it remains at native/build/launcher-harness-check/native.
The checker now visits each physical directory once, passes the cyclic tree,
still rejects a denied production token and accepts the negative control.

## Completed before packaging

- Native build compiled; core and networking suites passed, including new
  exact placement, rejection, stale world-drop and currency regressions.
- The real-server production-window inventory scenario passed with zero
  failures. Its former Unequip-button check now drags the worn spear to exact
  cells and verifies server placement and full UUID/quantity conservation.
- All 80 client scenarios passed. Frame-budget averages were 23.784 ms stationary
  and 26.197 ms moving at 3440x1440; the existing 40 ms bound was retained.
- Unattended launcher save/reload runs passed on the development executable;
  both restored the same QA profile and ended with clean client/server exits.
  This initial harness run is not a clean packaged-source claim.
- The longer native session suite was still running when this milestone record
  was prepared; final package evidence will be recorded separately.

## Remaining scope

Warhorn, quick rig/quiver, attendant, trophy rack, reagents pouch and relics altar
still need skill-tree authority, persistence and individual left-edge controls.
The old reserved-seat row remains visible in this milestone and must be replaced.
Further drag smoothness/preview checks and final packaged verification remain.
The normal installation is not replaced by this development milestone. Owner
saves and the newer typography/renderer histories are preserved.

Development evidence: native/build/inventory-repair-{scenario2,all,networking}.log,
native/build/inventory-repair-evidence/, native/build/launcher-harness-evidence/,
and native/build/inventory-repair-denylist-regression.log. The first compile
failed on a missing test namespace qualifier; subsequent builds fixed it.
The initial currency and readability tests expected a purse in the backpack;
their updated checks preserve the actual no-cell currency requirement.

## Final unattended packaged verification

Completed on exact clean packaged source
1d3db8ab7b838180dc5f70aafa97c81fd3052723:
C:/Users/Alex/Documents/ChatGPT/verdigris-consolidated-20260913/native/build/player-package-1d3db8ab7/Verdigris.exe.
Its asset root is native/client/assets/raster/runtime inside that package.

- All eight native suites finished successfully; the previously running session
  suite ended in `session tests passed`.
- All 80 scenarios passed in the packaged client, with recorded process exit 0.
  These include the native-server-backed inventory and consolidated menu flows,
  perspective, animation, gameplay regression and existing performance checks.
- Package verification passed 1,271 file hashes, 349 required resources and
  embedded clean executable source identity.
- Two top-level launcher processes used the same executable and the same newly
  created isolated QA profile. Effects 70% persisted across restart; both runs
  returned exit 0 with clean client/server shutdown and no remaining children.
- The production 1280x800 inventory capture and restarted settings capture were
  inspected. The inventory shows textured cells, coins outside the backpack,
  removed action strip and smaller X. The old reserved-seat row remains visible
  and is an outstanding defect, not an accepted completed extension layout.

Entry SHA256: 8E89CCD50FABE817211EB510C1E9A47FA404FEC2E0EC532411FCDDF825FC50C4.
Client SHA256: 4426BE2B64D122F4E83C87F162D0A19A785CCC9DB49FC668C810C615A52BCB2D.
Evidence is retained in evidence/packaged-1d3db8ab7/; complete scenario captures
remain under the package's qa/verification2 directory.

The first verification-wrapper run refused success because Windows PowerShell
lost the redirected process ExitCode. Both wrappers now retain the process
handle before waiting, drain redirected output, and reject absent exit codes.
The corrected complete wrapper rerun ended in exit 0. This is a test-runner fix;
no game binary was changed between those runs. Later publication of this fix
and evidence changes verification scripts/documentation, not packaged game code.

No desktop input was issued after the Computer Use stop. These tests use the
application's own isolated regression mode and production paint/menu/session
functions. They neither target the stopped player window nor disable Escape.
They establish automated results, not a completed live owner walkthrough.

## Normal installation and unfinished integration

A fresh manifest read found the separate typography task had updated the normal
entry C:/Users/Alex/Documents/Verdigris Native 2026-09-13/Verdigris.exe to clean
source cefd238b234c0c489abb2718e6b86ba001672c6c (owner-selected m5x7).
This task did not replace that newer typography with the older-font QA package.
The normal save still hashes to
696F462912374876476010B497FE9C394FCD473E0ACF60CB6CBFA9D01FF427C1.
No Verdigris launcher/client/server processes remained at the final read-only
process check. That absence does not retrospectively prove the interrupted
normal-window quit interaction completed successfully.

The six skill-tree-gated drawers, their left-edge controls and additional drag
preview/smoothness verification remain unfinished. Local work in core.hpp,
core.cpp, networking.cpp, core_tests.cpp and new inventory_extensions.hpp
preserves a separately core-tested multi-pack foundation based on actual WIZARD
IDs and gate coordinates. It is not part of the packaged source or a completed
UI feature. Preserve it, finish authority/persistence/UI integration, and merge
the newer typography branch before packaging and promoting the combined result.
The shared native branch remains d0179c8b3; this verification milestone does not
advance it. There is no outstanding request for owner input or Computer Use
access blocking independent implementation, builds, or packaged regression runs.
