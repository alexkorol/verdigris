# TASK-0192 zone runtime bridge prep report

## Deliverable

`native/client/zone_runtime_bridge.hpp` — composes cartographer_adapter zone
seeds with instance_refresh policies and gate_interaction worlds for Owner Demo
multi-zone runtime.

## Evidence

- `orchestration/tasks/TASK-0192-native-multizone-runtime/run-tests.ps1` — exit 0
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

- `core.cpp` zone load/runtime (integrator lease: ox-alpha-pc)
- Full TASK-0192 blocked until TASK-0178 and TASK-0191 ACCEPTED

## Successor

Integrator: load zones from `build_owner_demo_runtime()` mapgen + gate tables.
