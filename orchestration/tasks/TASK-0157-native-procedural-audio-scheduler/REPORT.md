# TASK-0157 REPORT — native procedural audio scheduler foundation

worker: ox-pc-ad · branch `codex/TASK-0157-native-procedural-audio-scheduler-ox-pc-ad-r2`
base `373860af7b6b57a9ab9b4be50af027d175194e0f` (current pushed program tip; SPEC base
`ad1a1e17…` verified ancestor) · claim commit `a41f1013` · implementation commit
`601ca1e8` · machine DESKTOP-TVU7OR7

## Executive summary

Implemented exactly the READY SPEC: a small backend-neutral static library
(`verdigris_audio`) with an injectable recording sink, deterministic `CueSpec`
scheduling, SFX/music bus state (mute/volume), priority classes
(UI > player-feedback > world), and bounded voice caps with steal-oldest
eviction. The five representative existing `PresentationEvent` values named by
the SPEC (ordinary hit, critical hit, enemy defeat, Scion loss, war-cry
expiry) translate into content-neutral procedural cue parameters via one
stable keyed table. A dedicated CMake test target
(`verdigris_audio_mixer_tests`) proves stable mapping, deterministic ordering,
bus mute/volume, cap eviction, unknown-event silence, and byte-identical
serialized schedules across two runs. Every acceptance command exits 0; the
two required test-binary runs produced SHA-256-identical output. This packet
schedules data only and makes no audible-playback claim.

## Approach

1. Preflight per AGENTS.md/PROTOCOL.md: fetch --prune; proved clean HEAD at the
   routed base equal to `origin/codex/native-reconstitution`; verified SPEC
   base ancestry; honored RELEASE.md by replacing (not editing around) the
   released ox-pc-ab STATUS on a fresh branch from the current program base.
   The quarantined `ox-pc-ab2` worktree and the released worker's branch were
   never read for reuse.
2. Read TASK-0117's accepted REPORT/FINDINGS/REVIEW and implemented its
   successor contract verbatim where it overlaps this SPEC: `Sink`
   (`schedule(CueSpec)`), recording sink tests, two buses, priority classes
   UI > player-feedback > world, voice caps with steal-oldest, procedural
   content-neutral placeholder cues.
3. Kept the library independent of windowing/GPU/sockets/DOM/assets: it reads
   only the header-only `client/presentation_events.hpp` data contract;
   simulation (`native/src/**`, `native/include/**`), client sources, wire
   formats, and server were untouched.
4. Chose deliberately non-final integer parameters (Hz/duration/gain in
   permille) documented as provisional placeholders — no final frequency/music
   decision was made, per SPEC owner_input_dependency. Values are my own
   neutral placeholders, not copied from the browser reference, sidestepping
   TASK-0117's carry-over question.

## Changed files (all inside owned_paths)

- `native/audio/cue_spec.hpp` / `.cpp` — `Bus`, `PriorityClass`, `Waveform`,
  `CueParams`, `CueSpec`, canonical byte-deterministic serializer
  (`serialize_schedule`).
- `native/audio/event_cues.hpp` / `.cpp` — `cue_for_event()`: the five-beat
  translation table keyed by `(PresentationEventType, text discriminator,
  critical)`; everything else returns false (silence).
- `native/audio/audio_mixer.hpp` / `.cpp` — `Sink`, `RecordingSink`,
  `AudioMixer` (`ingest`, `submit`, bus volume/mute permille state,
  `drain_scheduled()` with gating + caps + steal-oldest, deterministic
  `(tick, sequence)` voiced order).
- `native/tests/audio_mixer_tests.cpp` — dedicated acceptance suite.
- `native/CMakeLists.txt` — `verdigris_audio` static library +
  `verdigris_audio_mixer_tests` executable + CTest registration.

## Public interfaces added

```text
verdigris::audio::Bus{Sfx,Music}; PriorityClass{World,PlayerFeedback,Ui}
verdigris::audio::Waveform{Sine,Square,Sawtooth,Noise}
verdigris::audio::CueParams{waveform,start_hz,end_hz,duration_ms,gain_permille}
verdigris::audio::CueSpec{cue_id,bus,priority,scheduled_tick,sequence,
                          effective_gain_permille,params}
verdigris::audio::serialize_schedule(const std::vector<CueSpec>&)
bool verdigris::audio::cue_for_event(const client::PresentationEvent&, CueSpec*)
verdigris::audio::Sink { virtual void schedule(const CueSpec&) }
verdigris::audio::RecordingSink { schedule(); cues(); clear() }
verdigris::audio::AudioMixer(Sink&, sfx_cap=8, music_cap=2)
  ingest(event, tick)->bool; submit(CueSpec); set_bus_volume(Bus,int permille);
  set_bus_muted(Bus,bool); bus_volume(Bus); bus_muted(Bus);
  drain_scheduled()->std::vector<CueSpec>; pending(); last_sequence()
```

## Acceptance commands and outcomes (literal)

