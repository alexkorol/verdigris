# TASK-0179 — Verdigris splash/menu asset pack — REPORT

Lane: ox-alpha-pc-w6 · Worktree: Z:\Code\.worktrees\verdigris\worker-t0179 · Branch: ox/TASK-0179
Claim: 943be01a · Implementation: fb816a97 · State: REVIEW_REQUESTED

## Outcome

Branded Verdigris splash/menu art pack adopted byte-identically from the real WIZARD
`verdigris_splash` module (wizard.verdigris-splash v2.6.0) into
`native/client/assets/wizard/splash/**`, with full provenance, composition metadata, a
deterministic verifier with negative control, and a labeled contact sheet.

## References

- SPEC: orchestration/tasks/TASK-0179-verdigris-splash-asset-pack/SPEC.md (base_commit P0)
- Source (read-only): Z:\Code\WIZARD\tools\verdigris_splash\assets\{textures,world}
- Provenance ref (read-only): owner-demo-runway native/client/assets/wizard/source_manifest.json
  (TASK-0166) — contains no verdigris_splash family, so sha256/bytes/dims/mode/alpha were
  computed directly from source files at adoption time against WIZARD commit
  6c2f9e725761220c32bc08e81e7550f56f6d8fa0.

## Selection rule (9 of ~74 rasters; rest documented as near-duplicates/tooling)

- background: `background/hero_disc_night.png` (1254×1254 RGB) — night-lit disc world, teal
  coastlines + orange settlement glow (from celestial_world_illumination_concept_image2.png).
- variant: `background/menu_disc_side.png` (900×700 RGBA opaque) — floating disc-world side
  profile render for the main-menu backdrop (qa_detailed/detailed_side.png).
- layers (all true-alpha RGBA, z 0..40): sky_milkyway_band (2048×512), glow_eclipse_corona
  (1024²), moon_blood_moon (256²), planet_top_disc (2048², alpha outside rim),
  planet_underside_disc (2048²), fx_cloud_puff (512×320), fx_ember_glow (256²).
- Excluded as near-duplicates/non-splash: 16 illumination tiles + 36 tmp texture tiles (tile
  derivatives), 16-bit height/depth maps, STL/GLB meshes, prompt/build reports,
  debug_marked_top QA render, 22 MB 4K detail texture (topographic disc carries the same read).
- title: no raster title art exists in the source module (HTML/CSS wordmark `h1#title` +
  `.wordmark-echo`); documented in splash_meta.json with integration guidance. Nothing fabricated.

## Files delivered

- native/client/assets/wizard/splash/{background,layers}/*.png (9, byte-identical copies)
- native/client/assets/wizard/splash/splash_meta.json (roles, z-order, canvas hint, title note)
- native/client/assets/wizard/splash/adoption_manifest.json (per-file sourcePath/sha256/bytes/
  dimensions/mode/alpha-range/role/zOrder + source commit + selection rule)
- native/tools/verify_wizard_splash_assets.py (deterministic verifier + --corrupt control)
- orchestration/tasks/TASK-0179-verdigris-splash-asset-pack/contact_sheet.png (9 labeled cells,
  alpha checkerboard, z-order sorted)

## Commands and exit codes

- `python native/tools/verify_wizard_splash_assets.py` → exit 0
  ("OK (9 assets, roles+alpha+sha256 all pass)")
- `python native/tools/verify_wizard_splash_assets.py --corrupt` → exit 1 (negative control:
  flipped a byte of hero_disc_night.png in a temp copy; verifier caught sha256 mismatch +
  unreadable raster, as designed)
- `python native/tools/check_legacy_denylist.py` → exit 0 ("native legacy denylist: PASS")
- `git diff --check` → exit 0; `git status --short` after final commit → empty (clean worktree)

## Evidence

- Contact sheet (visual): contact_sheet.png in this task folder — recognizably Verdigris
  (disc world hero, floating-disc silhouette, glowing abyss underside).
- Provenance: adoption_manifest.json sha256 values reproduce from
  Z:/Code/WIZARD/tools/verdigris_splash sources; verifier re-checks them on every run.

## Residual gaps

- SPEC base_commit 3d358812f86c02e5ad405566413108f97ac4e090 does not exist in this repo
  (possibly from a pre-sweep orchestration repo); branch was actually cut at
  1498048722d1d3a54ae8d1d48677b87e503af241 — recorded in STATUS.md.
- native/client/assets/manifest.json (asset registry) was not touched — outside owned_paths;
  registration belongs to the integration task.
- No 16:9 pre-composited splash frame; composition is left to the renderer via
  splash_meta.json z-orders (canvas hint 1920×1080, cover fit).

## Successor note (TASK-0183 — native splash/menu integration)

Consume `native/client/assets/wizard/splash/` via `adoption_manifest.json` (sha256-verified
load) and compose with `splash_meta.json`: draw layers in ascending zOrder (sky_milkyway_band 0
→ glow_eclipse_corona 10 → moon_blood_moon 15 → planet_top_disc 20 / planet_underside_disc 21 →
fx_cloud_puff 30 → fx_ember_glow 40), then hero_disc_night (z 100) as the full-frame splash, or
menu_disc_side (z 101) as the alternate menu backdrop; layers are RGBA straight-alpha PNGs, the
milkyway band peaks at alpha 150 (additive-friendly). Render the VERDIGRIS wordmark as engine
text — there is deliberately no raster title. Run
`python native/tools/verify_wizard_splash_assets.py` as an integration precondition.
