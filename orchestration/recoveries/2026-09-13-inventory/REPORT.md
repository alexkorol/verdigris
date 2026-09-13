# Native inventory, equipment seats and combat ratings

Implemented on the consolidated renderer/application branch above `1d732241b`.
The older dirty `verdigris-playable` checkout was read and preserved. Its useful
inventory, seat and combat-field hunks were ported selectively; already-present
movement, melee, level persistence and build identity were not reapplied.

## Behavior

- Backpack capacity matches native authority at 12x7. Items occupy their actual
  rectangular footprints, including hit targets and drag previews. Occupied-cell
  count replaces the misleading carried-item count. Worn gear stays outside the
  backpack. If a remembered arrangement fragments after a swap, the complete
  server placement prevents silently hidden items. Rearrangement remains local
  presentation state; this change does not add persistence of dragged positions.
- Drops address the actual paper-doll seat, including both ring seats. Clicking
  a worn seat selects its item; U unequips through the native server. Captured
  drags cancel on focus/capture loss or closing inventory. Equipped armor and
  accessories no longer masquerade as the actor's main-hand weapon.
- Authority rejects wrong seats and two-handed conflicts. Swaps and unequips
  that cannot fit roll back completely rather than spilling owned equipment.
  The client uses explicit wear acknowledgement, never disappearance from the
  backpack, to report successful equip. Rejections clear pending feedback.
- Inventory/login/wear projections carry dimensions, seat and two-handed state.
  Gear/character panels consume validated server base attack, gear attack and
  defense ratings. Defense includes equipped channel totals. Damage formulas
  were not changed; these are ratings, not a promised damage-per-hit value.
- Existing raster item art is reused; remaining equipment categories use clear
  seat symbols instead of unrelated blade glyphs. No art generation was needed.
  Narrow Character/Inventory views retain the mixer through compact text layout.

## Checks completed before packaging

- MSVC build and native legacy denylist pass.
- Core, networking, camera2d, Fable camera, presentation-event, audio and settings
  test executables pass. The full session executable passes its death, heir,
  relic recovery, reconnect and exact-item continuity journey.
- The full 79-scenario run exposed six failed assertions in three scenarios:
  narrow dual-pane mixer placement, an obsolete equipped-item-in-backpack
  expectation, and the narrow review strip. All three were fixed; their exact
  scenarios pass on the rebuilt client: `hud-pane-readability`, `equipment`,
  `pane-stack`. `inventory-equipment` also passes after those changes.
- New socket-backed scenario proves authored multi-cell shape and edge rejection,
  real drag-to-seat equips, both rings, wrong-seat rejection at client and server,
  two-handed conflict, U-to-unequip, weapon/shield coexistence, authoritative gear
  stats, full-capacity visibility, atomic full-pack swap rejection, and full-pack
  unequip rejection. It captures production panels at 960, 1366 and 3440 widths.
- Viewed the production inventory captures at 960x600 and 3440x1440 and the
  corrected dual-pane muted capture at 960x600. Perspective and existing actor
  attachment are present. Evidence is under `native/build/inventory-final-evidence`.

## Completed packaged checks

- Clean packaged source: `756066a315b6e98e70ae8073157dc8f7d096ba2b`.
- Exact launcher: `C:\Users\Alex\Documents\ChatGPT\verdigris-consolidated-20260913\native\build\player-package-756066a31\Verdigris.exe`.
- Launcher SHA256: `D1AF57A2A509D9A23BF8D77C8F883EB8CE9AFCB7B3276D113D91EA361AC286E8`.
- Packaged client SHA256: `76C59CB0D984C516EB36A775B13B5E958B206E344A5CC25CBFEDCFA178EA5090`.
- Package validation passes all 1,216 manifest files, 296 required resources,
  source identity, profile alias locking and lock release after closure.
- All **79 packaged scenarios pass**, exit 0. This includes `consolidated-flow`,
  `inventory-equipment`, movement/camera, first fight, raster motion, frontend,
  audio preferences, pack drag and frame budget. The run used the packaged
  client from its package directory, `qa/settings.ini`, and `qa/captures`.
  Log: `native/build/inventory-packaged-scenarios.log`.
- Moving-frame average was 33.0 ms, passing the 40 ms average gate; the observed
  peak was 48.4 ms. This does not claim every frame remained below 40 ms.
- Launched this exact `Verdigris.exe` from outside its package with `--quick`
  and isolated `qa/launch-profile`. Verified launcher PID 25960 spawned the
  packaged server and client. Its session log records clean source `756066a31`,
  `renderer=Fable perspective=1 authored-poses=96`.
- Viewed its 3440x1440 gameplay capture and packaged inventory captures at
  960x600 and 3440x1440. Perspective and the native gear/backpack panels are
  present. Evidence: package `qa/launcher-gameplay.png` and `qa/captures`.

## Remaining limits

No automated failures remain. The separate full manual walkthrough of character
creation, movement/attacks and restart settings is not claimed complete by the
quick-launch check. No new live same-profile restart setting check was performed
on this source. Normal-launch promotion remains pending that broader verification;
it is not a prerequisite for this completed inventory implementation. The normal
launch entry and owner saves are unchanged. QA launcher cleanup is process cleanup,
not evidence of manual menu Quit.

Dragged backpack arrangement remains client-local. No new item art or damage
formula was introduced. The preserved older integration checkout remains intact.

The verification report follows the packaged source in a documentation-only
commit; subsequent report hashes do not imply additional packaged game changes.
