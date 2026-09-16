# Verified underarm visibility repair

This preserves the first mask-repaired sources. Current club attacks are in
`../contact-v4-repaired/`, punches in `../contact-v5-fist/`, and the grounded
female death in `../contact-v4-repaired/female-death/`. Male death and both hit
reactions below remain current. Earlier sources remain for reproducibility:

| Folder | Editable scene | Samples | Native frame / anchor |
|---|---|---:|---|
| male, female | club-attack-transition.blend | 16 | 128×128 / (64,96) |
| male-hit, female-hit | club-hit.blend | 8 | 96×96 / (48,80) |
| male-death, female-death | club-death.blend | 8 | 128×128 / (64,96) |

Every scene uses samples at frame `1 + 4*i`, four cardinal views and 48 pixels per metre. Attack includes grounded idle entry/recovery from transition-v2. Native PNG references, 384×384 inspections, review sheets, GIFs and frame manifests accompany each scene. The 50 ms/frame preview timing is configurable by the game importer.

The only source mutation is restoring existing side-torso vertices to the body's visible-skin vertex group. No body coordinates, clothing surfaces, skeletal poses or camera transforms were edited. Restored faces intersecting the tunic in any actual sample are excluded from restoration, preserving coverage over the front chest. These exclusions are derived independently for each action; the restoration method is shared.

| Character/action | Restored vertices | Covered vertices excluded | Maximum restored-face/tunic intersections |
|---|---:|---:|---:|
| Male attack | 181 | 5 | 0 |
| Male hit | 186 | 0 | 0 |
| Male death | 176 | 10 | 0 |
| Female attack | 239 | 93 | 0 |
| Female hit | 252 | 80 | 0 |
| Female death | 228 | 104 | 0 |

All 256 native directional frames pass canvas bounds. Four-direction native and high-resolution torso inspection found no new skin protrusion through the tunic. `skin-cloth-intersections.json` contains the per-sample BVH check; this checks newly restored faces against clothing, not every possible body or hair collision. `manifest.json` records actual changes against previous references rather than requiring the previous missing-skin pixels to remain unchanged.

The dark crescent under the raised right shoulder in some left views remains even with the entire clothing visibility mask disabled. An emission-material diagnostic renders this as continuous skin. It is a shaded deformation crease, not absent skin or false transparency; the two diagnostic images in `male/` demonstrate this. The minimal mask repair does not alter that deformation or the lighting.

These are verified source repairs for projection, not finished painted game sprites. Original sources and transition-v2 remain intact. No new image generation was needed.

Rebuild: run `render_starter_combat_skin_repair.py` in background Blender with `-- male` or `-- female`; append `hit` or `death` for those clips. Run `render_starter_combat_skin_qa.py` with matching arguments, then `render_starter_combat_skin_contacts.py` with regular Python.
