---
task: TASK-0171
title: Diablo-style inventory grid model
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3d358812f86c02e5ad405566413108f97ac4e090
owner_visible_contribution: Diablo-style inventory grid model
dependencies: []
owner_input_dependency: none for this bounded packet
owned_paths: [native/client/inventory_grid.hpp, orchestration/tasks/TASK-0171-native-inventory-grid-model/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, server/**, src/**, docs/product/**, orchestration/PROTOCOL.md, orchestration/DECISIONS.md, everything else]
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: Deliver fixed non-rotating placement/query.
successor_rule: on ACCEPTED release inventory integration
generation_provenance: owner interview 2026-08-23 Owner Demo P0
promotion_provenance: current-tip validation at 3d358812f86c02e5ad405566413108f97ac4e090
---

# Outcome

Create deterministic grid backpack with multi-cell footprints, placement, move/swap rejection, stack metadata, hover, and stable ids.

# Acceptance

C++ tests cover bounds, overlap, deterministic placement, full inventory, invalid items, serialization order. Add task-local deterministic tests or verifier with a failing negative control. Run that harness, python native/tools/check_legacy_denylist.py, git diff --check, and prove a clean worktree. Visual/art packets require a contact sheet or capture.

# Evidence and stop conditions

REPORT.md names references, files, commands/exit codes, evidence, commit, residual gaps, and successors. STOP if provenance/source is missing, forbidden integration paths are required, or supplied art would be replaced by generic placeholders.
