---
task: TASK-0172
title: Native paper-doll equipment model
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3d3588126e3abc228721fbed0ff3f8d7cae66448
owner_visible_contribution: Native paper-doll equipment model
dependencies: []
owner_input_dependency: none for this bounded packet
owned_paths: [native/client/paper_doll.hpp, orchestration/tasks/TASK-0172-native-paper-doll-model/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, server/**, src/**, docs/product/**, orchestration/PROTOCOL.md, orchestration/DECISIONS.md, everything else]
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: Deliver visual slot mapping without stat mutation.
successor_rule: on ACCEPTED release paper-doll integration
generation_provenance: owner interview 2026-08-23 Owner Demo P0
promotion_provenance: current-tip validation at 3d3588126e3abc228721fbed0ff3f8d7cae66448
---

# Outcome

Create deterministic paper-doll slots and compatibility for weapon/tool, armor, and accessories without medieval class restrictions.

# Acceptance

C++ tests cover equip/replace/reject/empty/two-handed conflicts/stable ordering. Add task-local deterministic tests or verifier with a failing negative control. Run that harness, python native/tools/check_legacy_denylist.py, git diff --check, and prove a clean worktree. Visual/art packets require a contact sheet or capture.

# Evidence and stop conditions

REPORT.md names references, files, commands/exit codes, evidence, commit, residual gaps, and successors. STOP if provenance/source is missing, forbidden integration paths are required, or supplied art would be replaced by generic placeholders.
