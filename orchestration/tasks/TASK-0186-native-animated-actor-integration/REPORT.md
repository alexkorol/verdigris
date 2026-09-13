# TASK-0186 bridge prep report

## Deliverable

`native/client/actor_combat_bridge.hpp` — ties `actor_animation` phase transitions to
`attack_vfx` primitive emission (swing arc on windup release, hit marker on react).

## Evidence

- `orchestration/tasks/TASK-0186-native-animated-actor-integration/run-tests.ps1` — exit 0
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

- `main.cpp` actor paint integration (integrator lease: ox-alpha-pc)
- Full TASK-0186 blocked until TASK-0173 ACCEPTED; TASK-0187 shares this bridge

## Successor

Integrator: drive actor ticks + VFX planner from authoritative combat events.
