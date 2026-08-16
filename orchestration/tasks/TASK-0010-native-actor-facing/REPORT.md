---
task: TASK-0010
state: REVIEW_REQUESTED
branch: codex/TASK-0010-native-actor-facing
commits:
  - 7e066aa
base_commit: 0493eb4
---

## Executive summary

The native simulation now owns deterministic actor facing. Movement and aim
commands set an integer 8-way heading, monsters face their pursuit target, and
Thrust resolves against the attacker's forward half-plane instead of the old
horizontal `+x` proxy. The client sends quantized mouse aim only when the
heading changes and renders the authoritative simulation facing.

## Implementation

- Added `Actor::facing` with bounded `-1/0/+1` components and default `+x`.
- Appended `CommandType::AimIntent` and added `Command::aim(dx, dy)` without
  changing existing command values.
- Updated movement, aim dispatch, and monster pursuit facing in the core.
- Kept Melee and Sweep omnidirectional; Thrust uses integer dot-product
  half-plane filtering.
- Added client aim quantization/throttling and removed the accepted `+x`
  limitation from presentation behavior.

## Changed files

- `native/include/verdigris/core.hpp`
- `native/src/core.cpp`
- `native/tests/core_tests.cpp`
- `native/client/main.cpp`

## Interfaces

- New `Command::aim(int dx, int dy)` input.
- New simulation-owned `Actor::facing` state.
- New appended `CommandType::AimIntent` enum value.

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests -RunClient` — PASS.
- `git diff 0493eb4..7e066aa --check` — PASS.
- Driven Win32/PostMessage pass — PASS: moved past the seeded monster, aimed
  left, pressed Q, and captured `attack thrust` plus `damage thrust (19)` on a
  monster left of the player.
- Independent validator `/root/validate_task_0010` — ACCEPT. Controlled
  capture: `C:\Users\Alex\AppData\Local\Temp\verdigris-task0010-facing-controlled.png`.

## Manual checks

The worker's driven-input capture confirms an aim-left command changes the
simulation heading and allows Q/Thrust to hit the monster on the player's left.

## Specification deviations

None reported.

## Risks and limitations

The heading is intentionally 8-way and the Thrust boundary is a strict
integer half-plane (`dot > 0`); this is deterministic and keeps the core free
of floating-point state. The client mirrors only the quantized heading for
throttling; hit resolution remains core-owned.

## Questions for Fable or the owner

None.

## Integration notes

TASK-0009 is integrated at `0434ebb`; this task is sequential and should be
integrated only after independent validation and architect review.
