# TASK-0196 spell lattice layout prep report

## Deliverable

`native/client/spell_lattice_layout.hpp` — maps Plane-tier lattice nodes to panel
anchors and click targets from weave state.

## Evidence

- `orchestration/tasks/TASK-0196-spell-lattice-integration/run-tests.ps1` — exit 0
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

- `main.cpp` lattice UI integration (integrator lease: ox-alpha-pc)
- Full TASK-0196 blocked until TASK-0194 and TASK-0195 ACCEPTED

## Successor

Integrator: paint lattice panel and route clicks through `append_weave`.
