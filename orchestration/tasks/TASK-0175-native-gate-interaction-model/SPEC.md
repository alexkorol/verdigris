---
task: TASK-0175
title: Readable gate interaction model
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3d3588126e3abc228721fbed0ff3f8d7cae66448
owner_visible_contribution: Readable gate interaction model
dependencies: []
owner_input_dependency: none for this bounded packet
owned_paths: [native/client/gate_interaction.hpp, orchestration/tasks/TASK-0175-native-gate-interaction-model/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, server/**, src/**, docs/product/**, orchestration/PROTOCOL.md, orchestration/DECISIONS.md, everything else]
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: Deliver label/highlight/normal entry with explicit refresh intent.
successor_rule: on ACCEPTED release gate integration
generation_provenance: owner interview 2026-08-23 Owner Demo P0
promotion_provenance: current-tip validation at 3d3588126e3abc228721fbed0ff3f8d7cae66448
---

# Outcome

Create physical gate interaction model with destination labels, hover highlight, range, normal entry, and Ctrl-click fresh-instance intent.

# Acceptance

Tests cover hover, labels, range, modifier, inaccessible gates, deterministic commands. Add task-local deterministic tests or verifier with a failing negative control. Run that harness, python native/tools/check_legacy_denylist.py, git diff --check, and prove a clean worktree. Visual/art packets require a contact sheet or capture.

# Evidence and stop conditions

REPORT.md names references, files, commands/exit codes, evidence, commit, residual gaps, and successors. STOP if provenance/source is missing, forbidden integration paths are required, or supplied art would be replaced by generic placeholders.
