---
task: TASK-0170
title: Native menu and Escape-state model
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3d358812f86c02e5ad405566413108f97ac4e090
owner_visible_contribution: Native menu and Escape-state model
dependencies: []
owner_input_dependency: none for this bounded packet
owned_paths: [native/client/menu_scene.hpp, orchestration/tasks/TASK-0170-native-menu-scene-model/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, server/**, src/**, docs/product/**, orchestration/PROTOCOL.md, orchestration/DECISIONS.md, everything else]
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: Deliver pure reducer without runtime wiring.
successor_rule: on ACCEPTED release menu integration
generation_provenance: owner interview 2026-08-23 Owner Demo P0
promotion_provenance: current-tip validation at 3d358812f86c02e5ad405566413108f97ac4e090
---

# Outcome

Create deterministic title/pause/menu state where Escape closes the top surface or opens pause and never directly exits; only explicit menu command requests quit.

# Acceptance

C++ tests cover title/play/pause/nested panes/resume/cancel/explicit quit/repeated Escape. Add task-local deterministic tests or verifier with a failing negative control. Run that harness, python native/tools/check_legacy_denylist.py, git diff --check, and prove a clean worktree. Visual/art packets require a contact sheet or capture.

# Evidence and stop conditions

REPORT.md names references, files, commands/exit codes, evidence, commit, residual gaps, and successors. STOP if provenance/source is missing, forbidden integration paths are required, or supplied art would be replaced by generic placeholders.
