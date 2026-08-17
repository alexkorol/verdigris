---
task: TASK-0017
state: REVIEW_REQUESTED
branch: codex/TASK-0017-native-movement-feel
base_commit: 1ae1b01
---

# TASK-0017 report — continuous movement / camera feel

## Executive summary

`resolve_move` displaced by the full per-second `move_speed` (300 units) per
MoveIntent, and the client dispatched a MoveIntent every 50 ms timer tick —
6000 units/sec in 300-unit teleports. Movement now uses a named per-tick step
derived from the fixed tick rate (`move_speed * kTickMs / 1000` in integer
math): the player walks 220 units/sec in 11-unit steps, the dash is a single
deterministic hop covering exactly one second of the actor's own movement,
and the client camera defaults implement owner ruling D-107 (slice ARPG
preset) with wheel zoom-in blending toward the Miniature treatment.

No combat range, spawn, or extraction constant changed; the existing content
remains reachable and the loop tests prove it with derived tick counts. The
recorded command stream schema is unchanged (D-002); only constant values
moved.

## Changed files

- `native/include/verdigris/core.hpp` — `kTickMs` and `movement_step_per_tick`.
- `native/src/core.cpp` — per-tick move step, dash hop, player move_speed.
- `native/client/main.cpp` — D-107 camera defaults, anchor/fog projection,
  Miniature zoom blend, distance fog pass, `--log-positions` diagnostic,
  headless demo tick counts derived from the named step.
- `native/tests/core_tests.cpp` — derived movement helpers, two new tests,
  replay stream repair.
- `orchestration/tasks/TASK-0017-native-movement-feel/captures/` — driven
  harness, position log, JPEG evidence (task folder, exempt).

## Constants introduced or changed, with derivations

Core (`native/include/verdigris/core.hpp`, namespace `verdigris`):

- `kTickMs = 50` — the existing fixed timestep (client `SetTimer(..., 50)`),
  promoted to a named simulation-contract constant next to `kTelegraphTicks`.
- `movement_step_per_tick(move_speed) = move_speed * kTickMs / 1000` —
  integer-only derivation; `ActorStats::move_speed` stays a per-second rate.
  Shared by every actor kind (actor symmetry).

Core (`native/src/core.cpp`, anonymous namespace):

- `kDashBurstTicks = 1000 / kTickMs` (= 20) — a dash is one second of the
  actor's own per-tick movement delivered as a single hop. Player hop:
  `11 * 20 = 220` units (2.2 tiles), down from the old `move_speed * 2 = 600`
  (6 tiles) teleport. Stays well inside `kMeleeRange` (1100), so no combat
  spacing breaks. No cooldown added — out of scope; the client only sends
  Dash on keypress.
- Player `move_speed` 300 → 220 — the slice's tuned ARPG pace (SPEC cites
  ~220 units/sec continuous as the reference feel). Per-tick: 11 units.
- Monster `move_speed` unchanged at 240 — monsters have no locomotion in this
  slice; the value remains a valid per-second rate under the shared
  derivation and now documents that in a comment. No behavioral change.

Unchanged core constants, with reachability justification:

- `kEnemySpawnX = 2000`: approach at 220 u/s takes ~4.1 s to enter melee —
  a readable walk, not a slog.
- `kMeleeRange = 1100`, `kThrustRange = 1650`, `kExtractionRange = 250`: all
  still reachable; loop tests (`defeat_enemy`, `extract_from_start`) now
  derive their walk counts from `movement_step_per_tick` and pass unchanged
  in structure. The thrust band crossing against the elite in route:tin:2:0
  is not newly lethal in the tested loop because the player carries position
  from the previous route and starts inside melee range.

Client (`native/client/main.cpp`):

- `kCameraDefaultZoom = 0.85`, `kCameraDefaultPitchDeg = 62.0`,
  `kCameraDefaultPerspective = 0.0006`, `kCameraDefaultAnchor = 0.52`,
  `kCameraDefaultFog = 0.4` — D-107 / slice ARPG preset (evidence:
  TASK-0012 REPORT). `anchor` replaces the hardcoded screen-center
  (`bounds.bottom / 2` → `bounds.bottom * anchor`) in `project`/`unproject`,
  matching the slice's `H * cam.anchor`.
- `kCameraMiniatureZoom = 1.08`, `kCameraMiniaturePerspective = 0.0013`,
  `kCameraMiniatureAnchor = 0.58`, `kCameraMiniatureFog = 0.6`,
  `kCameraMiniatureBlendStartZoom = 1.05` — wheel zoom-in past 1.05 blends
  perspective, anchor, and fog linearly toward the Miniature treatment;
  `t = clamp((zoom - 1.05) / (1.08 - 1.05), 0, 1)`, so the blend completes at
  the Miniature preset's own zoom. Effective values are computed in
  `Camera::effective_*()`; manual `-`/`=` perspective and Home reset keep
  working on the base (ARPG) values.
