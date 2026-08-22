---
task: TASK-0158
title: Native pane model and layout foundation
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
base_commit: ad1a1e178e689df442d4655937f8e8e037cf4cd2
owner_visible_contribution: removes the monolithic fixed-coordinate blocker for readable character, gear, and passive-tree panes
dependencies: []
owner_input_dependency: none for the pure layout/model seam; final art, typography, pane styling, and authored progression content remain owner-only
owned_paths: [native/client/pane_model.hpp, orchestration/tasks/TASK-0158-native-pane-model-foundation/**]
forbidden_paths: [native/client/main.cpp, native/client/remote_session.cpp, native/client/presentation_state.cpp, native/src/**, native/include/**, server/**, src/**, assets, final styling/content decisions, everything else]
---

# Outcome

Create a header-only, pure, dependency-free pane model/layout planner for future Gear,
Character, and Passive tabs. Given viewport, content minimums, active tab, and
row counts, it returns bounded panel/header/tab/content/footer rectangles plus
scroll/clipping metadata. It must never overlap the reserved top HUD or bottom
orbs/quickbar and must degrade explicitly at 960x600 rather than drawing
off-screen. No GDI painting or `main.cpp` integration belongs in this packet.

# Acceptance

Add a self-contained test source and PowerShell compile/run harness under this
task folder covering 960x600, 1366x768, and 1920x1080; zero/large row sets;
tab stability; reserved-region non-overlap; deterministic plans; and invalid
viewport failure. The harness must compile the production header with the
installed MSVC environment and leave outputs only under this task folder.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File orchestration/tasks/TASK-0158-native-pane-model-foundation/run-tests.ps1
git diff --check
git diff --name-only
```

# Negative controls and STOP conditions

No CMake edit, production painting, new hotkey, authored pane copy,
typography/backend choice, content/balance value, or gameplay/network change.
STOP if the pure planner cannot remain independent of Win32/GDI.
