# TASK-0163 — Gate-B ordinary-play journey reliability

- state: REVIEW_REQUESTED
- lane: ox-pc-bh
- model: openrouter/stealth/ox-alpha
- base SHA: 75ef6b7b68dd5986f26943728a2796d29e9eec23
- branch: worker/verdigris/pc/ox-pc-bh
- claimed at: 2026-08-23T17:27:35Z
- review requested at: 2026-08-24T00:18:20Z

## Scope

Owned: `native/tests/session_tests.cpp`,
`orchestration/tasks/TASK-0163-gate-b-ordinary-play-reliability/**`.

Minimal change per owner decision (QUESTION-0001, option a): the gate-b
driver's `monster:telegraph` handler must identify the boss ground-slam by its
authored signature (`skillId == "boss:ground-slam"`), not by "any
monster:telegraph". Non-slam telegraphs (ranged trash warnings) must be ignored
by the elite-hunt state machine. No timeout weakening, no assertion deletion,
no runtime/gameplay changes.

## Causal diagnosis (both observed failure surfaces)

One root cause, two surfaces. The pre-fix gate-b driver treated EVERY
`monster:telegraph` envelope as the boss ground-slam. TASK-0108's ranged trash
(Lurkers) now announce ordinary shots on the same `monster:telegraph` event but
with authored `skillId: "monster:attack"`; boss slams use the authored
`skillId: "boss:ground-slam"`.

1. Seven-minute four-kill sweep with no named Warden: a trash telegraph was
   misread as THE elite — the driver re-aimed the elite hunt at the lurker's
   shot tile (`elite_known=1` at a phantom position) and chased shooters
   instead of sweeping to the Warden's room, so the named Warden never spawned
   within the window.
2. Retry failed to observe the fatal fall: each trash telegraph also re-armed
   the slam-dodge window against a warning circle that never resolves into a
   slam, so the dodge state machine spun on phantom windows and the driver's
   fall observation raced/masked the real mortality check.

Fix (test driver only): extracted `GateBSlamKnowledge` +
`gateb_observe_slam()`; only an envelope that is BOTH `event ==
"monster:telegraph"` AND `skillId == "boss:ground-slam"` reveals the elite,
anchors the dodge circle on the payload geometry, and arms
`clear_at = arrival + durationMs + 120ms` skew buffer. All other telegraphs are
ignored by the state machine. Journey checks unchanged: named-Warden kill,
exact-UUID heirloom recovery, mortality/fall, succession, same-guest reconnect.

## Deterministic controls added (socket-free)

`gateb_driver_telegraph_controls()` in session_tests.cpp asserts: trash
telegraph neither reveals an elite nor arms the window; boss slam reveals the
elite at its true tile and anchors the circle on the payload; window arms for
durationMs + skew buffer; a later trash warning neither re-aims nor re-arms;
unattributed telegraphs (missing skillId) are ignored; slam skillId outside
`monster:telegraph` never feeds the machine.

## Acceptance gates — literal commands, exit codes, key output

Ran from clean tree at frozen head `59b299aa` (implementation commit
`1d587f05`; no source or fixture changes between any runs below):

```
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests
→ exit code 0
  native legacy denylist: PASS / verdigris core tests: PASS /
  verdigris networking tests: PASS / camera2d tests: PASS /
  session tests passed / presentation events tests: PASS
  gate-b journey WITH ranged wave present:
    hunt kills #1–#7 The Old Barrow Lurker (trash), #8 Warden of the Deep
    (named elite slain); all gate-b PASS lines incl.
    "slain elite surfaces the circulating heirloom", "exact relic uuid
    recovered underfoot", reconnect continuity block.
  new controls: 8/8 gate-b-driver PASS lines.

1..3 | ForEach-Object { native/build/verdigris_session_tests.exe; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE } }
→ exit code 0; three consecutive runs of the exact session executable,
  each ending "session tests passed"; each found and slew the named Warden
  (kill #8) after 7 lurker kills with elite_known staying 0 until the
  authored slam — no source or fixture changes between runs.

git diff --check
→ exit code 0 (no whitespace errors)

git diff --name-only
→ exit code 0 (empty output; tree clean)
```

No `dev:*`, no direct-state mutation, teleport, seeded reward injection,
hard-coded Warden coordinates, assertion deletion, timeout inflation, runtime
or gameplay change. Diff touches ONLY `native/tests/session_tests.cpp` plus
this task folder.

## Frozen head

- frozen_head: 59b299aac53f550202cf4d0d49d2bc7befe8b127 — every gate above ran
  against exactly this tree; implementation diff is commit `1d587f05`.
- Branch tip at push is this REVIEW_REQUESTED evidence commit (reviewer may
  treat pushed tip as the reviewed head; it differs from frozen_head only by
  STATUS/evidence files in this task folder).

## Heartbeat

- 2026-08-23T17:27:35Z CLAIMED (base 75ef6b7b).
- 2026-08-23T~17:03-07:00 WIP preserved per triage directive (commit 1d587f05),
  triage heartbeat ACTIVE (59b299aa).
- 2026-08-24T00:18:20Z REVIEW_REQUESTED — full gate green, session executable
  3× consecutive green, evidence recorded. Lane freezing this branch.
