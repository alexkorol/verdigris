# TASK-0166 REPORT — WIZARD source and provenance manifest

## Executive summary

Delivered `native/client/assets/wizard/source_manifest.json`: a hash-verified,
dimension-annotated provenance manifest covering 352 raster artifacts across
the 8 WIZARD families required by the Owner Demo (framekit, orbs, rpg_inventory,
splash, plus 4 code/reference modules), pinned to WIZARD commit
`66a5d9ff6810e886c1bd08cbeaaf83cabf92aae9`. Delivered
`native/tools/verify_wizard_source_manifest.py`, a deterministic verifier with a
failing negative control. All acceptance commands pass. Status: REVIEW_REQUESTED.

## References inspected

- `Z:\Code\WIZARD\modules.json` — module registry (17 modules, versions, verdigrisTargets)
- `WIZARD/tools/gui_framekit` — 5 real generated raster slices (orb-vitality/-mana/-essence sprites, panel.png, slot.png) + 16 evidence captures
- `WIZARD/tools/wizard_orbs` — 3 rendered orb screenshots; source texture pack at `WIZARD/tmp/orbs-original/extracted` (art/empty/mask/norm/pack/stone)
- `WIZARD/tools/rpg_inventory/assets` — 248 canonical Bronze Age item PNGs (weapons, armor, curios, trophies; e.g. macuahuitl_bone, greaves_bronze, girdle_bronzeplate); item ids/names/footprints in `assets_staging/*/manifest.json` (e.g. manual-web-wave-01: `helmet_light_riverhide`, grid "2x2", canvas "S")
- `WIZARD/tools/verdigris_splash` — 74 splash art files
- `WIZARD/tools/{geometric_skilltree,arcane_lattice,cartographer,rp_account_creator}` — code/reference modules (no raster required for this packet); registry records their launch/readme paths for successors TASK-0191/0193/0195/0197

## Current discrepancy addressed

No machine-verifiable provenance map existed linking WIZARD sources to the
native asset adoption lanes; successor asset packets (0167/0168/0169) and
adapter/model tasks (0180–0182, 0191, 0193, 0195) had no authoritative source
inventory or integrity check.

## Concrete result

- `native/client/assets/wizard/source_manifest.json` (103,892 bytes):
  - `sourceCommit` provenance, full `moduleRegistry` (id/version/targets/launch/readme)
  - per-family artifact entries: `sourcePath`, `sha256`, `bytes`, `dimensions` (w/h/mode)
  - counts: framekit 21, orbs 9, rpg_inventory 248, splash 74; 4 reference-only module families
- `native/tools/verify_wizard_source_manifest.py`: checks existence, sha256,
  size, raster dimensions, family minimums; `--corrupt` injects a hash mismatch
  (negative control).

## Commands and exit codes

| Command | Result |
|---|---|
| `python native/tools/verify_wizard_source_manifest.py` | `VERIFY OK: 352 artifacts across 8 families, WIZARD commit 66a5d9ff...` exit 0 |
| `python native/tools/verify_wizard_source_manifest.py --corrupt` | `VERIFY FAIL (1 problems...) hash mismatch` exit 1 (correct negative control) |
| `python native/tools/check_legacy_denylist.py` | `native legacy denylist: PASS` exit 0 |
| `git diff --check` | clean, exit 0 |

## Visual evidence

`orchestration/tasks/TASK-0166-wizard-source-manifest/contact_sheet.png`
(1024×512, 27 tiles) — visually confirms real raster art per family:
framekit orb sprites + gold panel/slot textures; wizard orb renders (red/blue
globes with figure pedestals) plus source texture pack; RPG Inventory Bronze
Age items (macuahuitl, war clubs, greaves, girdle, hides, reagent bowl); splash
planet/terrain art. No blank/broken tiles.

## Paths changed (all within owned_paths)

- `native/client/assets/wizard/source_manifest.json` (new)
- `native/tools/verify_wizard_source_manifest.py` (new)
- `orchestration/tasks/TASK-0166-wizard-source-manifest/{STATUS.md,REPORT.md,contact_sheet.png}` (new)

Forbidden paths untouched (`native/client/main.cpp`, `native/src/**`, etc.).
No gameplay change ⇒ `npm run playtest` not applicable to this packet (no
integrated-state behavior claim made).

## Deviations

None. Fallback not needed. Note: `base_commit` in SPEC is 3d358812; runway
branch head is 491f8f84 (runway seed commit on top of 3d358812) — packet
paths are unaffected.

## Residual gaps / risks

- rpg_inventory `assets_staging` wave manifests are referenced by path but not
  embedded (they contain absolute local paths); TASK-0169 should derive its
  stable ids/footprints from them.
- health_globe module has no raster art (HTML-only) — orb raster truth lives in
  wizard_orbs + tmp/orbs-original; flagged for TASK-0168.

## Successors released (dependency-aware)

- TASK-0167 (Framekit raster slice pack) — sources now pinned
- TASK-0168 (orb raster pack) — source textures + renders pinned
- TASK-0169 (RPG Inventory item-art pack) — 248 canonical assets pinned
- TASK-0191 / TASK-0193 / TASK-0195 (Cartographer, skill-tree, spell-lattice) —
  module registry + commit provenance available
