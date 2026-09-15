# First-slice art — incomplete

## Owner review hold — 2026-09-15

The owner rejected the current monster, hut and tree direction and the gate/portal
visible in the test scene. Earlier `accepted` folder names record technical/lane
review only; they do not establish owner approval. Do not publish these as the
finished first-slice art. The visible gate is inherited scenery, not an approved
prologue design. Preserve existing outputs for reuse; stop generation expansion.

The owner has reduced scope to basic views and short animations sufficient for
the Village Palisade slice. Defer separate sprint sets, extra equipment families,
diagonals and long combat cycles. Settle one scene's visual direction with the
approved starter character and a few environmental/monster guides before more
generation. The prologue specification says packs and a square/well boss; it
does not establish wolves as the approved creature design.

## Accepted nonplayer assets

The Village Palisade art pack now contains 16 static NPC cardinal views in
`npcs-v2/accepted`, eight scenery sprites in `world-scenery/accepted`, four starter
inventory icons in `ui-art/accepted`, and 176 pack-wolf/boss animation frames in
`monsters-v2/accepted`: pack idle/walk/attack/hit and boss idle/walk/hit in four
cardinal directions. Death and boss attack remain in production. These folders retain the generation inputs, prompts,
reconstruction evidence, and editable Blender sources where applicable. Their
native outputs have been inspected; this is not acceptance of a complete combat
set or of the village-defense gameplay implementation.

World sprites use 48 pixels per metre and declared canvas anchors. Inventory
icons use their inventory footprint. Preserve these distinct roles when importing.
The new player appearance and remaining combat animation work are in progress.

`runtime/manifest.tsv` installs those 204 nonplayer PNGs byte-for-byte, with
conversion manifests in `runtime-imports/`. The native renderer uses complete
actor-family gates, so these incomplete monster action sets remain available
for inspection and do not silently replace missing combat states. Generic trees
and huts now load in the normal local launcher; distinct old landmarks without
matching accepted replacements remain unchanged.

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

## New exterior: partial walk milestone

`starter-v2/accepted/manifest.json` contains 24 inspected frames: male front and
back walk, plus female front walk, with eight phases per clip. These use the
approved coarse-fabric exterior and retain exact imagegen calls, original RGBA
outputs, uploaded guides, native Blender references, editable scenes, and hashes.
They are incomplete and must not activate a player family in gameplay.

The canvas is 96×96 with anchor [48,80] at 48px/metre. Pixel Respecter reconstructs
the measured fourfold lattice without resizing the character. One shared integer
translation is allowed per sheet; the female sheet has a recorded shared row
offset. No individual pose is recentered, and no foreground is discarded.
The inspected row transition and wraparound preserve the source motion.
Registration scores use Blender alpha≥128, while the separately labeled final
silhouette scores include its antialiased edge coverage (alpha>0).

## Previous locomotion milestone

`reviewed/manifest.json` contains 128 painted player locomotion frames: male and
female, walk and sprint, eight phases in four cardinal directions. Each PNG has
a 96×96 canvas, [48,80] rig anchor, and 48 pixels per metre at the player plane.
The female has a centered back braid. Both wear untrimmed natural flax linen,
cord belts, and open sandals; the female tunic has the shorter practical cut.

These older outputs are **not a finished first-slice pack or the current player
appearance**. Their incomplete action coverage must not be mixed with newer art.
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
