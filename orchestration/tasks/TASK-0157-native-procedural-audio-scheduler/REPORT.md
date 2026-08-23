# TASK-0157 REPORT — native procedural audio scheduler foundation (revision 3)

worker: ox-pc-af · branch `codex/TASK-0157-native-procedural-audio-scheduler-ox-pc-af-r3`
routed base `c1ca7d7a` (= pushed revision branch head on origin) · claim commit
`7ffc8696` · implementation commit `1a91583e` · machine DESKTOP-TVU7OR7.
Frozen predecessor (ox-pc-ad r2): head `024dabb5`, claim `a41f1013`,
implementation `601ca1e8`, REVIEW verdict REVISE with exactly two numbered
corrections. This report is self-contained for revision 3.

## Executive summary

Revision 3 applies exactly the two REVISE corrections and nothing else:

1. **Correction 1 — literal clean-worktree acceptance is now true.**
   `native/build.ps1 -RunTests` itself now compiles the `verdigris_audio`
   library sources plus `native/tests/audio_mixer_tests.cpp` and links
   `native/build/verdigris_audio_mixer_tests.exe` at the SPEC-required path,
   then executes it as part of `-RunTests`. No separate manual CMake step is
   needed or used. Proven from a deleted `native/build/` directory: after the
   literal command, the exe exists and both direct executions pass with
   byte-identical output.
2. **Correction 2 — enemy-defeat cue discriminator restored.** The cue table's
   `ActorDied` arm now maps only when `event.text == "monster"`.
   `"scion"` (which core emits immediately before `ScionLost` on player
   death), empty, case-mismatched, and unknown discriminators stay silent, so
   one Scion death schedules exactly one Scion-loss cue and never also a kill
   cue. Focused positive/negative tests prove it; this restores the accepted
   TASK-0117 contract that cues are keyed by `(PresentationEventType, text
   discriminator, value)`.

The backend-neutral library shape from the reviewed predecessor is otherwise
preserved unchanged in behavior: injectable recording sink, deterministic
`CueSpec` scheduling, SFX/music bus state, priority classes
(UI > player-feedback > world), bounded voice caps with steal-oldest,
five-beat event translation, byte-identical serialized schedules across runs.
This packet still schedules data only and claims no audible playback.

## Changed files (all inside revision-owned paths)

- `native/build.ps1` — narrow test-target wiring only (granted by REVIEW
  correction 1): four added `cl /c` compiles (`audio/cue_spec.cpp`,
  `audio/event_cues.cpp`, `audio/audio_mixer.cpp`,
  `tests/audio_mixer_tests.cpp` with `/I audio` + `/I client`), one link into
  `$buildRoot\verdigris_audio_mixer_tests.exe`, one `-RunTests` execution line
  with exit-code guard. No other build behavior broadened or weakened;
  pre-existing targets, defines, and run lines are untouched.
- `native/audio/event_cues.cpp` — `ActorDied` arm gated on
  `event.text != "monster" → return false`; explanatory comment naming the
  core emission contract.
- `native/tests/audio_mixer_tests.cpp` — pinned table row and scripted/ordering
  fixtures updated to use text `"monster"`; new focused test
  `enemy_defeat_requires_monster_discriminator()` (positive mapping to the
  kill cue; silence for `"scion"`, empty, `"elite"`, `"MONSTER"`, and
  critical-flag variants; mixer-level proof that the real player-death
  sequence ActorDied("scion") → ScionLost yields exactly one `scion-lost`
  cue); registered in `main()`.
- `orchestration/tasks/TASK-0157-native-procedural-audio-scheduler/STATUS.md`,
  `REPORT.md` — this handoff. SPEC.md and REVIEW.md untouched.

Unchanged from the frozen predecessor: `native/audio/cue_spec.*`,
`native/audio/audio_mixer.*`, `native/CMakeLists.txt` (CMake target already
correct there).

## Public interfaces

No interface changes versus the reviewed predecessor; semantics of one arm of
one function tightened per REVIEW correction 2:

```text
bool verdigris::audio::cue_for_event(const client::PresentationEvent&, CueSpec*)
  — ActorDied now maps ONLY when event.text == "monster" (→ "kill" cue);
    all other ActorDied texts return false (silence).
```

## Acceptance commands and outcomes (literal, clean starting state)

Starting state proven before the run: `Remove-Item -Recurse -Force
native/build` succeeded (`Test-Path native/build` → False), `git status
--porcelain` empty at implementation commit `1a91583e`. No hidden prebuild.