- Distance fog renders the slice's `shade = (1 - y/H) * fog` gradient (capped
  0.85, slice fog color 13/15/18) as one stretched 1×N premultiplied DIB via
  the already-loaded `AlphaBlend`, drawn with the ground pass so actors and
  telegraphs keep unfogged colors — matching the slice's draw order.

## Tests added / updated

- Added `test_movement_step_matches_tick_rate_derivation` — per-tick
  displacement equals `move_speed * kTickMs / 1000` (11 at 220); diagonal
  splits the step in integer math; one real second of ticks (20) covers
  exactly the per-second rate (220 units).
- Added `test_dash_is_a_bounded_burst` — dash hop equals
  `movement_step_per_tick * (1000 / kTickMs)` = the per-second rate, and
  stays below `kMeleeRange`.
- Updated helpers `reach_enemy` / `extract_from_start` (new `steps_to_cover`
  / `walk`) to derive tick counts from the named per-tick step and live actor
  positions — melee/thrust reachability remains proven by the existing loop
  tests (`defeat_enemy`, `test_extraction`, relic/recovery loops).
- Updated `test_determinism` and `test_relic_resurface_replay_is_deterministic`
  to walk via `reach_enemy` so their streams still reach and kill the enemy
  (the old fixed 4-move streams relied on 300-unit steps). Byte-identical
  replay assertions unchanged.
- Headless demo (`run_headless_demo`, both platform copies) derives its
  approach/return tick counts from the named step; the loop still completes
  with 1 trophy + 1 item stored.

## Verification

```text
powershell -NoProfile -File native/build.ps1 -RunTests -RunClient
native legacy denylist: PASS
verdigris core tests: PASS
Verdigris native client shell
House: House Verdigris | trophies stored: 1 | items stored: 1
```

`git diff --check` — clean.

## Driven-input pass

Harness: `captures/drive.ps1` (PostMessage WM_KEYDOWN/WM_KEYUP/WM_MOUSEWHEEL
against the `VerdigrisNativeClient` window class, PrintWindow with
PW_RENDERFULLCONTENT, JPEG quality 78 — the established TASK-0004/0013/0016
pattern). The client wrote the authoritative per-second positions itself via
the new `--log-positions` flag (`captures/positions.log`):

```text
tick,x,y        note
20,44,0         ~1.0s — walk key down ~0.3s earlier
40,264,0        +220 units in second 2  = 2.20 tiles/s
60,484,0        +220 units in second 3  = 2.20 tiles/s
80,539,0        +55 units (key released mid-second)
100,759,0       +220 units — one dash hop in a single tick
120,759,0       +0 (idle)
```

Walking holds 220 units/sec = 2.20 tiles/sec (acceptance: ~2–2.5). The dash
moves exactly one second of movement (220 units) in one tick; captures show
the dust burst and the eased camera chase, so it reads as a dash, not a
teleport. Movement keys only set held-state flags — all dispatch happens on
the 50 ms timer, so there are no per-keypress bursts; the camera's existing
0.2 lerp smooths the 11-unit steps.

Captures (all 960×600 JPEG, 60–63 KB each, ≤ 250 KB):

| File | Shows |
|---|---|
| 01-walk-start.jpg | D-107 default framing (zoom 0.85, pitch 62, persp 0.0006, fog 0.4) |
| 02-walk-mid.jpg | mid-walk, camera easing behind the player |
| 03-walk-end.jpg | end of the 3.0 s walk (tick 65) |
| 04-dash-before.jpg | settled camera before the dash |
| 05-dash-mid.jpg | dash dust ring + player leading the camera (tick 85, effects 1) |
| 06-dash-settled.jpg | camera re-anchored after the hop (tick 100) |
| 07-zoom-miniature-blend.jpg | wheel zoom 1.13 → persp 0.00130, fog 0.60 (full Miniature blend) |

## Risks and limitations

- Diagonal movement truncates (`(dx*step)/length` per axis): a diagonal tick
  covers 10 units manhattan vs 11 cardinal. Same integer-truncation flavor as
  the pre-change code; deterministic.
- Dash remains +x-directed (pre-existing) and has no cooldown (pre-existing);
  key auto-repeat can chain dashes. Left untouched as out of scope.
- The fog pass is skipped when msimg32 `AlphaBlend` is unavailable (the
  billboard fallback path already reports that condition).
- Stop condition not hit: no range/spawn constant needed to change.
