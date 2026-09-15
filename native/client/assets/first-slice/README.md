# First-slice art — incomplete

## Accepted nonplayer assets

The Village Palisade art pack now contains 16 static NPC cardinal views in
`npcs-v2/accepted`, eight scenery sprites in `world-scenery/accepted`, four starter
inventory icons in `ui-art/accepted`, and 64 pack-wolf idle/walk frames in
`monsters-v2/accepted`. These folders retain the generation inputs, prompts,
reconstruction evidence, and editable Blender sources where applicable. Their
native outputs have been inspected; this is not acceptance of a complete combat
set or of the village-defense gameplay implementation.

World sprites use 48 pixels per metre and declared canvas anchors. Inventory
icons use their inventory footprint. Preserve these distinct roles when importing.
The new player appearance and remaining combat/boss animation work are in progress.

Web art-generation conversations belong in the existing ChatGPT project
[Pixel Art and Game Dev](https://chatgpt.com/g/g-p-68faa7735964819182de7eb32b19560d-pixel-art-and-game-dev/project).
Nine earlier workflow conversations have been moved there. New generation uses
the built-in imagegen skill while the web service is rate-limited.

## Current starter appearance reference

The user-endorsed exterior is [the saved concept](concept/starter-player-exterior.png),
with its scope recorded in [appearance-reference.json](concept/appearance-reference.json).
Use the image itself for both Blender revisions and appearance painting. Its long
female braid, heavier rope belt, rough asymmetric clothing, and woven footwear
supersede conflicting design assumptions from earlier trials. The existing 128-frame
locomotion milestone below predates this concept and still needs an appearance revision.

## Previous locomotion milestone

`reviewed/manifest.json` contains 128 painted player locomotion frames: male and
female, walk and sprint, eight phases in four cardinal directions. Each PNG has
a 96×96 canvas, [48,80] rig anchor, and 48 pixels per metre at the player plane.
The female has a centered back braid. Both wear untrimmed natural flax linen,
cord belts, and open sandals; the female tunic has the shorter practical cut.

These are agent-reviewed art outputs, **not a finished first-slice pack or an
integrated gameplay change**. Idle, combat, equipment variants, diagonal views,
NPCs, monsters, environment, and native integration are still outstanding.
Do not silently substitute these walk frames for missing combat or idle clips.

The package preserves original generated PNGs, exact native Blender references,
editable Blender milestones, reconstruction reports, checksums, and 2× previews.
The generator received individual reference images, not a ZIP to interpret.
The web prompts requested `background="transparent"` for each generation.
Returned alpha was verified; no undocumented API parameter or RGB key is used.

Reconstruction uses the existing Pixel Respecter at
`Z:/Code/Python/pixel-perfecter`. A fourfold grid candidate must pass its variance
gate. The importer permits only one integer translation per whole cycle, never
per-frame recentering or resizing. It rejects clipped pixels and large silhouette
changes. Packaging additionally checks source/reference/output hashes so stale
candidate files cannot pass under a newer report.

Source mannequin lineage: WIZARD commit
`68affeaa09d84fa69607b8f42421646411c3861c`,
`art_studies/starter-derivatives-v01/paint-v2`. The included female Blender files
add a braid centered at the actual scalp attachment. Original sources were kept.
Locomotion uses the previously retargeted CMU motion-capture data. The data was
obtained from mocap.cs.cmu.edu, whose database was created with funding from NSF
EIA-0196217; BVH conversion by Bruce Hahne.

Local `source`, `candidates`, `review`, `blender-*`, and `guides` folders contain
unfinished/rejected work and are deliberately excluded from the reviewed pack.
The experimental attack renderer is not visually accepted: finger curl and
clothing/body masking still need correction. NPC and several wolf paint passes
changed body silhouettes and must not be published merely because PNGs exist.
