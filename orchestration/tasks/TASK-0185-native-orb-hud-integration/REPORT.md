# TASK-0185 bridge prep report

## Deliverable

`native/client/orb_hud_layout.hpp` — production vital-orb anchor geometry + `orb_renderer`
draw plans from authoritative life/resource stats.

## Evidence

- `orchestration/tasks/TASK-0185-native-orb-hud-integration/run-tests.ps1` — exit 0
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

- `main.cpp` `paint_vital_orbs` replacement (integrator lease: ox-alpha-pc)
- Full TASK-0185 blocked until TASK-0181 ACCEPTED

## Successor

Integrator: swap primitive `draw_orb` circles for WIZARD layer blits using this plan.
