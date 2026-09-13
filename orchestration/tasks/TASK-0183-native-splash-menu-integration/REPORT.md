# TASK-0183 bridge prep report

## Deliverable

`native/client/owner_menu_input.hpp` — composes `menu_scene` + `input_focus` so gear
pane Esc closes before pause opens; bare Esc never requests quit.

`native/client/splash_menu_layout.hpp` — WIZARD splash composition fractions +
Framekit nine-slice panel plans for title and pause roots.

## Evidence

- `orchestration/tasks/TASK-0183-native-splash-menu-integration/run-tests.ps1` — exit 0
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

- `main.cpp` splash/Framekit paint + Win32 key routing (integrator lease: ox-alpha-pc)
- TASK-0183 full integration blocked until TASK-0179 + TASK-0180 ACCEPTED

## Successor

Integrator: wire `owner_menu_input::reduce` into production Escape path and paint title/pause chrome.
