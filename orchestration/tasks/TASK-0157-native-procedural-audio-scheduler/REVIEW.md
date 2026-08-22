---
task: TASK-0157
verdict: ACCEPTED
reviewed_head: bb7b8cebe0e22f832832722801f3999f6a7506be
reviewed_branch: codex/TASK-0157-native-procedural-audio-scheduler-ox-pc-af-r3
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

## Revision 3 review — ACCEPTED, integration held on TASK-0163

Fresh revision head `bb7b8cebe0e22f832832722801f3999f6a7506be`
closes both numbered corrections without widening product behavior:

1. From a new detached worktree with no `native/build` directory, the literal
   `native/build.ps1 -RunTests` command compiled and linked
   `native/build/verdigris_audio_mixer_tests.exe`. This directly closes the
   clean-checkout false green. The script adds only the three production audio
   objects, the focused test object, one link line, and one guarded test run.
2. `ActorDied` now returns silence unless `event.text == "monster"`. Focused
   tests cover `"monster"` positively and `"scion"`, empty, unknown,
   case-mismatched, and critical variants negatively. The production Scion
   death sequence (`ActorDied("scion")`, then `ScionLost`) produces exactly
   one Scion-loss cue.

Independent direct execution of the newly built audio test binary passed
twice with 60 output lines each and byte-identical output. The native legacy
denylist passed. Exact routed-base-to-head scope contains only
`native/audio/event_cues.cpp`, `native/build.ps1`,
`native/tests/audio_mixer_tests.cpp`, and this task's STATUS/REPORT files.
Worker local and remote heads match; the handoff worktree is clean.

The independent full native gate did not finish green: the unrelated Gate-B
ordinary-play journey killed the Warden but missed the exact relic pickup,
then failed downstream crypt/continuity assertions. This is the already-open
TASK-0163 test-driver reliability blocker; TASK-0157 changes no runtime,
networking, session-test, or gameplay path. The audio correction itself is
accepted, but **do not integrate it into the program line until TASK-0163 is
accepted/integrated and the combined literal native gate passes**. This is an
integration hold, not another audio revision request.

Future integration must carry the complete original implementation plus
revision correction lineage (`601ca1e8`, `1a91583e`, and terminal evidence
`bb7b8ceb`) and rerun the focused audio binary twice and the combined full
native gate before recording `integrated_at`.
