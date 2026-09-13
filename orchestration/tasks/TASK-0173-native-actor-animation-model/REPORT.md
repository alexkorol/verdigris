# TASK-0173 report

## Deliverable

`native/client/actor_animation.hpp` — tick-driven phases: idle, locomotion, windup,
swing/thrust/slam, hit, recovery, death. Facing updates, hit interrupts windup,
death terminal, deterministic replay.

## Evidence

80 checks PASS, denylist PASS. `is_attack_visible` true during windup/swing/thrust/slam.

## Successor

TASK-0186 animated actor integration.
