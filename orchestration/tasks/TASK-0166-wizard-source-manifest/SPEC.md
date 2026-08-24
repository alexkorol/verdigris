---
task: TASK-0166
title: WIZARD source and provenance manifest
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3d358812f86c02e5ad405566413108f97ac4e090
owner_visible_contribution: WIZARD source and provenance manifest
dependencies: []
owner_input_dependency: none for this bounded packet
owned_paths: [native/client/assets/wizard/source_manifest.json, native/tools/verify_wizard_source_manifest.py, orchestration/tasks/TASK-0166-wizard-source-manifest/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, server/**, src/**, docs/product/**, orchestration/PROTOCOL.md, orchestration/DECISIONS.md, everything else]
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: Produce verified source map without copying assets.
successor_rule: on ACCEPTED release source-family asset and adapter packets
generation_provenance: owner interview 2026-08-23 Owner Demo P0
promotion_provenance: current-tip validation at 3d358812f86c02e5ad405566413108f97ac4e090
---

# Outcome

Inventory exact Framekit, orbs, RPG Inventory, Cartographer, skill-tree, spell-lattice, splash, and Chronicles sources with hashes, provenance, dimensions/types, destinations, and reference evidence.

# Acceptance

Verifier fails on missing sources/hashes and report covers every required module. Add task-local deterministic tests or verifier with a failing negative control. Run that harness, python native/tools/check_legacy_denylist.py, git diff --check, and prove a clean worktree. Visual/art packets require a contact sheet or capture.

# Evidence and stop conditions

REPORT.md names references, files, commands/exit codes, evidence, commit, residual gaps, and successors. STOP if provenance/source is missing, forbidden integration paths are required, or supplied art would be replaced by generic placeholders.
