# Skill-gated inventory extensions

The three reserved equipment seats are removed from the bottom of the main
paperdoll. Warhorn, Quiver / Quick Rig, Attendant, Trophy Rack, Reagents Pouch
and Relics Altar each have a separate left-edge << control. Controls are absent
until the server grants the matching skill-tree unlock. One drawer opens at a
time; the three storage compartments are independent 4x4 grids. Their geometry
is shared by painting, clicking and dropping. Drawers remain within 960x600,
1280x800 and 3440x1440 viewports. The accepted main equipment arrangement and
existing texture/coin/close-button repairs are preserved.

## Authority and persistence

Actual WIZARD AUX_WINDOWS/PACK_DEFS identifiers and rim-10 gate coordinates are
preserved in native/include/verdigris/inventory_extensions.hpp. Owner-requested
display names replace the prototype pack labels. The native tree retains its
existing free-root, node and conduit point costs; its focused view now follows
connected allocations to those gates. Clicking an owned node recenters the
view so previously allocated paths remain navigable. No arbitrary unlock
counter or client-provided flag grants inventory access.

The server validates canonical in-bounds nodes, uniqueness, a connected route,
conduit endpoints and its own earned budget. Stale snapshots cannot remove
existing allocations or revoke access to occupied storage. Allocation, earned
quest points, exact equipment and each compartment's UUID/cell data are saved
per Scion. New Scions start locked; switching back and restarting restore the
original Scion's state. Old saves default to the main backpack and locked tree.

Main and extra compartments share atomic placement/swap rules while occupancy
is isolated per pack. Typed category restrictions are authoritative: trophies,
reagents/tools, and curios/relics/charts respectively. The server supplies item
eligibility to the client for drop previews. Equip and worn-to-pack operations
retain existing hand restrictions and rollback on rejection. All fourteen
physical seat identities are serialized; auxiliary seats remain skill gated.

No new production loot grants, art assets, combat balance or auxiliary combat
abilities were added. The transport tests explicitly label synthetic category
objects as QA fixtures in unique disposable profiles. They are not claimed as
finished production item artwork or additions to the loot catalogue.

## Completed checks

- Full native build and unchanged denylist gate passed.
- All eight native suites passed, including real session travel/combat/Chronicle
  lifecycle and new authority/storage tests. Locked moves/equips, forged budgets,
  disconnected gates, duplicate nodes, category mismatch, compartment bounds,
  Scion isolation and populated restart conservation are covered.
- All 80 client scenarios passed after the main integration. Moving fullscreen
  paint averaged 24.1 ms with a 33.5 ms peak; the 40 ms gate was unchanged.
- The production inventory scenario was extended further and rerun successfully
  after focused keyboard-accessibility fixes. It clicks connected skill-tree
  paths through actual Win32 handlers against a real native server; opens all
  six drawers at three viewports; checks UI input consumption and topmost Escape;
  drags labeled QA objects into and out of each auxiliary equipment seat and
  storage compartment; restarts the same profile; and checks exact persisted
  UUIDs, seats and cells through a fresh client/server connection.
- Keyboard selection reveals the containing drawer and its full-name tooltip.
  Escape keeps that drawer closed on the next paint. Another open sheet blocks
  world drops over its own surface, while uncovered world remains a drop target.
- Representative production captures were inspected, including empty equipment,
  4x4 storage, populated QA compartments and minimum-viewport long-name tooltip.

The first focused run exposed a no-op click becoming a pending network move,
which blocked an immediate subsequent drag. Fixed by keeping pure selection
local. A later category-fixture run exposed keyboard selection into a closed
drawer; fixed by revealing it and transferring focus correctly on close. A
compile-only test-variable naming collision was corrected. The final enhanced
inventory scenario exits 0 with no failures. The 80-scenario run predates only
these last focused keyboard reveal/close changes and extra fixture assertions;
the enhanced inventory rerun covers those final changes. Exact final packaged
verification still follows; no earlier package count is relabeled as this build.

Evidence: evidence/ contains the eight native-suite logs, full 80-scenario log,
final enhanced inventory scenario log, and selected production captures. Complete
captures remain in native/build/extension-ui-evidence and extensions-all-evidence.
This milestone was built from a dirty working tree based on 8e8e4ec85; it is not
claimed as a clean packaged executable. The following implementation commit
records these source changes for subsequent clean packaging.

## Remaining goal work and handover

The earlier drag/currency/texture milestone is retained. The six extensions are
now implemented rather than a storage-only draft. Remaining work is sustained
drag timing/visual verification, integration of the newer owner-selected m5x7
branch, a clean combined package and verification of that exact executable,
and the authorized normal-launch/shared-development-baseline handover.

Normal entry remains C:/Users/Alex/Documents/Verdigris Native 2026-09-13/Verdigris.exe,
last observed with source cefd238b234c0c489abb2718e6b86ba001672c6c from the typography
task. This milestone does not overwrite it or touch the owner's profile. Shared
native baseline remains d0179c8b3. Do not replace m5x7 with this older-font build.
Merge codex/native-typography-sans-20260914, keeping the removed action strip
removed and preserving the newer text measurement and compact-value fixes.

Standing authorization includes committing and pushing verified implementation,
then promoting the verified combined result without another general approval.
Computer Use remains stopped; independent code, builds and application-owned QA
continue. The complete goal is not marked achieved by this milestone.
