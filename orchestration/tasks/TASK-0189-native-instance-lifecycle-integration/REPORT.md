# TASK-0189 instance gate bridge prep report

## Deliverable

`native/client/instance_gate_bridge.hpp` — composes `gate_interaction::activate`
with `instance_refresh::evaluate` for EnterZone reuse vs Ctrl-click fresh instance.

## Evidence

- `orchestration/tasks/TASK-0189-native-instance-lifecycle-integration/run-tests.ps1` — exit 0
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

- `networking.cpp` authoritative instance routing (integrator lease: ox-alpha-pc)
- Full TASK-0189 blocked until TASK-0176 and TASK-0188 ACCEPTED

## Successor

Integrator: route `issued_travel` to session join with `instance_id`.
