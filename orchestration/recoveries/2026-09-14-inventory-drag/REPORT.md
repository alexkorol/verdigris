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