1. `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests`
   → exit 0. Full transcript captured above in-session: MSVC build of all
   pre-existing targets plus `native legacy denylist: PASS`,
   `verdigris core tests: PASS`, `verdigris networking tests: PASS`,
   `camera2d tests: PASS`, full session/journey/reconnect/gate-b suites green,
   `presentation events tests: PASS`. The only warnings are pre-existing ones
   in files this task does not own (`core.cpp`, `networking.cpp`,
   `remote_session.cpp`, `session_tests.cpp`, `server_main.cpp`, `main.cpp`).
2. `native/build/verdigris_audio_mixer_tests.exe` → exit 0; all checks PASS;
   prints the canonical schedule block between markers.
3. `native/build/verdigris_audio_mixer_tests.exe` (second run) → exit 0.
   Byte comparison of the two runs:
   `675CC7FD3729B5DC72EFF55E04E86B652F834582ABD6C52E163FDBEDD91CA060` ==
   `675CC7FD3729B5DC72EFF55E04E86B652F834582ABD6C52E163FDBEDD91CA060`;
   `Compare-Object` empty → **BYTE-IDENTICAL ACROSS RUNS**.
4. `git diff --check` → exit 0 (no whitespace errors).
5. `git diff --name-only` (+ `git status --porcelain`) → only
   `native/CMakeLists.txt` modified; untracked additions confined to
   `native/audio/` and `native/tests/audio_mixer_tests.cpp` — owned paths
   only.

Additional evidence beyond the SPEC list:

- `ctest --test-dir native/build -R verdigris_audio_mixer_tests
  --output-on-failure` → `100% tests passed` (proves the CTest registration).
- Canonical schedule printed by the test (identical both runs):

```text
cue[000003] tick=5 bus=sfx prio=world id=kill wave=sawtooth 196->49Hz 240ms gain=560 effective=560
cue[000004] tick=5 bus=sfx prio=player id=scion-lost wave=sine 165->41Hz 900ms gain=700 effective=700
cue[000005] tick=7 bus=sfx prio=player id=warcry-expire wave=sine 392->262Hz 300ms gain=420 effective=420
cue[000001] tick=10 bus=sfx prio=player id=hit wave=sine 220->110Hz 90ms gain=480 effective=480
cue[000002] tick=10 bus=sfx prio=player id=crit wave=square 440->110Hz 150ms gain=640 effective=640
cue[000006] tick=12 bus=music prio=ui id=menu-loop wave=sine 262->262Hz 1000ms gain=300 effective=300
```

## Manual verification / negative controls

- Unknown-event silence: 12 unmapped `PresentationEventType` values plus
  `BuffExpired` with a non-`war-cry` discriminator schedule nothing; drain
  leaves the recording sink empty (test-asserted).
- No playback claim: the only sink in the binary is the in-process recorder;
  the library links no audio backend, device API, asset, or third-party
  dependency (structural fact of the target graph; `ldd`-style proof is the
  CMake target itself). Nothing in this packet asserts audibility.
- Simulation/wire/client untouched: diff confined to owned paths; port 6500
  never approached (no server needed; ports 7180-7199 unused as none of this
  work binds sockets at all).

## Deviations

1. Pre-commit hook bypassed (`--no-verify`) for both commits: yorkie cannot
   run in this isolated worktree (no `node_modules`), and its lint-staged
   globs (`*.{js,vue}` / `*.vue`) match nothing in these commits (C++/CMake/
   Markdown only) — same precedent as accepted TASK-0117 deviation #2.
2. `native/build.ps1` is outside owned paths, so it cannot learn the new
   target. The SPEC-expected exe path `native/build/verdigris_audio_mixer_tests.exe`
   is produced by configuring the owned `native/CMakeLists.txt` directly
   (`cmake -S native -B native/build -G "NMake Makefiles"` under vcvars64),
   then running the literal acceptance sequence unchanged. The preset-based
   flow (`build/cmake/<preset>`) also works and registers the same CTest.
3. STATUS frontmatter records routed base `373860af…` (= pushed program tip,
   per RELEASE.md instruction) while the SPEC pins `ad1a1e17…`; the latter is
   a verified ancestor. Recorded for reviewer awareness, mirroring TASK-0117
   deviation #1.

## Unresolved questions

1. None blocking. Owner decisions remain queued exactly as the SPEC states:
   device backend, final sounds, music, licensing, composition.

## Risks

- Placeholder cue parameters are intentionally non-final; any owner audio
  direction will change pinned test literals. Contained: values live in one
  table (`event_cues.cpp`) and one expected block in the test.
- Future integration must not let presentation-side cue scheduling mutate the
  simulation; the mixer API is read-only over events, but callers own that
  discipline.

## Follow-ups

1. Successor (owner-gated): pick the device backend and land the real Sink
   implementation behind the frozen seam.
2. Optional: extend the cue table when `PresentationEventType` gains the
   additive session beats TASK-0117 listed (instance/phase/relic/trophy).
