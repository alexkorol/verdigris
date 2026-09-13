# Image 2.5: creator evidence for game assets and animation

Reviewed September 9–10, 2026. These are early creator experiments and public
critiques, not a benchmark proving game-ready animation. Primary X posts were
opened in the browser by the research worker because the web text fetch
returned 403. No promotional provider's affiliation was assumed.

## Start with these actual workflows

| Task | Original to reuse | Evidence boundary |
|---|---|---|
| Walking poses from one character | [PURESO's prompt screenshot](https://x.com/pureso_studio/status/2097520619193868590) | Published reference and walking sheet; no validated playback or alpha audit |
| Planned idle or attack | [Kiki's idle prompt](https://x.com/Mayz1169/status/2097535082248671625) and [combat prompt](https://x.com/Mayz1169/status/2097540160611287452) | Actual detailed inputs and output examples; retain action beats and registration, omit chibi conversion |
| A sheet that changes the design | [Noel's separate-frame fallback](https://x.com/elle_elle_e/status/2097439673308389840) | Original rejected grids and later portrait animation; not a walking test |
| Controlled next-frame editing | [Flixly's three-frame experiment](https://www.flixly.ai/blog/stop-motion-chatgpt-images-2-5) | Opening identity plus previous-frame reference; company demo at two frames per second |
| A repeatedly wrong pose | [Higgsfield's complete guide prompt](https://x.com/higgsfield/status/2097514684811554911) | Each frame already has a correct geometry guide; vendor demo, not generated physics |

These sources do not establish a universal successful prompt. The most useful
negative evidence is the [combat-sheet discussion](https://www.reddit.com/r/aigamedev/comments/1wbmvnm/gpt_image_25_nailed_a_16_frame_combat_sprite_sheet/):
the creator publishes the input, while readers identify swapped limbs and
missing action beats. An attractive sheet is only the start of acceptance.
The detailed entries below distinguish creator claims, inspected media and
our adaptations. Aggregator reconstructions are never labelled original inputs.

## Model identification

OpenAI released Images 2.5 on September 8, including API variants Flare and
Sunburst. The announcement describes reference preservation and multi-turn
editing. This establishes the names and release, not sprite accuracy.
[Official release](https://openai.com/index/introducing-chatgpt-images-2-5/).

The built-in tool available here has no model/variant argument. Preserve the
actual tool name and any returned model metadata; do not manufacture a Flare
or Sunburst selection in provenance. MAI-Image-2.5 and Gemini 2.5 are different
models and are excluded from claims about OpenAI Image 2.5.

## Primary game and animation examples

### 852話 / hakoniwa: reference to combat poses, then cleanup

September 8, explicitly GPT-Image 2.5. The original post supplies a reference,
a 4x4 combat sheet, and an assembled GIF. Character identity remains readable
across a variety of poses. The original chat screenshot acknowledges a
1254-pixel sheet with a painted background pattern, rather than the requested
128-pixel frames and true alpha. The next stage removes the background,
slices, aligns, and assembles. This directly supports separating art
generation from deterministic asset preparation; it does not prove all poses
form a usable combat cycle.
[Original post and media](https://x.com/8co28/status/2097423580229521849),
[chat screenshot](https://x.com/8co28/status/2097423580229521849/photo/2).

### Practical_Low29: shared English combat prompt and critical feedback

September 9, creator reports GPT Image 2.5 through Atlas Cloud; variant is
unspecified. Source render, sheet and GIF are posted. Their exact prompt:

> Make a simplified 128px pixel art combat motion sprite sheet of this character, transparent background if you can, arrange it 4x4.

The author then asks Codex to assemble the GIF. Comments specifically identify
swapped limbs, invented occluded anatomy, missing anticipation/follow-through,
and uneven timing. Other commenters consider it useful prototype art. A
commercial provider link is present; independence is unverified.
[Post, prompt and criticism](https://www.reddit.com/r/aigamedev/comments/1wbmvnm/gpt_image_25_nailed_a_16_frame_combat_sprite_sheet/?sort=top).

### のえる / Noel: individual frames after a failed grid

September 8, explicitly Image 2.5 editing a reference originally made with
Nano Banana Pro. The creator reports unwanted stylization/design changes in
a 3x3 attempt, then two sets of nine individual variations, later assembled.
The Japanese term does not necessarily mean broken anatomy. The original post shows
a nearly stationary pixel portrait/bouquet GIF and the instruction screenshot.
This is not a demonstrated walking cycle. Its request emphasizes
separate outputs based on the original pixel image, with consistent size and
placement. This is a documented conditional fallback, not evidence that every
sheet fails. Reuse an accepted pixel reference instead of re-describing the
character from scratch.
[Original experiment and prompt screenshot](https://x.com/elle_elle_e/status/2097439673308389840).

### Kiki: a deliberately modest idle cycle

September 8, explicitly Image 2.5. A posted sheet and GIF accompany a detailed
reference-led prompt. It preserves recognizable identity, asks for limited
pixel colors, small breathing/secondary movement and a blink, fixes feet,
camera and scale, and excludes turning or travel. Sixteen frames are requested
in a 4x4 layout, with margins, a common baseline, and a roughly 2.4-second
cycle. Export checks include neighboring fragments, clipping, jumps, actual
alpha and GIF trails. The original asks for chibi proportions: **do not carry
that adaptation into this game's grounded adults**. Frame/layout constraints
still require verification after generation.
[Original result](https://x.com/Mayz1169/status/2097533706022056339),
[full published prompt](https://x.com/Mayz1169/status/2097535082248671625).

### Kiki: combat beats and variable holds, with a second character example

The separate combat prompt explicitly sequences:

> Ready stance → Anticipation → Attack → Impact peak → Effects dissipate → Recovery.

It asks for body mechanics instead of moving a static sprite, different frame
holds for preparation and a fast strike, a 1.5–2 second action, fixed ground
registration, and enough cell margin for the weapon and effects. It leaves
the distribution across sixteen frames unspecified. The prompt also includes
GIF/PNG/ZIP assembly and transparency/disposal instructions; these describe
the wider ChatGPT workflow, not outputs our image-only tool can produce.

The creator posts Tanjiro and later Rengoku using the same approach. That is
same-author reuse, not independent replication. A reply still reports blur
and noise. Its chibi conversion is explicit and must be omitted for our adult
actors. This supplies a better starting combat brief, not evidence of a
weapon-consistent isometric animation set.
[Original combat example](https://x.com/Mayz1169/status/2097539942985728162),
[full combat prompt](https://x.com/Mayz1169/status/2097540160611287452),
[second character](https://x.com/Mayz1169/status/2097900600105308347),
[critical reply](https://x.com/JustLingonberry/status/2097717870251884833).

### Readers of Kiki's prompt: posted attempts and concrete reservations

The coordinator directly reopened the full combat prompt and inspected the
original 4x4 sheet in the browser. The visible sequence contains preparation,
a sword swing with water effects, and recovery; this visual inspection does
not establish native pixel dimensions, alpha, or game-ready timing.

AOSY replies with an animated chibi character and says they tried it. The
original reply and its displayed animation frame were inspected. This is a
second user's reported attempt, but they do not publish the exact submitted
prompt, model settings, rejected attempts, or individual frames. Do not count
it as a controlled reproduction or a validated adult isometric cycle.
[AOSY's attempt](https://x.com/aosy_ai/status/2097592739638735206).

Gregor's reply asks whether the palette stays consistent during playback and
distinguishes a sheet screenshot from an engine test. This is a useful review
criterion, not a measured finding that Kiki's particular output changed colors.
[Original critique](https://x.com/bygregorr/status/2097597642561708345).
Naves asks about position jumps based on earlier sprite attempts; their
question does not explicitly establish an Image 2.5 failure.
[Registration question](https://x.com/MauriNaves/status/2097655875745821139).

### SOTN mod: assets actually integrated into a game

September 9, author explicitly credits Astra and GPT Image 2.5 for a posted
character replacement in Castlevania: Symphony of the Night. The user-facing
instruction is a short request to replace Alucard with Goku, not a detailed
sprite brief. The author notes imagegen may need to be requested explicitly.
No frame audit or reproducibility package is published. This is useful evidence
of integration, not a validated recipe for our animation cadence.
[Gameplay post and author's workflow](https://www.reddit.com/r/ChatGPT/comments/1wc1ods/swapped_alucard_with_another_character_in_a_sotn/).

### Flixly Team: keep both the opening image and the previous frame

September 8, explicitly GPT-Image-2.5 Sunburst. The creator supplies a starting
clay-fox image, three generated states and a short assembled video. Frame two
references the opening image; frame three supplies the previous frame first
and the opening image as the additional reference. Each edit changes one small
action while retaining the camera, light, body and supporting paws. The
published action instruction is:

> Lift the front paw nearest the camera off the tabletop.

The result is hard-cut stop motion at two frames per second. This supports an
identity anchor plus an optional transition reference; three opaque clay
frames do not establish transparent sprite quality or a fluid walk cycle.
This is a tool team's demonstration, not an independent benchmark. The research
worker read the original page and media through the browser after web text
fetch failed.
[Original prompts, reference strategy and video](https://www.flixly.ai/blog/stop-motion-chatgpt-images-2-5).

### FomskyWei: name a complete action before slicing

September 8, explicitly GPT-Image 2.5. The original post displays a pixel
goblin archer GIF made by requesting sixteen frames covering drawing and firing
a bow, then slicing and assembling the sheet. It does not publish a reference
image or reproducible API settings. The comparison with an older model uses
different characters, so it cannot establish a controlled improvement.
[Original animation and instruction](https://x.com/Fomsky_Wei/status/2097559943075533257).

### heliumcraft / iurimatias: image keyframes plus a separate video model

September 9, creator explicitly identifies Image 2.5 for image restyling and
MiniMax H3 for the resulting motion. They extract source frames at one-second
intervals, edit each in the same ChatGPT conversation, then give H3 the edited
images at their corresponding timestamps. Their exact image prompt is:

> this is a 80s anime. convert the art style and quality to modern anime quality and art style, sakuga art

The creator reports using half-second reference spacing for a difficult shot
and suggests denser references for an invented moving ring. They also correct
an initially wrong uploaded clip in the comments. Feedback disputes whether
the output preserves the original animation. This is a practical storyboard
or cutscene experiment; it does not prove an engine-ready sprite cycle, and
H3's interpolation must not be credited to Image 2.5 alone.
[Creator's workflow, corrections and discussion](https://www.reddit.com/r/StableDiffusion/comments/1wblyjb/gptimage_25_local_minimax_h3_to_convert_a_40_year/).

### 12ui / withmagi: give each reference a specific job

September 8–9, firsthand developer comparison of Image 2, Image 2.5 Flare and
Sunburst across twelve interface prompts, with output galleries and reported
quality, cost and time. The original comparison page exposes the full prompts
and input images. Its film-catalogue example assigns image one to interaction
and information requirements, image two to composition, and image three to
contrast. It asks for new content and visual assets rather than copying the
references. The creator considers Flare medium a useful draft setting; that is
their evaluation, not a sprite benchmark or a setting available in our tool.

This supplies a concrete pattern for using a Diablo UI reference alongside our
own material/style reference: label each image's role explicitly. It does not
justify mixing several actor identities in one sheet.
[Original interactive comparison and complete prompts](https://12ui.com/gpt-image-2.5-vs-2),
[developer's report](https://www.reddit.com/r/codex/comments/1wb8p1g/gpt_image_25_comparison_for_ui_generation/).

### Guizang: game integration succeeded while transparency still failed

September 9, the creator credits GPT-6 Astra, GPT-image-2.5 and JavaScript for
a 2D game with generated backgrounds, platforms, monsters and action sheets
assembled into keyframe animation. The post links a gameplay demonstration.
Five minutes later, the same creator reports that Image 2.5 would not produce
a transparent PNG for them. The accessible text supplies no image prompt or
variant/settings, so this does not establish a cause. Both are the creator's
own Weibo posts syndicated by Sina, directly read by the research worker.
[Game integration post](https://www.sina.cn/news/detail/5341225257274843.html),
[transparency follow-up](https://www.sina.cn/news/detail/5341226594469014.html).

### Segmind / Himanshu Goel: inspect alpha and the whole edited image

September 9, a vendor-authored firsthand Flare/Sunburst test supplies reference,
mask and output images. A Flare badge requested with `background: transparent`
and `output_format: png` returned actual RGBA: the author measured 55.6 percent
fully transparent pixels, maximum alpha 254 and a partial-alpha fringe. The
exact badge prompt is not published. A separate Sunburst masked fruit edit
also changed colors beyond the requested mask; the reported 97.5 percent
outside-pixel difference uses a threshold of only one channel level.

These are individual tests, not a general failure rate. Different tasks and
access paths prevent a causal comparison with Guizang's PNG failure. For our
pipeline, this supports measuring alpha and reviewing the full output after
a focused edit. Our tool does not expose these API output parameters.
[Original experiments and measurements](https://blog.segmind.com/gpt-image-2-5-api-the-ultimate-guide-to-flare-and-sunburst/).

### HaremVictoria: game-room edits improved; surface noise persists

This game-room creator reports less degradation during repeated ChatGPT edits,
but persistent microtexture across walls and floors. Their reproduction subject
is a post-apocalyptic brick basement. They still prefer Nano Banana for rooms.
No complete prompt, comparison gallery, variant or settings are supplied;
their reply confirms ChatGPT rather than API use. This is firsthand feedback,
not a controlled comparison.
[Original game developer comment and follow-up](https://www.reddit.com/r/codex/comments/1waxfbk/comment/p8m88mg/).

Our inference: inspect quiet surfaces and character/background separation at
gameplay size. The report supplies no proven corrective prompt. The owner's
objection to excessive patina remains a separate art-direction constraint.

## Additional original reports checked during the follow-up

The following originals were opened directly during the follow-up. X text
was read through the browser when web fetch returned 403. The frame counts
below are creator reports, not our frame-by-frame media audits.

- **Ivana, September 8:** reports generating 36 Image 2.5 stills and assembling
  them in Codex without a video model. The original has a video but no reusable
  prompt or frame package. This expands the evidence beyond three-frame demos;
  it does not establish a reliable sprite-production recipe.
  [Original report](https://x.com/ivanainai/status/2097446105906553188).
- **Gabriel Chua, September 8:** reports 99 Sunburst frames made through small
  edits, preserving an otter, a person and a stall, with Astra assembling the
  result and audio. A reply asks how continuity was checked; no answer was
  visible in the inspected thread. No full prompts or rejection counts were
  published there. Independence was not established.
  [Workflow report](https://x.com/gabrielchua/status/2097512638704197681),
  [linked film](https://x.com/gabrielchua/status/2097512144766128152).
- **Fun_Walk_4965:** publishes a numbered dance sheet prompt, then uses that
  whole sheet as MiniMax H3's only image reference. The original explicitly
  separates the image and video stages and contains a commercial provider
  link. Its exact image prompt is:

  > Create a 4x4 numbered pose sheet, panels 1 to 16, of this one character performing a continuous dance. Plain white background, one full body pose per panel, small number in the top left corner of each panel, thin motion lines allowed, no text, keep hair accessory, sweater print, skirt pattern and socks identical in every panel. Square image.

  Useful as a cutscene pose-reference workflow. Its numbers, motion lines and
  opaque background are deliberate storyboard choices, not sprite-export
  requirements. The phrase "no text" also conflicts with its requested panel
  numbers; retain that distinction when adapting, rather than blindly copying.
  [Original prompt and workflow](https://www.reddit.com/r/aivideos/comments/1wbgidh/one_16_panel_pose_sheet_from_gpt_image_25_was_the/).

Search also surfaced older AutoSprite and Codex-sprite-skill discussions.
Their dates predate this Image 2.5 release, so they were excluded from
model-specific evidence. Prompt aggregators were used to locate original
posts, not counted as independent successful tests. The follow-up still found
no verified Image 2.5 terrain-seam recipe or complete adult isometric gait.

## September 10 follow-up: actual walking and explicit pose guides

The research workers opened these original X posts in the browser and read
their prompt screenshots or expanded text. Model names below are creator
claims; their backend variants were not independently established.

- **PURESO, September 8:** reports using free ChatGPT Images 2.5 for combat
  and then walking sheets of the same character. The walking instruction
  requests simplified 128px pixel art and sixteen poses in a 4x4 sheet. The
  creator praises clothing continuity and both legs participating. Similar
  stride silhouettes remain in the displayed sheet; alternating support,
  alpha, timing and loop closure were not validated. The reopened original
  contains two still screenshots, not walking playback. Its suggestion about
  paid access comes from a Plus upsell banner, not a paid/free comparison.
  This is an actual walking
  prompt, unlike adapting a combat prompt and calling that published gait
  evidence. [Original post and prompt screenshot](https://x.com/pureso_studio/status/2097520619193868590).
- **Higgsfield, September 8:** the vendor's Image 2.5 spring-turntable example
  supplies a geometry/pose/composition guide for each frame. Exact excerpt:
  "the supplied guide already contains the correct rotation for this frame,
  so copy its pose exactly." Surface and lighting instructions are separate;
  handedness, silhouette, contact point, camera, scale and margins are retained.
  The complete prompt also explicitly discards the guide's background, material
  shading and long hard shadow. State both what each reference must preserve
  and what its rendering must replace; assigning an image a vague role is less
  specific than this published instruction.
  The guide determines the rotation, so the demonstration does not establish
  simulated spring physics or humanoid gait. Our local adaptation assigned
  accepted hero walk frames to pose and the raider idle to identity. Five
  built-in calls produced four selected contact/passing configurations, including
  an axe repair; both support sides survived Pixel Respecter reconstruction.
  Narrower bodies, mask changes and smaller axes remain. These are our measured
  candidate results, not a claim about the vendor's humanoid-animation quality.
  Production acceptance is recorded separately with the game captures.
  [Original demonstration](https://x.com/higgsfield/status/2097514650518831462),
  [complete original prompt](https://x.com/higgsfield/status/2097514684811554911).
- **YuK1, September 8:** explicitly credits Image 2.5 and Astra Pro for a
  fighting-animation experiment and says the first-pass result still needs
  substantial work. No reproducible prompt or settings are published. This is
  firsthand negative feedback, not a repair recipe.
  [Original experiment](https://x.com/YuK1_Game/status/2097446904942780699).
- **Zho, September 8:** reports losing the intended task after ten consecutive
  Image 2.5 generations while praising style transfer. The complete prompt
  sequence is absent; ten is one observed run, not a model context limit.
  [Original test](https://x.com/ZHO_ZHO_ZHO/status/2097573152675316038).

The workers also traced Noel's [two rejected 3x3 outputs](https://x.com/elle_elle_e/status/2097440118718316964),
which accompany the original separate-image fallback above. The root reopened
Practical_Low29's original post and criticism directly in the browser.

The circulating [278-prompt collection's author](https://www.reddit.com/r/PromptEngineering/comments/1wcbfjw/i_recovered_the_prompts_behind_openais_own_gpt/)
explicitly describes reconstructing prompts from OpenAI's example images.
Those are not the original prompts behind those outputs. Keep reconstructed
recipes separate from creator-published inputs. This follow-up still found no
verified Image 2.5 terrain-seam recipe.

## Reusable published prompt excerpts

These are source excerpts, not new all-purpose recipes. The linked originals
show their surrounding instructions and results. Preserve the successful
structure before adapting the subject and action.

| Purpose | Published instruction | What it controls |
|---|---|---|
| Reference to combat sheet | Practical_Low29's complete one-line prompt above | One character reference, simplified pixel style, one motion study |
| Failed sheet to separate frames | Noel, translated: "Create nine variation images for a GIF, each as a separate image." | Separate generated images; original reference, size and placement retained in the accompanying brief |
| Idle alignment | Kiki: "Use identical alignment and a fixed baseline across all cells." | Shared placement in a small-motion idle cycle; [full prompt](https://x.com/Mayz1169/status/2097535082248671625) supplies the rest |
| One next-frame change | Flixly's paw instruction above | A single physical change with opening and previous images as references |

No report here verifies a complete, weapon-consistent, eight-direction adult
actor set. Similar combat-sheet reposts must not be counted as independent
replications. Creator claims, inspected outputs, public criticism and our own
reproductions are separate kinds of evidence.

## Useful feedback that must not be misattributed

- HN user jjcm supplies Image 2 versus 2.5 UI outputs and reports better
  reference adherence, including a Warcraft III-style button, while noting
  blurry tiny decorative glyphs. Applicable to reference fidelity and UI
  inspection, not proof of sprite animation.
  [Firsthand comparison](https://news.ycombinator.com/item?id=49616298).
- HN's previous-frame-plus-direction recommendation explicitly reports
  **Nano Banana** results. It is adjacent-model advice, not a tested 2.5 claim.
  [Exact comment](https://news.ycombinator.com/item?id=49615718).
- The nearby recommendation to normalize scale, reduce palettes and align
  grids refers to unspecified generative models. Our Pixel Respecter pipeline
  performs those concrete operations, but that comment is not a 2.5 benchmark.
  [Creator's cleanup experience](https://news.ycombinator.com/item?id=49615395).

- Animation clips using Image 2.5 together with H3 or Seedance demonstrate a
  combined workflow. Credit Image 2.5 for its still images and edits; do not
  attribute the video model's motion synthesis to the image model.

### Existing tiling implementation worth reusing: different models

The public [ianlintner game-asset repository](https://github.com/ianlintner/ai-pixel-art-image-generation)
contains source and example PNG/TSX/TMJ exports. It explicitly names
`gpt-image-2` and Gemini 2.5 Flash Image, not OpenAI Image 2.5. The worker and
root inspected its source; neither ran it or validated the example artwork.

Its [tileset generator](https://github.com/ianlintner/ai-pixel-art-image-generation/blob/main/scripts/generate_tileset.py)
generates terrain concepts separately with shared style/palette constraints.
The [seam-processing implementation](https://github.com/ianlintner/ai-pixel-art-image-generation/blob/main/scripts/lib/seamless.py)
tries a center crop, half-image offset with seam blending, and opposite-edge
matching with inward feathering and palette quantization. It measures edge
differences to select a result. This is existing engineering to investigate
before inventing another tiling pipeline; it is not a prompt-only success.
The metric checks opposite edges of one tile, not compatibility between
different materials, quiet detail density or hidden repetition. Review those
properties separately. The owner's actual Pixel Respecter reconstruction
remains the required pixel-grid stage in our pipeline.

The Happycapy/Min Zhou game lead remained a discovery-only mirror with no
verified original prompt in this audit. It is not added as an independent
successful recipe. No verified Image 2.5-specific terrain-seam recipe was found.

## Applied rules and next experiments

1. Use an actual accepted reference; preserve its design in edits. For a style
   change, simplify once, inspect it, then use the simplified image as the
   continuing reference. Keep natural material colors and adult proportions.
2. Preserve the structure of the relevant published recipe. The short combat
   study and Kiki's longer planned-action prompt serve different purposes;
   these examples do not establish that shorter prompts always work better.
   Try one character and one motion in a sheet when appropriate. If the grid
   or anatomy fails, use Noel's separate-image workflow.
3. Distinguish recognizable identity from correct motion. Review left/right
   limbs, feet, weight shifts, anticipation, impact, recovery and loop closure.
4. Keep alpha cleanup, measured crop boundaries, pivot alignment, palette/grid
   reconstruction, atlas packing and playback in tools. Exact prompt dimensions
   and a visible background pattern are not acceptance evidence.
5. Review the animation at intended playback speed and in the game. Count
   usable frames, reject duplicated phases, and preserve motion timing. Do not
   treat a generated GIF or a grid of attractive poses as a finished animation.
6. No verified primary terrain-seam recipe was found in this pass. Test any
   terrain repetition rather than claiming a prompt guarantees seamlessness.
7. Reference-led editing is worth trying before restarting an accepted asset:
   the game-room developer reports improved edit stability. Review the whole
   image after each edit, especially large surfaces; better identity retention
   does not guarantee quieter texture or correct animation.

`PROMPTING.md` contains our adapted prompt template. Those adaptations are
project decisions, not quotations or claims that the creators tested our game.
The source/provenance files record our actual outcomes, including failures.
