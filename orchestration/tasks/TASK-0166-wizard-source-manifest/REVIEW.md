# REVIEW — TASK-0166 wizard-source-manifest

- reviewer: independent validator (claude subagent), judged and recorded by
  coordinator-of-day claude-architect-pc, 2026-08-24 ~09:15 PDT
- head reviewed: 9334434f (the real deliverable commit — manifest +
  deterministic verifier, 5 files; it sits on
  codex/TASK-0171-native-inventory-grid-model-cursor because the shared
  worktree HEAD moved mid-wave, as f1e63660's truth note records; branch
  ox/TASK-0166 is a red herring containing TASK-0170 work). Both commits
  already ancestors of the program branch; manifest/verifier byte-identical
  to program tip.
- verdict: **REVISE** (minor, targeted — 3 numbered corrections). The
  hash/dimension layer that 0167/0168/0169 consume was independently
  verified sound (9/9 hashes, 3/3 dimensions, verifier + two negative
  modes reproduced, 301 tracked paths zero-drift), so downstream
  acceptances stand.

## REVISE corrections (numbered, testable)

1. **Make the provenance claim honest for untracked sources.** 51/352
   entries (all 6 tmp/orbs-original/extracted/* + 45 splash files under
   assets/textures, assets/world/qa_detailed, tools/verdigris_splash/
   tmp/**) exist in NO WIZARD commit — the uniform sourceCommit pin is
   vacuous for them. Annotate those entries (e.g. "tracked": false, or a
   per-entry provenance field) and scope the manifest's top-level claim.
   Acceptance: verifier proves every entry marked tracked resolves in the
   pinned commit, and the negative control covers a false "tracked" flag.
2. **Dedupe**: tools/gui_framekit/assets/evidence/fk-107-assets-demo.png
   appears twice (identical sha/bytes) — "352 artifacts" is 351 distinct
   files. Dedupe and make the verifier reject duplicates.
3. **REPORT contradiction**: residual-gaps claims staging-wave manifests
   are "referenced by path" for TASK-0169, but
   families.rpg_inventory.stagingManifests is [] and "assets_staging"
   appears nowhere. Populate or correct the REPORT.

## Hardening (non-blocking, may ride with r2)

4. REQUIRED_FAMILIES (native/tools/verify_wizard_source_manifest.py:25)
   enforces only the 4 raster families; reference families and their
   launch/readme paths are never existence-checked.

## OWNER ATTENTION (outside fleet authority)

The 51 untracked WIZARD source files live only as loose files in
Z:\Code\WIZARD (plus their hash-verified copies in THIS repo). A
git clean -fd in WIZARD destroys the originals. Recommend committing them
in WIZARD this evening — the fleet does not write to that repo.

- revision lane: claude-b claims the r2 revision per BUS.
