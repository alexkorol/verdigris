---
task: TASK-0177
title: Owner Demo town content seed
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3d3588126e3abc228721fbed0ff3f8d7cae66448
owner_visible_contribution: Owner Demo town content seed
dependencies: []
owner_input_dependency: none for this bounded packet
owned_paths: [native/content/seeds/owner_demo_town.json, orchestration/tasks/TASK-0177-owner-demo-town-content/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, server/**, src/**, docs/product/**, orchestration/PROTOCOL.md, orchestration/DECISIONS.md, everything else]
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: Deliver town, three roles, services, one direction.
successor_rule: on ACCEPTED release town runtime integration
generation_provenance: owner interview 2026-08-23 Owner Demo P0
promotion_provenance: current-tip validation at 3d3588126e3abc228721fbed0ff3f8d7cae66448
---

# Outcome

Author valid non-combat settlement with elder, weapons/tools trainer, armor/ritual merchant, services, crisis direction, and readable exits.

# Acceptance

Validator proves classification, NPC roles/services, positions, and exits. Add task-local deterministic tests or verifier with a failing negative control. Run that harness, python native/tools/check_legacy_denylist.py, git diff --check, and prove a clean worktree. Visual/art packets require a contact sheet or capture.

# Evidence and stop conditions

REPORT.md names references, files, commands/exit codes, evidence, commit, residual gaps, and successors. STOP if provenance/source is missing, forbidden integration paths are required, or supplied art would be replaced by generic placeholders.