1. `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests`
   → exit 0. Full transcript captured in-session (SHA-256 of capture
   `84C23BDF…D36D1A`): MSVC compiles of every pre-existing target plus the
   four audio objects, links including `verdigris_audio_mixer_tests.exe`,
   then `native legacy denylist: PASS`, `verdigris core tests: PASS`,
   `verdigris networking tests: PASS`, `camera2d tests: PASS`,
   session/journey/reconnect/replaced/render-list/gate-b suites green,
   `session tests passed`, `presentation events tests: PASS`, the full audio
   mixer suite (mapping, discriminator, silence, ordering, bus, cap,
   serialization checks all PASS), `all audio mixer checks passed`.
   Only warnings are the pre-existing ones in files this task does not own.
   **After this literal command, `Test-Path
   native/build/verdigris_audio_mixer_tests.exe` → True** (the exact point
   that failed under review).
2. `native/build/verdigris_audio_mixer_tests.exe` → exit 0; output captured;
   SHA-256 `88793B5F…A063A4`.
3. `native/build/verdigris_audio_mixer_tests.exe` (second direct run) →
   exit 0; SHA-256 identical:
   `88793B5F317B42A1255FD9B3D0E867F95E5F317FB0ABC28D2AA4AA26A6A063A4` ==
   `88793B5F317B42A1255FD9B3D0E867F95E5F317FB0ABC28D2AA4AA26A6A063A4`;
   `Compare-Object` count 0 → **BYTE-IDENTICAL ACROSS RUNS**, no hidden
   prebuild involved.
4. `git diff --check` → exit 0.
5. `git diff --name-only` → empty; `git status --porcelain` → empty (all work
   committed; tree confined to owned paths by construction).

Canonical schedule printed identically in both direct runs (unchanged content;
only the ingesting event text changed):

```text
cue[000003] tick=5 bus=sfx prio=world id=kill wave=sawtooth 196->49Hz 240ms gain=560 effective=560
cue[000004] tick=5 bus=sfx prio=player id=scion-lost wave=sine 165->41Hz 900ms gain=700 effective=700
cue[000005] tick=7 bus=sfx prio=player id=warcry-expire wave=sine 392->262Hz 300ms gain=420 effective=420
cue[000001] tick=10 bus=sfx prio=player id=hit wave=sine 220->110Hz 90ms gain=480 effective=480
cue[000002] tick=10 bus=sfx prio=player id=crit wave=square 440->110Hz 150ms gain=640 effective=640
cue[000006] tick=12 bus=music prio=ui id=menu-loop wave=sine 262->262Hz 1000ms gain=300 effective=300
```

## Manual verification / negative controls

- Discriminator focus (correction 2): positive `"monster"` → kill cue
  (Sfx/world/sawtooth shape asserted); negative `"scion"`, `""`, `"elite"`,
  `"MONSTER"` (case), critical-flag variants → all silent; end-to-end
  player-death sequence asserts exactly one scheduled cue.
- Unknown-event silence retained: 12 unmapped `PresentationEventType` values
  plus `BuffExpired` with a non-`war-cry` discriminator schedule nothing.
- No playback claim: only sink is the in-process recorder; no audio backend,
  device API, asset, dependency, socket, or listener anywhere (ports
  7220-7239 never bound; port 6500 never approached).
- Simulation/wire/client/server/assets untouched: implementation diff touches
  only `native/audio/event_cues.cpp`, `native/tests/audio_mixer_tests.cpp`,
  and `native/build.ps1` within the granted scope.

## Deviations

1. Pre-commit hook bypassed (`core.hooksPath=Z:/Code/.fleet/no-hooks`) for
   both commits, per packet instruction; same precedent as the predecessor
   report (yorkie cannot run in this isolated worktree and its globs match
   nothing committed here).
2. None other. The prior deviation about `build.ps1` being outside owned
   paths is resolved by REVIEW correction 1's explicit grant; the routed-base
   deviation note from r2 does not recur (this lane was routed at
   `c1ca7d7a` = pushed branch head, and SPEC base `ad1a1e17` remains an
   ancestor of it via the program line).

## Unresolved questions

None blocking. Owner-gated decisions unchanged: device backend, final sounds,
music, licensing, composition.

## Risks

- Pinned cue literals remain provisional placeholders awaiting owner audio
  direction; contained in `event_cues.cpp` + one expected block in the test
  (unchanged assessment).
- If future core code ever emits `ActorDied` with an enemy text other than
  `"monster"`, it will stay silent until the table gains that discriminator;
  this is the conservative direction required by the REVIEW.

## Follow-ups

1. Successor (owner-gated): device backend Sink behind the frozen seam.
2. Optional: extend the cue table for additive session beats listed by
   TASK-0117 (instance/phase/relic/trophy).
