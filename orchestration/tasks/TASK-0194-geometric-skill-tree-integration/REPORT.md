# TASK-0194 skill tree layout prep report

## Deliverable

`native/client/skill_tree_layout.hpp` — maps geometric_skill_tree axial seats to
panel pixel anchors and click targets for first level-up UI.

## Evidence

- `orchestration/tasks/TASK-0194-geometric-skill-tree-integration/run-tests.ps1` — exit 0
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

- `main.cpp` level-up tree paint (integrator lease: ox-alpha-pc)
- Full TASK-0194 blocked until TASK-0193 ACCEPTED

## Successor

Integrator: open tree panel on level-up notify using `plan_level_up_panel`.
