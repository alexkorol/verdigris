# Image 2.5: creator evidence for game assets and animation

Reviewed September 9, 2026. These are early creator experiments and public
critiques, not a benchmark proving game-ready animation. Primary X posts were
opened in the browser by the research worker because the web text fetch
returned 403. No promotional provider's affiliation was assumed.

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
Nano Banana Pro. The creator reports deformation in a 3x3 attempt, then two
sets of nine individual variations, later assembled. The original post shows
a pixel character GIF and the instruction screenshot. Its request emphasizes
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
references the opening image; frame three references both the previous frame
and the opening image. Each edit changes one small action while retaining the
camera, light, body and supporting paws. The published action instruction is:

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

## Applied rules and next experiments

1. Use an actual accepted reference; preserve its design in edits. For a style
   change, simplify once, inspect it, then use the simplified image as the
   continuing reference. Keep natural material colors and adult proportions.
2. Begin with a published short prompt before adding a long constraint list.
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

`PROMPTING.md` contains our adapted prompt template. Those adaptations are
project decisions, not quotations or claims that the creators tested our game.
The source/provenance files record our actual outcomes, including failures.
