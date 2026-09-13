# Tree 01: model-stage review, unapproved

Owner feedback: the button treatment passes visual review but is not promoted
to production. Both panels fail real-alpha requirements; both trees fail the
pixel-art/structural direction; both bowls are too perfect and ornate for
beginner equipment. Bowl revisions should explore handmade clay, wood or
stone, not bronze/brass. The supplied webchat's conflicting button criticism
does not supersede the owner's direct ruling.

This isolated checkpoint uses Tree 01 as a provisional working choice from
the supplied webchat. It does not select an approved species or tree recipe.
No new appearance candidate, bowl, UI reroll or runtime change is included.

## Review

- `model-comparison.png`: original recovered Blender model, revised model,
  revised branches with foliage hidden, and the original appearance candidate.
- `pixel-comparison.png`: logical original and revised guides with the existing
  player, plus exact 2x enlargement.
- `revised-logical.png`: direct 136x272 render normalized to 16 gray levels
  and binary alpha, with no resize.
- `preappearance-reference-4x.png`: exact 4x nearest-neighbor enlargement,
  544x1088 RGBA. This is a proposed input, not an image already consumed by
  an appearance model. No appearance generation has occurred.
- `tree-01-revised.blend`: editable model checkpoint.

The original scene and image in `baseline/` are byte-identical copies from
the recovered sprint art at commit 443fb250314f7be3f3fef3aff86c2ded5cbe3eac.
Original trunk/main branch meshes, camera transform, orthographic scale,
lighting and framing are retained. Secondary tapering forks and leaf planes
replace the solid ellipsoid foliage. Unequal group weights expose selected
gaps. Seed 301 fixes the new subordinate structure for subsequent edits.
This modifies the existing scene; it does not adopt MTree or The Grove.

The retained straight trunk and regular primary-branch arrangement remain
limitations. The grayscale model is a structural reference, not finished
pixel art. Model approval is required before any appearance pass; afterward
the result still requires Pixel Respecter normalization and game-scale review.

## Scale evidence and limitation

The existing `hero_se.png` contains 63 visible rows on an 80x96 canvas.
The logical canvas is calculated from the unchanged camera's projected
height of an assumed 1.8-model-unit adult, giving 136x272 rather than an
arbitrary generator-sized canvas. This adult height is provisional: no
canonical player-plane calibration was found in the inspected local guides.
The comparison therefore does not claim compliance with an unverified
player-plane standard or acceptance in the live native game.

## Reproduce and verification

Run Blender 2.91 in background with `--python revise_model.py`, then run
`python build_comparison.py` from this directory (Pillow and NumPy required).
The model script resolves all paths relative to itself.

Both scripts completed successfully. Both final comparison sheets were
viewed at their native dimensions. The comparison script checks every 4x4
reference block against its logical source pixel, the roundtrip, and binary
alpha. Camera equality is asserted before rendering the revised model.
Hashes and explicit null generation-input status are in the provenance files.
No native gameplay or legacy web tests apply to these isolated art studies.

This branch is based on f564ab681c00a27cc944ca86faf621f9d1830ba3. It does
not include the unpushed recovered sprint implementation or change its
packaged source relationship. Packaged settings save/relaunch acceptance
remains paused after the owner's Computer Use stop.
