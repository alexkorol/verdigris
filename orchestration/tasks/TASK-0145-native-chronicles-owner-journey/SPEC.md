---
task: TASK-0145
title: Native Chronicles owner-facing journey
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
dependencies: [TASK-0081 ACCEPTED, TASK-0142 ACCEPTED]
base_commit: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2
owned_paths: [native/client/main.cpp, native/client/client_model.hpp, native/client/remote_session.cpp, native/client/remote_session.hpp, native/client/local_session.cpp, native/client/local_session.hpp, native/client/presentation_state.cpp, native/client/presentation_state.hpp, native/client/render_list.hpp, orchestration/tasks/TASK-0145-native-chronicles-owner-journey/**]
forbidden_paths: [native/client/assets/**, native/src/**, native/include/**, native/tests/**, server/**, src/**, playtest/**, .github/**, CI, final owner art]
---

# Outcome

Replace the native client's abrupt game-window entry with an owner-facing
Chronicles journey against the already accepted Gate-B envelopes. A remote
guest who has no active House/Scion must be able to see a coherent pre-game
screen, found a House, create/select a Scion, choose the existing mortal-oath
field when available, and set out without a browser or dev console. Existing
House/Scion state must render on reconnect. In expedition play, surface the
current House, Scion, commission/objective, death/successor, and relic-recovery
states already carried by the accepted protocol; do not invent lore, balance,
or server behavior.

This is the default owner path, not a debug overlay. Preserve local play and
the current TASK-0142 vector presentation. Add a deterministic headless client
scenario for the full screen-state journey and use the real remote session
seam where the harness supports it.

# Acceptance

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native/build/verdigris_client.exe --scenario chronicles-gate-b
git diff --check
git diff --name-only 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2..HEAD
```

All commands exit 0. The new scenario proves screen transitions and actionable
controls, not just parsed fields. REPORT includes fresh screenshots or
offscreen captures of the Chronicle front door and expedition HUD at the
default resolution. No zero is allowed in the Gate-B 12-point rubric in
`docs/rebuild/NATIVE_PRODUCT_CONVERGENCE.md`; score must be at least 9/12.

# Stop conditions

Stop before changing server/protocol authority, assets, gameplay rules, or
content. If an accepted envelope lacks a field, record the exact missing field
and continue every independent UI state rather than inventing it.
