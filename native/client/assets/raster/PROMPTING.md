# Reference-driven pixel art production

This standard records results observed in the September 9, 2026 run, including
the owner's corrections. It describes this tool's observed behavior rather
than making universal claims about a model version. The built-in image tool
does not expose a model selector in this session.

The first draft overgeneralized our sheet failures. Subsequent primary-source
research found both useful single-character motion sheets and failed grids.
Read `RESEARCH.md`: use the demonstrated workflow appropriate to the asset,
then judge the actual result. Individual frames are a fallback and a focused
control method, not a universal limitation of Image 2.5.

## What the attempts showed

| Attempt | Observed result | Production rule |
|---|---|---|
| Sixteen actor views in a 4x4 sheet | Wrong or near-duplicate back views; feet crossed cell boundaries | Study one character/direction; use isolated frames when needed; assemble the production atlas in code |
| Four walk frames across a directional sheet | Leading legs repeated; passing poses duplicated | Specify one temporal transition using an accepted reference; review a played sequence |
| "Pixel art" with textured material language | Detailed raster illustration instead of deliberate low-resolution clusters | Set a logical body height and palette; inspect reconstructed pixels at native and gameplay scales |
| Exact dimensions and tiled texture requests | Grid and dimensions drifted; seamlessness was not established | Crop/pack with measured bounds; verify any intended repeat at actual scale |
| Editing a sheet that showed a transparency pattern | The pattern became opaque RGB background | Prefer an isolated alpha-bearing reference; request transparency, then test the actual alpha channel |
| Name-derived patina styling | Green spread to armor, props and architectural accents | Describe materials directly: bronze, linen, leather, wood, clay, bone and stone |
| Reference edits of clothing and bodies | Identity was more stable than pose and layout | Keep accepted identity references; independently verify direction and movement |
| Referenced SE walk as four separate images | Opposite contact repeated the same leg in three candidates; a focused correction naming screen-left/right foot positions finally changed the silhouette | Use visible pose differences to resolve anatomical ambiguity; retain the identity reference and reject repeated phases |
| Four reconstructed SE walk candidates at a shared pivot | Distinct contact/passing poses survive at native size; small color/body shifts remain | Keep preview acceptance separate from actual game motion acceptance |
| Separate back-left wight and artisan edits | Missing direction slots filled; artisan brightness and staff handedness still need review | Fix the direction with a targeted reference edit, then review other properties across the full set |

## Workflow

Before generating, choose the closest published example in `RESEARCH.md`.
Record its original URL, the supplied reference images, the published prompt
or available excerpt, and the changes made for our asset. A new prompt should
answer a demonstrated gap in that recipe; do not replace it with an invented
general-purpose brief merely because one is easy to write.

Match the recipe to the task, including its level of detail. The published
one-line combat study explores poses; Kiki's longer prompt specifies a planned
action. There is no evidence here for a universal short-prompt advantage.
Read the linked original before adapting it. Preserve its motion instructions;
separate its image request from file assembly and checks performed by Codex.

| Need | Starting evidence | Preserve when adapting |
|---|---|---|
| Small idle motion | Kiki's reference-led idle sheet | Fixed feet, camera, scale and baseline; modest breathing/blink; shared margins |
| First combat pose study | Practical_Low29's short 4x4 prompt | One reference character and simple motion request; independently audit anticipation, hand identity and recovery |
| A planned combat cycle | Kiki's combat prompt and second character result | Explicit preparation/contact/recovery beats, full weapon clearance and variable frame holds; preserve our adult proportions |
| A failed sheet or one bad phase | Noel's separate images; Flixly's focused next-frame edit | Accepted identity anchor plus a clearly assigned transition reference; one physical change |
| Material-rich UI | 12ui's comparison prompts | Assign each reference an explicit job: requirements, composition or contrast |
| Cutscene planning | Fun_Walk_4965's pose sheet and H3 workflow | Separate still-image generation from motion synthesis; do not import numbered storyboard panels as runtime sprites |

For a reused recipe, first make a small reviewable example. Keep source quality,
usable animation, and runtime readiness as separate decisions. Our repeated
supporting-leg failures demonstrate why increasing the sheet size or adding
more prose is not sufficient evidence of progress. Preserve the good poses,
repair a specific bad transition, and consult an ordered motion reference when
the same ambiguity recurs. This last choice is our production adaptation,
not a published claim that Image 2.5 follows D2 motion references reliably.

1. Establish one isolated anchor: one actor, neutral ready pose, one camera
   direction. For a human, begin around 64 logical pixels from crown to sole,
   with grounded adult proportions and about 24–32 purposeful colors.
