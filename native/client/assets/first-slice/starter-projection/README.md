# Starter player projected appearance

Current review cohorts: `male-v7` and `female-v4`. Earlier directories are preserved experiments or superseded raster/source versions. Only the current cohort's `candidate-binary/manifest.json` should feed native-contract packaging, after independent visual review.

Each sex has 416 native frames: four directions, unarmed and branch-club equipment, four idle phases, eight walk/sprint/hit/death phases and sixteen attack phases. Normal frames are 96x96 with ground anchor [48,80]. Attack/death frames are 128x128 with anchor [64,96] to contain motion. Both retain the same 48px/metre player-plane scale. A larger action canvas does not enlarge the character.

The appearance comes from actual ImageGen paint, projected onto existing animated Blender meshes using the exact painted source poses, camera matrices, visibility/depth checks, surface normals and original vertex correspondence. Male front/back sources use accepted walk-frame72 paint; side sources use registered idle-frame1 paint. Female sources all use her own refined walk-frame72 model and paint. Each sex retains one atlas across every action. The branch material comes from a separately isolated ImageGen wood patch, with original and patch hashes recorded.

Materials use nearest texture sampling and emission to avoid a second lighting pass. The final raster uses Cycles BOX reconstruction, width1.0, without denoising. Raw RGBA renders are preserved; binary exports threshold alpha at128 and zero hidden RGB, without resizing, recentering or fitting the silhouette. Blender geometry supplies the alpha boundary. Source paint alpha controls surface-color confidence only.

The female braid follows the same nape-to-shoulder-to-front curve in every action. Its upper/middle woven cross-section is intentionally enlarged1.55x, tapering to the original tip. Curve control points, radii, bevel depth and a common local-profile hash document this approved 3D change. Other material binding preserves mesh positions and shape keys exactly. Combat sources independently repair underarm coverage; female death also corrects source corpse grounding in3D. Unarmed attacks use actual punch animation with closed fingers, not hidden-weapon club swings.

Attack contact is frame index8 of16, normalized effect phase0.5. First and last frames return to actual idle. Preview GIF timing is for inspection; gameplay uses the runtime's normalized effect phase. Reports carry exact source-frame mapping, source Blender hashes, equipment identities, atlas/weapon hashes, raster filter settings and packed-scene lineage.

Reproduction tools in `native/tools/`:

- `starter_projection_multisource.py`: bind painted material to an explicit source Blender action; `--prepare-only` creates a packed editable scene.
- `starter_projection_raster.py`: raster-only cohort from those packed scenes, with explicit per-action override cohorts. Refuses an existing destination and validates source hashes.
- `starter_projection_package.py`: binary-alpha native frame export, manifest and inspection contacts.
- `starter_projection_final_audit.py`: frame/hash/alpha/canvas/anchor/atlas/braid/timing checks and idle-to-attack endpoint comparisons.

Projection is not a replacement for source quality: source paint contains baked directional shading, and surfaces not confidently observed use paint-derived semantic color. Native-scale visual review and in-game review remain separate acceptance gates.
