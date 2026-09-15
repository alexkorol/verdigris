# Scenery paint prompt set

Mode: built-in `image_gen.imagegen`, one asset per call, with the corresponding
nearest-neighbor enlarged native Blender PNG as `referenced_image_paths`.
The prompts below preserve the operative instructions used for the accepted
paint passes. The hut pass requested fourfold pixels; the other passes requested
fivefold pixels. The service returned 1254-square PNGs and Pixel Respecter found
the fivefold grid used in the accepted native reconstructions. No image was
resized to make its painted silhouette fit a preferred dimension.

## Shared contract

Repaint this exact low-resolution Blender reference as gritty realistic,
pre-rendered Bronze Age action-RPG simulated pixel art. Preserve its silhouette,
camera, position, scale, generous transparent margins, and complete canvas.
The logical canvas is 256 × 256 pixels. Material details resolve into uniform
five-by-five image-pixel square clusters, like nearest-neighbor enlargement;
no finer detail, soft antialiasing, photographic rendering, or cartoon styling.
Use warm upper-left light and muted natural materials. Generate a genuinely
transparent RGBA PNG: alpha zero outside the object and inside genuine gaps.
No checkerboard, solid white/black background, border, label, unrelated object,
ground, or shadow beyond the reference silhouette. Do not enlarge, crop,
recenter, or change the object angle. Preserve true background transparency
in the final output alpha channel.

## Hut

Weathered coarse golden-brown reed thatch with dark bundled courses and subtly
irregular lower ends; warm taupe wattle-and-daub walls with a timber doorway
frame, and small areas of worn daub exposing woven twigs. Tiny textures resolve
as chunky restrained pixel clusters, earthy Diablo II pre-rendered realism,
without cartoon outlines. Preserve doorway darkness and the source lighting
from upper left. Preserve the exact hut silhouette and doorway. Keep the
doorway opening transparent wherever the source is transparent. The original
call explicitly requested a 1024-square canvas, 256-square logical grid and
fourfold intended pixels; the actual returned fivefold grid is recorded in
`reports/hut.json`.

## Square well

Keep the square rim, solid corners, wood posts, crossbar, and rope, with the
exact silhouette and placement low in the canvas. Warm-gray rough fieldstone
with a few moss joints, weathered dark oak with splits, tan hemp rope, and a
dark well interior. No holes in the four solid stone corners. No added bucket,
roof, ground, scene, or shadow beyond the silhouette. The corrected Blender
reference is authoritative; an earlier generation with masonry corner holes
was rejected and is not included in this package.

## Intact palisade

Preserve thirteen vertical timber stakes and their exact silhouette, position,
camera, and scale. Weathered dark oak logs with splits, knots, stripped bark,
lighter pointed heartwood tops, and dark fine gaps. Retain the straight
horizontal front projection and original footprint. No added grass or ground.

## Breached palisade

Preserve four intact stakes on the left, four on the right, and broken stumps
in the central breach. Rough splintered stump tops in the breach; aged brown
oak stakes, lighter cut heartwood points, deep grain and knots. Preserve
all thirteen stake positions, the open breach, footprint and camera. Keep
background and breach genuinely transparent.

## Fieldstone cluster

Slate-gray weathered natural fieldstones with warm highlight facets, subtle
moss in crevices, no crystalline magic or cartoon shape. Do not change the
individual stone silhouettes. Retain the original cluster, exact size and
low placement in the spacious canvas.

## Tree

Maintain the exact trunk and canopy silhouette, scale, camera and placement.
Paint the plain canopy surfaces as realistic deciduous oak leaf clusters with
shadowed branching structure and irregular small clumps within the source
silhouette. Weathered ridged brown bark on the trunk. Muted warm olive,
deep woodland green and brown palette. Preserve transparency around the tree
and under the canopy. No added tree, hanging object, ground, or external shadow.

## Worn-earth patch

Flat ground decal of worn brown earth with scattered tiny gravel, dried straw
and sparse short olive grass around the edge. Preserve the original thin
flat elliptical ground plane, its exact size, location, angle and footprint.
Flat texture without thickness: no floating island or pile of rocks.
Subdued earth browns, dusty olive grass, low contrast suitable under characters.
True alpha zero outside the irregular patch. It is a decal, not a seamless tile.

## Reproduction and review

`first_slice_scenery_blender.py` produces the editable references for the well,
palisades, stones and ground decal at 48 pixels per metre. Hut and tree reuse
the existing calibrated village Blender models retained in `blender/`.
`first_slice_scenery_import.py` requires genuine source alpha, a passing expected
pixel-grid candidate, silhouette IoU of at least 0.75, and zero clipping. It
allows only integer translation, never silhouette resizing or color-key alpha.
`first_slice_scenery_package.py` verifies hashes and packages the explicitly
reviewed sprites. The first shrub attempt failed silhouette comparison and was
replaced with a targeted geometry-locked paint pass. The retry is included.

Each accepted report contains source, reference and output hashes, actual grid
fit, integer registration, silhouette overlap, and visual-review status. These
are reviewed art assets; a playable scene and in-game acceptance remain the
integrator's separate responsibility.

## Woodland shrub — exact accepted retry prompt

Edit this exact image with minimal changes: a subtle material paint pass over the existing small woodland shrub. Geometry is LOCKED: retain every leaf position, gaps, exact outer silhouette, size and pixelated boundaries. No leaf extensions, no additional leaves, no thickening stems, no increased height or width. The entire image is 1280x1280 containing a 256x256 logical grid enlarged exactly5x nearest neighbor. The shrub occupies logical x96..147 y169..204, only51x35 logical pixels, and must remain this small. Retain exact5x5 image-pixel square grid. Repaint existing pixels only with muted olive greens and earthy twig brown, realistic D2 pre-rendered material shading, warm upper-left highlights. Do not make a new plant, no rearrangement or recentering. Background and gaps must be genuinely transparent PNG alpha zero, not black pixels, white pixels or checkerboard. No ground, shadows, border or text. Keep same sparse leaf construction and fullness. Return true transparent RGBA, same canvas framing and a strictly51x35-logical-pixel shrub silhouette, uniform5x5 pixel clusters, minimal material-only changes.