2. Inspect the source. Reject wrong anatomy, baked backgrounds, full-detail
   illustration, excessive texture, green palette bias, and an incorrect view.
3. Reconstruct with the actual Pixel Respecter API. Use measured pixel pitch,
   limited colors, crisp alpha, nearest-neighbor only, and recorded tool/source
   hashes. Normalization must preserve common scale and a stable feet pivot.
4. Inspect the native output and an integer enlargement. Accept this isolated
   image as the identity anchor only when silhouette and material reads survive.
5. For an individual-frame workflow, retain the accepted identity anchor in
   every edit. When using Flixly's demonstrated reference arrangement, supply
   the preceding accepted frame first as the edit target and the opening image
   second as the identity reference. Name both roles in the prompt. This is a
   documented arrangement, not evidence that reference order alone fixes gait.
   A single-character motion sheet is also a valid experiment: begin with
   the published reference-led prompt in `RESEARCH.md`, preserve its simple
   structure, and change the subject/action only. Do not expand it into a
   mixed-character, mixed-direction brief. Verify every frame before reuse.
6. Describe facing in screen terms: "face upper-left, show the back and the
   left-facing silhouette" rather than relying only on "NW". Verify visually;
   do not silently relabel a duplicate NE view as NW.
7. For movement, request explicit leg/arm changes: left foot planted forward,
   right foot passing, right foot planted forward, left foot passing. Ensure
   contact points and frame timing form a cycle rather than a row of portraits.
   For combat, retain the published preparation/contact/recovery structure.
   Author playback timing separately from the image request; uniform frame
   holds are not obligatory. Align the visible contact with the game's actual
   confirmed strike event, rather than delaying damage to fit a showcase GIF.
8. Assemble sheets in code with integer cell rectangles and stored pivots.
   Do not independently stretch each pose to fill its cell.
9. Check terrain repetition in a 3x3 preview and the live camera. A texture
   may be useful as a patch without being seamless. Do not label it tileable
   until opposite edges and visible joins are reviewed.
10. Run the asset inside the production renderer. Check movement, attacks,
    equipment changes, occlusion, contrasting ground, and performance. A contact
    sheet and a passing build do not prove gameplay visual acceptance.

## Single-frame prompt template

Start with this compact adaptation of Noel's separate-image method and the
Flixly one-action edit. Expand it only to address a visible failure:

> Edit reference 1, the preceding accepted frame. Reference 2 is the original
> character and style anchor. Advance the lower-right-facing walk: the right
> foot moves forward and plants; the left foot trails. Retain the character,
> camera, scale and palette. Keep the whole silhouette on a transparent
> background. Return one frame.

This is our adapted prompt, not a quotation from a creator or a proven gait.
Anatomical left/right must be checked in the resulting image. A walking
transition also needs a plausible weight shift and coordinated arms; Flixly's
nearly stationary fox example does not validate freezing a human torso through
a stride. For a single reference, omit reference 2 rather than implying an
image was provided.

Use case: stylized-concept (new reference-led generation) or identity-preserve
(edit of an accepted actor).

Asset: one isolated [actor/prop], [one pose], [one screen direction].
Reference 1: previous accepted frame to edit; advance [named movement and
weight transition]. If this is the first pose, use the identity anchor here.
Reference 2, if supplied: original accepted identity anchor; preserve anatomy,
clothing, palette, camera elevation, apparent scale, and light direction.
Pixel construction: [64]-pixel crown-to-sole logical body, deliberate square
clusters, [24–32] color palette, large readable shadow masses, crisp edges.
Materials: [specific natural materials and colors].
Composition: full silhouette and all extremities visible, stable ground
contact and generous empty margin, no other subjects or scenery.
Background: actual transparent alpha.
Output: one frame. Atlas placement and exact output dimensions are handled
by the importer.

Do not paste every failure into every prompt. Keep constraints short and
target the current failure; repeated negative descriptions can contaminate
the requested appearance. Save the actual prompt, references, output path,
observed checks, acceptance state, and conversion provenance for each attempt.

## Adapting the published combat workflow

Use [Kiki's complete original prompt](https://x.com/Mayz1169/status/2097540160611287452)
for a planned action. Preserve its action beats, body mechanics, fixed camera,
registration and weapon clearance. Author frame holds against the game's
cadence. Adapt the character to our accepted adult reference and specify the
actual action/equipment. Codex handles slicing, alpha checks, Pixel Respecter
conversion and assembly; the image tool supplies the image.

Review palette stability during playback, position jumps and limb continuity.
Those checks follow the reader critiques; the critiques are not measurements
of our output. Keep successful poses and repair the observed failed transition.
