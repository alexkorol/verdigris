---
task: TASK-0179
state: REVIEW_REQUESTED
worker: ox-alpha-pc-w6
worktree: Z:\Code\.worktrees\verdigris\worker-t0179
branch: ox/TASK-0179
base_commit: 1498048722d1d3a54ae8d1d48677b87e503af241
started_at: 2026-08-24T00:52:00-07:00
finished_at: 2026-08-24T01:14:00-07:00
claim_commit: 943be01a
implementation_commit: fb816a97
---

Claimed by lane ox-alpha-pc-w6. Plan: adopt a minimal coherent Verdigris splash set from WIZARD/tools/verdigris_splash (night-lit disc-world hero, floating-disc menu backdrop variant, alpha planet/sky/fx layers) into native/client/assets/wizard/splash/**, with splash_meta.json (roles, z-order, sha256), adoption_manifest.json (full provenance vs Z:\Code\WIZARD @ 6c2f9e72), deterministic verifier with --corrupt negative control, and a labeled contact sheet. Note: SPEC base_commit 3d358812f86c02e5ad405566413108f97ac4e090 is not present in this repo; branch was cut at 14980487 (same base as sibling asset packs).

IMPLEMENTED 2026-08-24 (fb816a97): 9 byte-identical rasters adopted (background hero_disc_night 1254x1254, variant menu_disc_side 900x700, 7 true-alpha layers z 0..40), splash_meta.json + adoption_manifest.json (sha256/bytes/dims/mode/alpha vs Z:/Code/WIZARD @ 6c2f9e725761220c32bc08e81e7550f56f6d8fa0), native/tools/verify_wizard_splash_assets.py, contact_sheet.png. Gate results: verify_wizard_splash_assets exit 0; --corrupt negative control exit 1 (caught corruption as designed); check_legacy_denylist exit 0; git diff --check exit 0; worktree clean after commit.
