# TASK-0173 report — native actor animation-state model

## References

- `orchestration/tasks/TASK-0173-native-actor-animation-model/SPEC.md` — owned/forbidden paths, acceptance
- `native/client/menu_scene.hpp` (accepted TASK-0170) — house style pattern-matched (constexpr free-function reducer, Status/Decision, constexpr name())
- `orchestration/tasks/TASK-0170-native-menu-scene-model/run-tests.ps1` — cl.exe/vcvars64 invocation reused verbatim
- Successor target: TASK-0186 animated actor integration

## Deliverable

Header-only presentation model `native/client/actor_animation.hpp`
(namespace `actor_animation`, zero dependencies beyond `<array>/<cstdint>`):

- **States:** Idle, Locomotion (speed blend 0–1000 permille), Windup, Attack
  variants Swing/Thrust/Slam/Cast (normalized-time phase per variant),
  Recovery, Hit reaction, Death — plus renderer-ready `Pose` output
  (phase, keyframe index, facing degrees, root bob, weapon/off-arm angles,
  lean).
- **Event-free advance:** `tick(dt_ms)` alone moves time; overflow carries
  into successor states so identical dt sequences land identically. Death's
  clock runs on without ever wrapping/replaying.
- **Interruption rules:** hit interrupts windup only below halfway
  (i-frames deliberately out of scope); active attacks absorb hits; death
  overrides every state including mid-swing and is irreversible (all requests
  answer `Terminal` forever); fresh attacks chain only from the late recovery
  window (`recovery_chain_window_ms`); facing locks during committed actions
  and after death.
- **Facing:** 8-way compass with wrap-around rotation helpers.
- **Determinism/purity:** constexpr-friendly, IEEE basic arithmetic only
  (no libm), no clocks, no I/O, no rendering.

Task-local harness: **87 checks**, MSVC `/std:c++20 /W4` clean.

## Negative control (documented)

`run-tests.ps1` builds twice:

1. Normal build — all checks must pass (exit 0).
2. `/DNEGATIVE_CONTROL` build — compiles an inverted assertion claiming a
   zero-duration attack config is acceptable (`valid_config(zeroed)` in
   `test_nonzero_visible_attacks`). That binary **must** fail with exit 1;
   the harness throws if it does not. Verified: negative run printed
   `FAIL: NEGATIVE CONTROL (expected failure): zero-duration attack accepted`
   and exited 1, harness still exited 0 because the failure was required.
   This proves the nonzero-visible-attack checks can detect an
   invisible-zero-length-attack regression rather than passing vacuously.

## Commands and exit codes

```text
powershell -NoProfile -ExecutionPolicy Bypass -File orchestration/tasks/TASK-0173-native-actor-animation-model/run-tests.ps1
  -> exit 0; acceptance build "87 checks passed"; negative-control build
     failed with exit 1 as required; harness PASS

python native/tools/check_legacy_denylist.py
  -> exit 0; "native legacy denylist: PASS"

git diff --check
  -> exit 0, silent

git status --porcelain (after final commits)
  -> empty
```

## Files

| Path | Role |
|---|---|
| `native/client/actor_animation.hpp` | Production animation-state model |
| `orchestration/tasks/TASK-0173-native-actor-animation-model/actor_animation_tests.cpp` | Acceptance tests (87 checks + negative control) |
| `orchestration/tasks/TASK-0173-native-actor-animation-model/run-tests.ps1` | MSVC dual-build harness |
| `orchestration/tasks/TASK-0173-native-actor-animation-model/STATUS.md` | Claim / review request |
| `orchestration/tasks/TASK-0173-native-actor-animation-model/REPORT.md` | This file |

## Evidence

Pure model packet: no screenshot possible or required (SPEC forbids touching
`main.cpp`; nothing renders here). Proof of behavior is the test matrix:
transition legality, timing/carry wrap, facing lock/wrap, interruption matrix,
death irreversibility, byte-identical determinism trace (>2000 chars,
two-run equal), nonzero distinct attack curves per variant, zero-duration
rejection at config validation.

## Residual gaps

- Not integrated with any actor/session code — that is TASK-0186's job
  (released on ACCEPTED per successor_rule)
- Locomotion requests are refused while busy (windup/attack/hit): callers
  must re-request movement after recovery resolves; no input buffering yet
- Pose limb angles are abstract degrees for vector/sprite rigs; binding them
  to actual skeleton/sprite strips is integration work

## Successor note (TASK-0186 animated actor integration)

Drive one `State` per visible actor: call `attack/start_move/take_hit/die`
from gameplay intents, feed `tick(dt, cfg)` once per frame with a shared
`default_config()` per archetype, render from `pose(state, cfg)` (keyframe +
phase select frames; angles drive vector limbs; `facing_degrees` picks the
sprite row). Death needs no cleanup hook — it self-latches. Keep configs
validated once at spawn (`valid_config`) so zero-length animations can never
ship.
