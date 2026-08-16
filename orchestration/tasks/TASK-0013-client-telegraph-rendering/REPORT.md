---
task: TASK-0013
state: REVIEW_REQUESTED
branch: codex/TASK-0013-client-telegraph-rendering
commits:
  - c2d62c3
base_commit: 6b309e7
---

## Executive summary

The Win32 client now renders simulation-owned elite attack telegraphs. It
tracks `AttackTelegraphed` events by actor, snapshots event-time position and
facing, renders a pulsing red thrust wedge or sweep circle, labels the pending
action over the elite life bar, and reports the active telegraph count in the
debug overlay. Warnings end on strike, death, route transition, dead actor, or
elapsed windup.

## Implementation

- Presentation-only event tracking in `native/client/main.cpp`.
- Thrust geometry is a forward half-plane wedge; Sweep is a ground-plane
  circle. Both use the event snapshot and documented approximate private core
  ranges (1100/1650 units) without adding core exports or combat logic.
- Existing procedural effect vocabulary and headless behavior remain intact.

## Changed files

- `native/client/main.cpp` only.

## Interfaces

No core interfaces changed. The client consumes existing
`AttackTelegraphed`, `AttackStarted`, and `ActorDied` events plus actor
snapshots.

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests -RunClient` — PASS
  (denylist, core tests, headless output, and Win32 client build).
- `git diff --check 6b309e7..c2d62c3` — PASS.
- Worktree clean.
- Independent validator `/root/validate_task_0013` — ACCEPT. It confirmed
  exact client-only scope, unchanged headless output, event/expiry/geometry
  behavior, and the supplied before/after captures.

## Manual checks

Driven route:tin:2:0 pass:

- Before strike: debug state showed `telegraphs 1` and a red warning; capture
  `C:\Users\Alex\AppData\Local\Temp\verdigris-task0013-telegraph-before-screen2.png`.
- After strike: player life fell from 78 to 67 and debug state showed
  `telegraphs 0`; capture
  `C:\Users\Alex\AppData\Local\Temp\verdigris-task0013-telegraph-after-screen2.png`.
- Route unlock bootstrap was temporary and reverted before the final commit.

## Specification deviations

None reported.

## Risks and limitations

Core range constants are private, so the client geometry intentionally remains
an approximate presentation promise. No explicit cancellation event exists;
death/strike handling and elapsed windup provide the upper lifetime bound.

## Questions for Fable or the owner

None.

## Integration notes

TASK-0011 is integrated at `c733945`; this is the sole in-flight client task
and must be integrated only after independent validation and architect review.
