---
task: TASK-0157
verdict: REVISE
reviewed_head: 024dabb50d0f9e27fc2770c59585e9e629f96e48
reviewed_branch: codex/TASK-0157-native-procedural-audio-scheduler-ox-pc-ad-r2
reviewed_by: PC Verdigris architect/orchestrator
reviewed_at: 2026-08-22 15:10 -07:00
---

# TASK-0157 architect review — REVISE

The backend-neutral library shape is promising and the full existing native
suite passed at the frozen worker head. It is not integration-ready because
two load-bearing acceptance claims fail independent review.

## Required corrections

1. Make the SPEC's literal clean-worktree acceptance sequence true. Starting
   from a fresh detached worktree, the architect ran
   `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1
   -RunTests`; it passed, but `native/build/verdigris_audio_mixer_tests.exe`
   did not exist afterward. The report's direct-binary evidence depended on a
   separate manual CMake configure/build that is not in the acceptance
   sequence. Update the now-owned `native/build.ps1` so `-RunTests` builds and
   links the dedicated audio test executable at the required path. Prove both
   direct executions pass and their captured output is byte-identical without
   any hidden prebuild.
   This numbered revision correction explicitly grants the fresh revision
   lane ownership of `native/build.ps1` solely for that narrow target-wiring
   change; the READY SPEC remains immutable. No other build behavior may be
   broadened or weakened.
2. Restrict the enemy-defeat cue to
   `PresentationEventType::ActorDied` with `event.text == "monster"`.
   `native/src/core.cpp` emits `ActorDied` with `"monster"` for enemy death,
   but emits `ActorDied` with `"scion"` immediately before `ScionLost` for
   player death. The reviewed table ignores that discriminator, so one Scion
   death schedules both a kill cue and a Scion-loss cue. Add focused positive
   and negative tests proving `"monster"` maps to the kill cue while
   `"scion"`, empty, and unknown discriminators remain silent. This also
   restores the accepted TASK-0117 contract that event cues are keyed by
   `(PresentationEventType, text discriminator, value)`.

## Independent evidence retained

- Frozen worker head equals the pushed remote head and the worktree is clean.
- Full `native/build.ps1 -RunTests` suite: PASS.
- Clean-worktree required executable existence check: FAIL (`Test-Path` was
  false after the literal build command).
- Exact worker scope is confined to the original owned audio/CMake/test/task
  paths.
- Source inspection confirms core death discriminators `"monster"` and
  `"scion"`; the reviewed cue table switches only on event type.

Do not amend or force-push the reviewed branch. Continue on a fresh revision
branch, preserve the frozen handoff, rerun the entire literal acceptance
sequence from a clean starting state, and request review again.
