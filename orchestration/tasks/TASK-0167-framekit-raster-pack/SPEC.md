---
task: TASK-0167
title: Framekit raster slice pack
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3d358812f86c02e5ad405566413108f97ac4e090
owner_visible_contribution: Framekit raster slice pack
dependencies: []
owner_input_dependency: none for this bounded packet
owned_paths: [native/client/assets/wizard/framekit/**, native/tools/verify_framekit_assets.py, orchestration/tasks/TASK-0167-framekit-raster-pack/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, server/**, src/**, docs/product/**, orchestration/PROTOCOL.md, orchestration/DECISIONS.md, everything else]
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: Package smallest panel, slot, and circular-frame subset.
successor_rule: on ACCEPTED release Framekit adapter and UI integrations
generation_provenance: owner interview 2026-08-23 Owner Demo P0
promotion_provenance: current-tip validation at 3d358812f86c02e5ad405566413108f97ac4e090
---

# Outcome

Adopt real generated ornate Framekit art as native raster slices: corners, edges, fills, slots, circular frames, ornaments, transparency, and nine-slice metadata. No generic styling.

# Acceptance

Verifier checks alpha/dimensions/hashes/slice bounds; contact sheet reconstructs target. Add task-local deterministic tests or verifier with a failing negative control. Run that harness, python native/tools/check_legacy_denylist.py, git diff --check, and prove a clean worktree. Visual/art packets require a contact sheet or capture.

# Evidence and stop conditions

REPORT.md names references, files, commands/exit codes, evidence, commit, residual gaps, and successors. STOP if provenance/source is missing, forbidden integration paths are required, or supplied art would be replaced by generic placeholders.
