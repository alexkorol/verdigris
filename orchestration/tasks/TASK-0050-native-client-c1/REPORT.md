---
task: TASK-0050
state: REVIEW_REQUESTED
coordinator: deepseek
worker_branch: codex/TASK-0050-native-client-c1-deepseek
base_commit: cc67a15ee7adb4244ba12d2e14296097f6afa288
architect_review_required: true
---

# TASK-0050 REPORT — Native client wave C1

## Executive summary

All three owner-visible deliverables are implemented on the native Win32
client, driven by D-118 (drop the broken 2.5D billboard parallax for a clean
2D top-down):

1. **2D top-down presentation** — the client now projects through the
   architect's `camera2d.hpp`. The depth-scaled `project()`/`unproject()` and
   every 2.5D constant/field/method are deleted; world entities draw at a
   uniform, camera-independent scale and sort by `camera2d::draw_order_key`.
2. **Visible combat** — elite thrust/sweep telegraphs, swing/sweep/war-cry
   arcs, hit flashes, **floating damage numbers** (red when the Scion takes
   the hit), monster death rings + removal, and drop diamonds are all rendered
   from core events.
3. **Real inventory pane** — a grid backpack + weapon paperdoll seat + stats
   readout (with equipped attack bonus) + banked/extraction summary, with
   equip/unequip working through the pane.

The native core gained one small, deterministic command (`unequip`) so the
pane can empty the weapon seat; it is locked by a new core test.

## What was REMOVED (the projection path)

From `native/client/main.cpp`:

- `kCameraDefaultPitch`, `kCameraDefaultPerspective`, `kCameraDefaultAnchor`,
  `kCameraDefaultFog`, `kCameraMiniaturePerspective`,
  `kCameraMiniatureZoomThreshold`, `kCameraMiniatureBlendEndZoom`.
- `Camera::pitch_deg`, `Camera::perspective`, `Camera::anchor`,
  `Camera::fog`, `Camera::ground_squash()`, `Camera::depth_scale()`,
  `update_camera_perspective()`.
- The old `project()` body (`screen X *= depth_scale(rel_y)` — the exact
  slide-against-motion bug) and the old `unproject()` body.
- Pitch/perspective input bindings (PgUp/PgDn, `-`/`=`), and the Home reset of
  pitch/perspective/anchor/fog (Home now resets zoom only).
- Every `ground_squash()` call site (telegraphs, effects, contact shadows,
  extraction pad, facing line) — replaced with uniform circles and no
  vertical "lift" offset.

Nothing reintroduces a position-dependent scale: `camera2d_tests.cpp` locks
translation invariance, round-trip, uniform scale, and centering, and it
passes.

## Changed files

- `native/client/main.cpp` — camera swap + 2.5D removal, draw-order sort,
  floating damage numbers, grid/paperdoll/stats inventory pane, unequip
  binding (`U`).
- `native/include/verdigris/core.hpp` — `CommandType::Unequip`,
  `Command::unequip()`, `resolve_unequip()` declaration.
- `native/src/core.cpp` — `Command::unequip()` + `resolve_unequip()`.
- `native/tests/core_tests.cpp` — equip/unequip identity test extension.

## Public interfaces added

- `verdigris::Command::unequip()` and `verdigris::CommandType::Unequip` —
  clears the equipped flag and the actor's `equipped_item_id` (no other rule
  change; the attack bonus already resolves dynamically from the equipped id).

## Test commands and outcomes

`powershell -File native/build.ps1 -RunTests -RunClient`:

```
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
camera2d tests: PASS
Verdigris native client shell
House: House Verdigris | trophies stored: 1 | items stored: 1
```

## Scripted demo — documented manual sequence

The interactive exe (`native/build/verdigris_client.exe`) cannot be captured
headlessly in this environment, so the D-117 play test is the architect's; the
sequence below is the exact input path that produces each required beat
against the seeded `route:tin:1:0` start.

1. **Move in all 8 directions near the starting trees/houses** — `W/A/S/D`
   (and diagonals). Expectation: scenery stays fixed relative to the world; no
   element slides against player motion (the `camera2d` translation-invariance
   test is the standing proof).
2. **Kill → swing → damage → drop** — hold `W` toward the spawned enemy, press
   `LMB` (melee) or `Q` (Thrust) in range. Expectation: a swing arc, a hit
   flash, a floating white/yellow damage number, the monster life bar
   shrinking; on kill a death ring + dust, the monster vanishes, and a loot
   diamond (item, gold) plus sparkle appears at the kill site. Elite monsters
   first show a red thrust/sweep telegraph circle/fan during their windup
   (from `AttackTelegraphed`).
3. **Pickup into the pane** — stand near the drop and press `X`. Expectation:
   the diamond disappears and the item appears in the gear pane (`I`).
4. **Equip** — press `I`, use arrow keys to select the item, press `Enter`.
   Expectation: the Weapon seat shows the item name; the stats readout `ATK`
   gains `(+N)`.
5. **Unequip** — with the pane open, press `U`. Expectation: the Weapon seat
   returns to `(empty)` and the `(+N)` disappears.
6. **Extract** — return to the blue extraction pad and press `F`. Expectation:
   the pane's `Banked items N · trophies M` increments (and the headless proof
   `trophies stored: 1 | items stored: 1` is the same path).

## Deviations / notes

1. **Projectiles.** The native core's action vocabulary is
   `Melee/Dash/Thrust/Sweep/WarCry` (no ranged skill). "Swings/projectiles"
   therefore renders as swings/arcs only; no new core skill was invented
   (native boundary — presentation reads state, never owns rules).
2. **Single weapon seat.** The core models one equipped item
   (`Actor::equipped_item_id`), so the "paperdoll" is a single Weapon seat —
   native-testbed fidelity, not browser pixel parity. `unequip` was added to
   the core (owned path `native/**`) to make equip/unequip genuinely work.
3. The architect should replay the manual sequence above and judge D-117 feel
   before ACCEPTED.

## Commits

- `5cfb0c4` (claim), `98b8e38a` (D1), plus the D2/D3 + core-unequip commit
  (SHA recorded at push).
