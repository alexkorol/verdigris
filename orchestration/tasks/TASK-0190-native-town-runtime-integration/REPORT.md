# TASK-0190 town runtime prep report

## Deliverable

`native/client/town_runtime_layout.hpp` — maps owner_demo_town tile anchors to
interaction bounds, builds `gate_interaction` exit worlds, and exposes town
`instance_refresh` policy for integrator wiring.

## Evidence

- `orchestration/tasks/TASK-0190-native-town-runtime-integration/run-tests.ps1` — exit 0 (24 checks)
- `python native/tools/check_legacy_denylist.py` — PASS
- `npm run playtest` — 32/32 exit 0 (port 6510); see `playtest-tip-evidence.txt`

## Residual gaps

- `core.cpp` town/NPC runtime load (integrator lease: ox-alpha-pc)
- Full TASK-0190 blocked until TASK-0177 ACCEPTED

## Successor

Integrator: spawn NPCs/services from seed JSON using layout anchors.
