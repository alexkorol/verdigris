---
task: TASK-0173
title: Native actor animation-state model
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3d3588126e3abc228721fbed0ff3f8d7cae66448
owner_visible_contribution: Native actor animation-state model
dependencies: []
owner_input_dependency: none for this bounded packet
owned_paths: [native/client/actor_animation.hpp, orchestration/tasks/TASK-0173-native-actor-animation-model/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, server/**, src/**, docs/product/**, orchestration/PROTOCOL.md, orchestration/DECISIONS.md, everything else]
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: Deliver idle/move/one attack/hit/death.
successor_rule: on ACCEPTED release animated actor integration
generation_provenance: owner interview 2026-08-23 Owner Demo P0
promotion_provenance: current-tip validation at 3d3588126e3abc228721fbed0ff3f8d7cae66448
---

# Outcome

Create presentation-only states for idle, locomotion, windup, swing/thrust/slam, hit, death, and recovery for vector/sprite actors.

# Acceptance

Tests prove transitions/timing/facing/interruption/determinism and nonzero visible attacks. Add task-local deterministic tests or verifier with a failing negative control. Run that harness, python native/tools/check_legacy_denylist.py, git diff --check, and prove a clean worktree. Visual/art packets require a contact sheet or capture.

# Evidence and stop conditions

REPORT.md names references, files, commands/exit codes, evidence, commit, residual gaps, and successors. STOP if provenance/source is missing, forbidden integration paths are required, or supplied art would be replaced by generic placeholders.
