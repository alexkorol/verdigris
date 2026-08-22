---
task: TASK-0157
title: Native procedural audio scheduler foundation
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
base_commit: ad1a1e178e689df442d4655937f8e8e037cf4cd2
owner_visible_contribution: creates the deterministic backend-neutral cue seam required to add responsive combat and UI sound without licensing or device lock-in
dependencies: [TASK-0117 ACCEPTED]
owner_input_dependency: none for this foundation; device backend, final sounds, music, licensing, and composition remain owner-only
owned_paths: [native/audio/**, native/tests/audio_mixer_tests.cpp, native/CMakeLists.txt, orchestration/tasks/TASK-0157-native-procedural-audio-scheduler/**]
forbidden_paths: [native/client/main.cpp, native/client/remote_session.cpp, native/client/presentation_state.cpp, native/src/**, native/include/**, server/**, src/**, binary/audio assets, third-party dependencies, device APIs, final audio direction, everything else]
---

# Outcome

Implement the backend-neutral portion of accepted TASK-0117: a small static
library with an injectable recording sink, deterministic `CueSpec` scheduling,
SFX/music bus state, priority classes, and bounded voice-cap/steal-oldest
behavior. Translate representative existing `PresentationEvent` values for
ordinary hit, critical hit, enemy defeat, Scion loss, and war-cry expiry into
content-neutral procedural cue parameters. This packet schedules data only; it
must not claim audible playback.

# Acceptance

Add a dedicated CMake test target. Tests must prove stable event-to-cue
mapping, deterministic ordering, bus mute/volume, cap eviction, unknown-event
silence, and byte-identical serialized schedules across two runs.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests
native/build/verdigris_audio_mixer_tests.exe
native/build/verdigris_audio_mixer_tests.exe
git diff --check
git diff --name-only
```

# Negative controls and STOP conditions

No audio asset, backend/device API, dependency, simulation mutation, wire
change, final frequency/music decision, or client integration. STOP rather
than choosing miniaudio, SDL, WASAPI, CoreAudio, a license, or authored sound.
