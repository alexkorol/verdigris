# TASK-0187 combat VFX layout prep report

## Deliverable

`native/client/combat_vfx_layout.hpp` — maps `attack_vfx` primitives to
fade-weighted stroke plans using `stable_render_order`.

## Evidence

- `orchestration/tasks/TASK-0187-native-combat-vfx-integration/run-tests.ps1` — exit 0
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

- `main.cpp` VFX paint integration (integrator lease: ox-alpha-pc)
- Full TASK-0187 blocked until TASK-0174 and TASK-0186 ACCEPTED

## Successor

Integrator: paint strokes from `plan_combat_vfx` after `actor_combat_bridge` ticks.
