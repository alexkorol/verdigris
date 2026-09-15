# Village attackers

Two distinct human attackers replace the old raider fallback in the Village
Palisade prologue. The lean invader wears charcoal rough cloth, short black hair
and no beard. The broader breaker wears russet hide, longer brown hair, a jaw
beard, and carries a heavier branch. Each has 20 complete action/direction clips:
one idle, four walk, six strike, two hit and four death frames per cardinal view.

The pipeline retains the reviewed anatomical MakeHuman rigs, CMU walking and
Quaternius combat motion. Deterministic Blender variants modify the whole actor
and its attached clothing/weapon together. There is one continuous cloth shell;
no extra intersecting armor layer. The same geometry creates the four native
paint guides and the animation frames.

The built-in imagegen tool painted one appearance atlas per enemy. The exact
prompts and unmodified generated RGBA images are included. Generated atlas scale
drift is measured and registered once to the four source poses. Pixel Respecter
reconstructs dominant colors and binary alpha on the fixed native grid. Depth
visibility and surface normals bind the paint to the existing animated vertices.
Blender supplies every final silhouette, pose, hand grip, pivot and native pixel.
No animation frame is image-generated, resized, trimmed, or fitted to its ink.

All output uses 48 pixels/metre, 96px or 128px padded canvases, and the shared
ground pivot [48,80] or [64,96]. Box1.0 rendering and alpha threshold128 match the
player pipeline. `manifest.json` contains all source/appearance hashes and clip
timing. Accepted pixels were inspected in four-view native contact sheets;
duplicate settle frames were replaced with collapse/recovery phases before
installation. Native renderer and package verification are recorded in the
repository handoff. Technical review does not claim the owner's visual approval.

Tools: `render_village_attackers.py`, `transfer_village_attackers.py`, and
`package_village_attackers.py` in `native/tools`. Packed editable scenes are
retained in each identity's `scenes/` and archived with the source milestone.
