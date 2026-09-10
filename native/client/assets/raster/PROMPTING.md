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
| NE overhead contact edited from a prior pose plus the original identity | The corrected left forearm supported an attached axe toward upper-right; the next follow-through attempt switched arms again | Accept individual transitions only after reviewing the actual held weapon; a successful reference edit does not validate the remaining cycle |
| Gray-matte cleanup on a failed follow-through source | Pale wrap, fist and forearm pixels were also removed | Compare reconstructed anatomy with the original source; reject cleanup that erases foreground colors and request actual alpha instead of accepting a clean-looking silhouette |
| Reference-led NE/NW raider walks | Fourteen reconstructed frames show leg progression in the production renderer, with narrower bodies and smaller axes | Accept motion and identity separately; retain the shared pivot and document the remaining cross-clip differences |
| SE raider sheet plus two targeted repairs | All three attempts kept the same anatomical leg forward, even with an ordered motion reference in the final repair | Stop promoting repeated contacts as a walk; preserve the failed experiment and leave that direction unavailable until an actual opposite support pose exists |

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
| Revising scenery | HaremVictoria's game-room editing report | Reuse an accepted image; inspect walls and ground for excess microtexture. The report supplies a failure example, not a verified corrective prompt |
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

For scenery, reserve quiet areas so the player, monsters and paths remain
readable. The firsthand game-room report in `RESEARCH.md` specifically flags
microtexture across large surfaces even after editing improved. Compare the
edited room or prop with its accepted reference at gameplay scale. Preserve
the owner's natural material colors; do not reintroduce green patina through
the game title or use added texture as a substitute for pixel construction.

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

These NE observations are local built-in-tool results, not a claim that a
particular Image 2.5 variant was selected. The source and conversion can fail
independently: the rejected follow-through switched arms before cleanup, and
its reconstructed foreground then suffered additional matte damage. Repairing
alpha alone would not make that pose acceptable.

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

## Applied follow-up: selected frames and fixed palettes

The September 10 NE repair uses Flixly's explicit reference jobs: the accepted
preceding contact pose supplies the transition, and the accepted idle supplies
identity. Preserve the successful windup/contact frames; repair only the failed
phase. This produced a usable same-arm follow-through, but its body remained
broader and its colors drifted. Reference reuse is an input method, not proof of
animation consistency.

Once native frames are reconstructed, `palette_reference` in the importer can
pin them to an accepted native PNG through the real Pixel Respecter
`palettes.snap_to_palette` API. This is a local deterministic cleanup step, not
a published Image 2.5 prompting trick. It preserves alpha, positions and the
shared pivot; it cannot repair anatomy, timing, material assignment or a bad
pose. Keep before/after native-size comparisons and the reference hash. The NE
ready frame remains exactly the existing idle instead of a newly generated copy.

The raider study retained five poses from its first same-character SW sheet and
replaced only the anticipation pose with unwanted olive clothing. Neutral-only
matte windows were checked against the source so pale skin was not mistaken for
background. Retain a failed cleanup when it explains the corrected recipe.

Finally inspect the poses inside the game. The first raider warning capture hid
both actors behind an opaque telegraph despite valid sprite traces. The warning
now preserves the interior pixels, and confirmed contact selects frame three on
the damage event. Atlas checks and an assembled GIF alone would miss that defect.

## Applied follow-up: dust, slash and collapse

The next three sources each used one built-in call with accepted references.
Four dust phases keep a shared64x48 canvas and pivot32,42; four raider collapse
poses use128x112/pivot64,96, extending the accepted idle grid without fitting
each pose. The single slash retains measured rotation/contact centers. These
are documented local adaptations of the published motion workflows.

The death source still painted a checker. Neutral-only cleanup was measured
and compared with the source; real dust alpha needed no matte removal. Actual
Pixel Respecter reconstruction and source/native review were necessary before
production. The first four-copy Sweep composition then looked like a flower in
the game, despite valid PNGs and passing tests. Two opposed halves corrected
that specific runtime failure without regenerating the accepted slash.

Live event timing needed a separate fix: age old effects before ingesting new
contact, so the first paint retains its contact phase. No image prompt can
replace that engine check. See reviews/2026-09-10-feedback/README.md for ordered
production frames, exact acceptance scope and remaining gaps.
