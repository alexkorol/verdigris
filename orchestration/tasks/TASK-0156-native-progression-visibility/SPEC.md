---
task: TASK-0156
title: Native passive-tree progression visibility
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: ad1a1e178e689df442d4655937f8e8e037cf4cd2
owner_visible_contribution: makes earned and committed Verdigris progression visible in the normal native gear surface instead of silently living only in the server payload
dependencies: [TASK-0119 ACCEPTED, TASK-0153 ACCEPTED]
owner_input_dependency: none; authored node names, topology, effects, costs, balance, lore, and final pane design remain owner-only
owned_paths: [native/client/client_model.hpp, native/client/remote_session.cpp, native/client/presentation_state.cpp, native/client/presentation_state.hpp, native/client/main.cpp, orchestration/tasks/TASK-0156-native-progression-visibility/**]
forbidden_paths: [native/src/**, native/include/**, native/tests/**, native/CMakeLists.txt, server/**, src/**, playtest/**, content/balance decisions, everything else]
resource_capsule: native client implementation; use only the routed 20-port loopback capsule; never touch port 6500
---

# Outcome

Close accepted TASK-0119 gap G-2 without inventing a passive-tree design.
Mirror the existing authoritative `passiveTree` envelope into plain client
model fields and show a compact progression summary in the already-shipped
`I` gear overlay. At minimum the owner sees earned/unspent skill points and
committed allocation count. The UI must distinguish zero, nonzero, and absent
payload state; absence may not be rendered as zero.

Use the existing `passiveTree.points.skill`, `nodes`, and `conduits` payload
shape. Do not render raw JSON, create authored node labels, add allocation
commands, or imply that a selection UI exists. The summary must be legible at
960x600 and 1366x768 without colliding with backpack, banked state, controls,
or the top HUD.

# Required proof

- Add a deterministic client scenario `progression-surface` in the existing
  scenario harness inside `main.cpp`. It must ingest a real representative
  remote payload through the production parser/presentation seam and prove
  absent, zero, and nonzero states plus visible render-list text.
- Preserve every existing client scenario and the Esc-first pane-dismissal
  contract.
- Produce fresh 960x600 and 1366x768 captures under this task folder from the
  real GDI paint path and inspect them before handoff.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native/build/verdigris_client.exe --scenario progression-surface
git diff --check
git diff --name-only
```

# Negative controls and STOP conditions

- No server, simulation, protocol-schema, save, balance, or passive-tree
  authority change.
- No test-only render behavior or direct production-state mutation.
- No raw node IDs presented as owner-facing copy.
- STOP if the current payload cannot support an honest points/allocation
  summary without a server change; report the exact missing field instead.

