# TASK-0184 bridge prep report

## Deliverable

`native/client/inventory_pane_layout.hpp` — gear pane geometry matching production
`gear_pane_rect`, Framekit panel/slot plans, 12x6 grid cells, paper-doll slots,
and item-art blits from `inventory_grid` + `paper_doll` + `item_art_renderer`.

## Evidence

- `orchestration/tasks/TASK-0184-native-inventory-pane-integration/run-tests.ps1` — exit 0
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

- `main.cpp` gear overlay paint replacement (integrator lease: ox-alpha-pc)
- Full TASK-0184 blocked until TASK-0171, 0172, 0180, 0182 ACCEPTED

## Successor

Integrator: replace list-rectangle `paint_gear_overlay` with planned blits.
