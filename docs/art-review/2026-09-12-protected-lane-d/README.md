# Protected art review — 2026-09-12

Six requested visual studies were generated, plus one targeted panel-alpha retry: **7 candidates, cap 8**. Every original is retained. These are proposals; nothing is integrated into production. No gameplay, renderer, source asset, lore, or Burning Touch work was performed.

Open `index.html` for the gallery, or `native-size-review.png` for the fixed 1100x660 review sheet. The sheet was viewed at its native dimensions with the image viewer, not accepted from numeric checks alone. It preserves whole images and uses display resizing only.

## Visual triage

| Original | Status | Native-size finding |
| --- | --- | --- |
| `originals/bowl-01-bronze.png` | Proposal | At 96x96 and 64x64 the empty basin, continuous rim and pedestal remain clear. Bronze is fairly bright; material polish remains an owner choice. |
| `originals/bowl-02-clay.png` | Proposal | At 96x96 and 64x64 the simple clay basin and full lip remain clear. Clay is a proposed material, not an item or lore ruling. |
| `originals/tree-01-dense.png` | Proposal | At 128x256 a slender trunk and vertical crown remain readable. Denser branching looks more natural than study 02. |
| `originals/tree-02-airy.png` | Needs visual review | At 128x256 the pale trunk and branch gaps read clearly, but round foliage clusters suggest deliberate pruning. Species and final silhouette remain unresolved. |
| `originals/ui-01-button.png` | Needs visual review | At 138x55 the dark field, rails and stepped corners retain the FrameKit family. Generated outer padding reduces its visible height relative to the source; it is not a pixel-exact replacement. |
| `originals/ui-02-panel.png` | Reject for runtime; visual proposal only | At 251x250 the family is preserved, but the exterior checkerboard is baked RGB, not alpha. |
| `originals/ui-02-panel-alpha-retry.png` | Reject for runtime | A targeted background-only correction still returned RGB checkerboard and slightly changed texture. Preserved for audit; no further generation. |

The five RGBA outputs have transparent exterior corners, but sampled opaque object interiors have alpha 252–253 rather than 255. Originals are untouched. Their full alpha edges and background behavior need a production preparation pass only if selected. No candidate has production approval.

Review sizes are explicit working proposals, not extracted runtime dimensions: bowls 64/96 square; trees 128x256. A nominal 64 px adult would make the tree canvas four adult heights; actual world scale and camera matching require a later approved in-game check. UI sizes match the inspected source raster canvases (button 138x55; panel 251x250). No gameplay claim is made, so the gameplay goal harness was not run for this isolated artifact task.

## Viewed references and exact templates

- `Z:/Code/WIZARD/tools/rpg_inventory/assets/bowl_bronze_offering.png`: viewed accepted local bronze bowl. Used for material/light quality, not copied geometry. Local reference copy: `references/bowl_bronze_offering.png`.
- `Z:/Code/WIZARD/tools/rpg_inventory/core/PROMPT.txt`: inspected established item-rendering prompt; used its three-quarter view, upper-left key, cool rim, clean material, plain-background rules. The source was not changed.
- `Z:/Code/WIZARD/tools/gui_framekit/assets/concepts/controls-and-panels.png`: viewed actual family sheet. Source corner geometry, dark center, restrained bronze rails and pale-stone panels informed the two UI requests. Local copy: `references/controls-and-panels.png`.
- `Z:/Code/WIZARD/tools/gui_framekit/game/assets/btn_primary.png`: exact image edit input, copied unchanged to `references/btn_primary.png`.
- `Z:/Code/WIZARD/tools/gui_framekit/game/assets/panel_plain.png`: exact image edit input, copied unchanged to `references/panel_plain.png`.
- `docs/reference/25d-overhaul/assets/tree.png`: inspected historical local reference. Its squat broad crown was not reused; the explicit tall/slender request and Blender massing guided these studies. It was not supplied to generation.
- [Spriters Resource Diablo II archive](https://www.spriters-resource.com/pc_computer/diablo2diablo2lordofdestruction/): searched and direct access attempted on 2026-09-12. Direct fetch returned **403 Forbidden**. No actual sprite sheet from that website was visually inspected or downloaded; no generation claims rely on it.

The WIZARD repository and RPG Inventory AGENTS guidance, Verdigris canonical AGENTS, product constitution, orchestration protocol and D-113 were inspected. The current user-authorized plain study batch takes precedence over older bulk-roster/loadout workflow. No edits were made to WIZARD or any shared runtime path.

## Reproducible guides and provenance

`guides/build_guides.py` was run successfully with local Blender 2.91. It produces four saved scenes and RGBA guide renders:

- `guides/bowl-01-guide.blend` and `.png`: continuous revolved basin with integral foot.
- `guides/bowl-02-guide.blend` and `.png`: continuous simple basin with flat underside.
- `guides/tree-01-guide.blend` and `.png`: narrow upright trunk and denser tall crown masses.
- `guides/tree-02-guide.blend` and `.png`: narrow trunk with more separated upper masses.

Each guide was viewed before being supplied to generation. The bowls use an orthographic camera at (4,-7,5), target (0,0,0.5/0.4), ortho scale 3.5; trees use (7,-12,8), target (0,0,3), scale 7.3. These are exploratory poses, not validated production-camera calibration.

Mode: built-in image generation, seven independent calls, no API/CLI fallback. Exact prompts are in `prompts.json`, `ui-prompts.json`, and `panel-retry-prompt.json`. Provider original paths and copied destinations are in `generation-log.json`. Hashes, dimensions, formats, sampled alpha, triage and integration status are in `image-metadata.json`.

`build-review.ps1` was run successfully to create the native review sheet and metadata; it resizes only within the review sheet and never alters originals. Model configuration: parent-inherited GPT-6/Astra lane; the precise reasoning-effort setting was not exposed to this worker and no override was requested.

Git preflight found an existing journal modification and a branch 15 commits behind origin after a concurrent remote-ref update error. This explicit isolated lane added only this new directory; it did not switch branches, commit, push or modify the journal. The coordinating agent owns final integration/commit decisions.
