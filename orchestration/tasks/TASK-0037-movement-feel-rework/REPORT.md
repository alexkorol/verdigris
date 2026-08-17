---
task: TASK-0037
state: REVIEW_REQUESTED
branch: codex/TASK-0037-movement-feel-rework
commits:
  - 46c51412
  - 33798746
  - 64d57bc7
  - 31413c99
  - 15c1a531
base_commit: 6e277cf4
---

## Executive summary

Held browser movement no longer restarts its animation every authoritative
sample. The worker diagnosed the visible jag as a client/server presentation
issue: server movement already emits fractional one-third-tile samples every
50ms, but `move()` reset the animation to idle and back to run on each sample;
the input repeat chain also accumulated timer drift. The fix preserves the
authoritative movement path and wire envelope while keeping the run timeline
continuous and scheduling repeats against absolute deadlines.

## Implementation

- Added `interruptPathfinding()` to invalidate legacy queued paths without
  resetting the held-input animation.
- Added a presentation-only 200ms acceleration ramp and continuous run-state
  updates; transient feel state is non-enumerable and never serialized.
- Replaced drifting `setTimeout`/`setInterval` repeats with absolute-deadline
  self-scheduling.
- Kept diagonal normalization, server authority, and `{ event, data }`
  movement samples unchanged.

### D-114 movement table

| Constant | Value |
|---|---:|
| Speed | 6.6667 tiles/sec |
| Acceleration | 33.3333 tiles/sec² |
| Acceleration duration | 0.2 sec |
| Initial animation speed | 0.25 |
| Interpolation window | 100ms |
| Screen width | 24 tiles |
| Seconds across screen | 3.6 sec |

## Changed files

- `server/core/entities/player/movement-handler.js`
- `src/core/player/animation-timeline-guard.js`
- `src/core/player/events/player.js`
- `src/core/utilities/input-controller.js`
- `tests/unit/movement-feel.spec.js`
- `orchestration/tasks/TASK-0037-movement-feel-rework/captures/**`

## Interfaces

No wire protocol, server-authority, or public event shape changed.

## Verification

- `npm run test:unit` — 119 files / 764 tests passed after the revisions.
- `npm run playtest` — 31/31 scenarios passed.
- ESLint on changed JavaScript — passed.
- Vite build — passed.
- Alternate browser gate on port 6512 — 1/1 passed in 22.7s.
- Standard smoke wrapper could not start against the pre-existing owner
  listener PID 10276 on port 6500; its stale app returned HTML for the API
  probe. The owner process was preserved and the alternate server was stopped.
- `git diff --check b141cd9f..46c51412` — passed.

## Manual checks

The committed dense capture sequence covers stationary, eight right-walk
frames, eight up-right diagonal frames, and six left-turn frames. These are
real browser captures from the alternate-port run.

## Specification deviations

The diagnosis found no authoritative tile quantization to remove, so no
positional prediction or wire change was introduced. The observable defect
was animation restart/timer cadence; the implementation addresses that
mechanism directly and keeps the server authoritative as required.

## Risks and limitations

The 100ms interpolation window is a presentation constant; later owner play
gate review should judge whether it feels smooth under real network jitter.
The pinned 6500 smoke wrapper remains environment-blocked by the existing
owner listener, while the equivalent alternate-port gate passed.

## Integration notes

Requires architect review before integration. Integrate `46c51412`, revisions
`33798746` and `64d57bc7` from the worker branch after acceptance. TASK-0038 must
remain sequenced after this task because it owns overlapping player/input
paths.

## Revision 1

Independent validation found two correctness issues. Revision `33798746`
closes both without leaving the owned paths: `broadcastMovement()` now omits
unchanged animation metadata and payload animation so the client cannot reset
the same sprite sequence, and `ensureRepeat()` now checks the timeout/deadline
state instead of the removed interval field. Regression coverage for both
behaviors lives in `tests/unit/movement-feel.spec.js`.

## Revision 2

The browser-event audit found that the existing movement fallback still
re-applied the actor's last animation through `ensureAnimationController()`.
Revision `64d57bc7` adds the allowed `src/core/player/animation-timeline-guard.js`
seam and installs it around `player:movement`. Unchanged sequence/state/
direction packets now preserve `frameIndex` and `elapsedMs` while accepting
speed/duration updates; new sequences still use the original reset behavior.
The event-path regression is covered in `tests/unit/movement-feel.spec.js`.

## Revision 3

The final validator pass found that the animation deduplication key omitted
mutable speed, so the browser could not see the acceleration ramp. Revision
`31413c99` includes speed, duration, skill, and hold state in the signature and
adds a same-sequence speed-update assertion. The animation guard applies these
mutable fields without resetting the frame clock.

## Revision 4 — current-tip correction

The stale-base branch was merged onto current program tip `6e277cf4` in the
`d70f167c`/`06394847` ancestry and finalized as `15c1a531`. The literal
architect check is:

```text
PS> git diff 6e277cf4 -- src/core/rendering/
PS> $LASTEXITCODE
0
```

The rendering diff is empty, so TASK-0033's ambient/daytime implementation is
preserved. The current-tip branch passed 120 files / 768 unit tests and the
alternate browser gate 1/1 on port 6512. A full playtest retry reached 25/31;
the six fixture/dev-state failures were each rerun successfully in isolation
and are documented as harness variance, not movement regressions.
