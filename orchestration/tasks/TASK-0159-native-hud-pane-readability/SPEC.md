---
task: TASK-0159
title: Native HUD and gear-pane readability pass
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17
owner_visible_contribution: removes the obvious text collisions and prototype-like hierarchy defects visible during normal 960x600 and 1366x768 play
dependencies: [TASK-0153 ACCEPTED, TASK-0156 ACCEPTED]
owner_input_dependency: none for collision removal and deterministic layout; final typography, art direction, wording, and asset selection remain owner-only
owned_paths: [native/client/main.cpp, orchestration/tasks/TASK-0159-native-hud-pane-readability/**]
forbidden_paths: [native/client/remote_session.cpp, native/client/presentation_state.cpp, native/client/client_model.hpp, native/src/**, native/include/**, server/**, src/**, assets, gameplay rules, balance, authored lore/copy, everything else]
resource_capsule: loopback ports 7100-7119; never touch port 6500
---

# Outcome

Make the current native window read like a deliberate game surface at 960x600
and 1366x768. Remove the visible collision between the global controls hint and
the open gear pane/title, keep the objective/status chips from fighting pane
chrome, and establish a clear hierarchy for identity, objective, connection,
combat HUD, and the existing gear/progression content. This is a bounded layout
and paint-order correction using the existing palette and text; do not redesign
gameplay, invent copy, or hide authoritative information.

# Required proof

- Add or extend a deterministic client scenario that opens the real gear pane
  through the production presentation path at 960x600 and 1366x768.
- Hard-fail on rectangle/text-baseline intersections between global HUD text,
  pane title/stats/progression/footer, objective, connection/art chips, or the
  bottom quickbar/orbs.
- Preserve the first-Escape pane-dismissal contract and the bare-Escape exit
  contract.
- Produce fresh real-GDI captures under this task folder and inspect them before
  handoff.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native/build/verdigris_client.exe --scenario first-session-clarity
git diff --check
git diff --name-only
```

# Negative controls and STOP conditions

No server, wire, simulation, save, balance, asset, font dependency, authored
content, or test-only paint path. Do not solve collisions by deleting the
progression line, controls, objective, connection truth, or art-status truth.
STOP if a requested hierarchy change requires owner-authored copy or art.
