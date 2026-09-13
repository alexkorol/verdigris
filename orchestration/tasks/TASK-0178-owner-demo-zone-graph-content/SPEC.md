---
task: TASK-0178
title: Owner Demo multi-zone graph content
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3d3588126e3abc228721fbed0ff3f8d7cae66448
owner_visible_contribution: Owner Demo multi-zone graph content
dependencies: []
owner_input_dependency: none for this bounded packet
owned_paths: [native/content/seeds/owner_demo_zones.json, orchestration/tasks/TASK-0178-owner-demo-zone-graph-content/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, server/**, src/**, docs/product/**, orchestration/PROTOCOL.md, orchestration/DECISIONS.md, everything else]
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: Deliver town, two combat zones, one dead-end branch.
successor_rule: on ACCEPTED release Cartographer and zone integration
generation_provenance: owner interview 2026-08-23 Owner Demo P0
promotion_provenance: current-tip validation at 3d3588126e3abc228721fbed0ff3f8d7cae66448
---

# Outcome

Author valid graph with village-defense prologue, town, two combat zones, main route, optional branch, bosses/landmarks, gates, and instance metadata.

# Acceptance

Validator proves connectivity/reachability/branch/encounters/gate labels/seeds. Add task-local deterministic tests or verifier with a failing negative control. Run that harness, python native/tools/check_legacy_denylist.py, git diff --check, and prove a clean worktree. Visual/art packets require a contact sheet or capture.

# Evidence and stop conditions

REPORT.md names references, files, commands/exit codes, evidence, commit, residual gaps, and successors. STOP if provenance/source is missing, forbidden integration paths are required, or supplied art would be replaced by generic placeholders.
