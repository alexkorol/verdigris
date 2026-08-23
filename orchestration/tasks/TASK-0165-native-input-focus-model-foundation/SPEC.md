---
task: TASK-0165
title: Native input focus and pane-close model foundation
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
base_commit: b949b3e4653961b7f13661f38ef3addfb8af0df4
owner_visible_contribution: prevents future gear, character, passive, modal, and text panes from reintroducing global-Esc exits or leaking gameplay input through focused UI
dependencies: [TASK-0153 ACCEPTED]
owner_input_dependency: none for deterministic focus and close ordering; final bindings, pane styling, and authored copy remain owner-only
owned_paths: [native/client/input_focus.hpp, orchestration/tasks/TASK-0165-native-input-focus-model-foundation/**]
forbidden_paths: [native/client/main.cpp, native/client/pane_model.hpp, native/client/remote_session.cpp, native/client/presentation_state.cpp, native/src/**, native/include/**, native/tests/**, native/CMakeLists.txt, server/**, src/**, bindings, styling, authored copy, gameplay rules, everything else]
---

# Outcome

Create a header-only deterministic input-focus reducer for future native panes.
Given the currently focused surface and an abstract input intent, it returns the
next focus plus an explicit disposition (`Consumed`, `PassToGameplay`, or
`RequestQuit`). The model must encode the accepted first-Esc-closes-the-topmost-
pane, second-bare-Esc-requests-quit contract; prevent movement/combat intents
from leaking through modal/text focus; and keep nonmodal pane navigation
deterministic. This packet freezes no key binding and performs no Win32/GDI or
`main.cpp` integration.

# Acceptance

Add a self-contained C++ test source and PowerShell MSVC compile/run harness in
this task folder. Cover no-focus, Gear, Character, Passive, modal, and text
focus; stacked close priority; first-Esc/second-Esc behavior; gameplay-input
suppression; navigation consumption; unknown intent safety; determinism; and
invalid state failure without undefined behavior.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File orchestration/tasks/TASK-0165-native-input-focus-model-foundation/run-tests.ps1
python native/tools/check_legacy_denylist.py
git diff --check
git diff --name-only
```

# Negative controls and STOP conditions

No keycode or final binding choice, no production event loop or painting edit,
no hidden global mutable state, no gameplay command dispatch, no window close,
no authored copy/style, no dependency, and no CMake change. STOP if the pure
model cannot express the accepted Esc ordering without changing production
runtime behavior in this packet.
