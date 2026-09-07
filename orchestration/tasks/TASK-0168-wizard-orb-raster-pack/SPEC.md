---
task: TASK-0168
title: WIZARD life and mana orb asset pack
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3d3588126e3abc228721fbed0ff3f8d7cae66448
owner_visible_contribution: WIZARD life and mana orb asset pack
dependencies: []
owner_input_dependency: none for this bounded packet
owned_paths: [native/client/assets/wizard/orbs/**, native/tools/verify_wizard_orb_assets.py, orchestration/tasks/TASK-0168-wizard-orb-raster-pack/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, server/**, src/**, docs/product/**, orchestration/PROTOCOL.md, orchestration/DECISIONS.md, everything else]
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: Deliver static layered raster states.
successor_rule: on ACCEPTED release orb adapter and HUD integration
generation_provenance: owner interview 2026-08-23 Owner Demo P0
promotion_provenance: current-tip validation at 3d3588126e3abc228721fbed0ff3f8d7cae66448
---

# Outcome

Package actual WIZARD life/mana art, masks, fills, surrounds, and metadata for non-primitive depletion states.

# Acceptance

Verifier checks layers/alpha; contact sheet shows full/half/low/empty states. Add task-local deterministic tests or verifier with a failing negative control. Run that harness, python native/tools/check_legacy_denylist.py, git diff --check, and prove a clean worktree. Visual/art packets require a contact sheet or capture.

# Evidence and stop conditions

REPORT.md names references, files, commands/exit codes, evidence, commit, residual gaps, and successors. STOP if provenance/source is missing, forbidden integration paths are required, or supplied art would be replaced by generic placeholders.
