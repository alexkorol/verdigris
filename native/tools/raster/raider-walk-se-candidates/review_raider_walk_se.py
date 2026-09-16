"""Assemble fixed-canvas rejection evidence; this script does not edit sprites."""
from pathlib import Path
import hashlib
import json
from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
RASTER = ROOT / 'native/client/assets/raster'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def record(path):
    return {'path': str(path.relative_to(ROOT)).replace('\\', '/'), 'sha256': sha(path)}


idle_path = RASTER / 'runtime/raider_se.png'
idle = Image.open(idle_path).convert('RGBA')
paths = [HERE / 'v3' / f'raider_walk{i}_se.png' for i in range(8)]
frames = [Image.open(p).convert('RGBA') for p in paths]
assert all(im.size == (80, 96) for im in frames)
report_path = HERE / 'v3/raider-walk-se-v3.provenance.json'
report = json.loads(report_path.read_text())
assert all(a['before_resize'] == a['placed_size'] for a in report['assets'])
assert all(a['anchor_px'] == [40, 96] and a['cell_size'] == 6 for a in report['assets'])

ordered = [idle, *frames, idle]
strip = Image.new('RGBA', (80 * len(ordered), 96), (36, 33, 30, 255))
for i, im in enumerate(ordered):
    strip.alpha_composite(im, (i * 80, 0))
for scale in (1, 3):
    strip.resize((strip.width * scale, strip.height * scale), Image.Resampling.NEAREST).convert('RGB').save(HERE / f'raider-walk-se-v3-idle-cycle-idle-{scale}x.png')

def surface(im):
    bg = Image.new('RGBA', (80, 96), (36, 33, 30, 255))
    bg.alpha_composite(im)
    return bg.convert('RGB')

for scale in (1, 3):
    video = [surface(im).resize((80 * scale, 96 * scale), Image.Resampling.NEAREST) for im in frames]
    video[0].save(HERE / f'raider-walk-se-v3-loop-{scale}x.gif', save_all=True, append_images=video[1:], duration=[100] * 8, loop=0, disposal=2, optimize=False)

with Image.open(HERE / 'raider-walk-se-v3-loop-1x.gif') as gif:
    timing = []
    for i in range(gif.n_frames):
        gif.seek(i)
        timing.append(gif.info.get('duration'))
assert timing == [100] * 8

phases = [
    'F0: anatomical-left/screen-right leg extends forward; anatomical-right/axe-side leg bent behind. Possible isolated left-contact study, not approved identity.',
    'F1: leading anatomical-left shin becomes more upright; right leg remains occluded behind. Possible compression study only.',
    'F2: near-duplicate F1 stance; no clear opposite knee passing the planted leg.',
    'F3: left boot extends forward again, returning to same contact side.',
    'F4: left boot remains forward despite requested opposite contact. No valid half-cycle reversal.',
    'F5: left leg remains the low upright support, right leg remains behind. Repeats F1 rather than opposite support.',
    'F6: left boot extends forward; right boot stays behind; sole is one native row above shared baseline.',
    'F7: repeats the same forward left-leg silhouette, then loops to same-side F0.'
]

originals = [
    'C:/Users/Alex/.codex/generated_images/01a0896d-2712-78b1-8ee3-5e362a202349/exec-ac73c699-5ed1-4942-b2e4-b81a3012f47c.png',
    'C:/Users/Alex/.codex/generated_images/01a0896d-2712-78b1-8ee3-5e362a202349/exec-87eb77f8-175b-4460-81a6-8d42ddb5fadf.png',
    'C:/Users/Alex/.codex/generated_images/01a0896d-2712-78b1-8ee3-5e362a202349/exec-8382b90d-18b2-4c6b-989b-9a3515efbb2a.png',
]
external = Path('C:/Users/Alex/Documents/ChatGPT/diablo-reference-cache/motion-study-20260909/body-corrected-cof/se-ordered-4x2-3x.png')
recipe = {
    'url': 'https://www.reddit.com/r/aigamedev/comments/1wbmvnm/gpt_image_25_nailed_a_16_frame_combat_sprite_sheet/',
    'original_inspected': 'Opened original page and read creator prompt/source-sheet-GIF explanation during this task.',
    'published_prompt': 'Make a simplified 128px pixel art combat motion sprite sheet of this character, transparent background if you can, arrange it 4x4.',
    'adaptation': 'Single reference character and motion sheet preserved; action changed to8 lower-right walking phases in4x2; native identity/equipment and explicit opposite contacts added.',
    'evidence_limits': 'Creator reports GPT Image2.5 via Atlas Cloud with a commercial provider link; variant and independence unspecified. It is a combat study, not a demonstrated raider gait. Our external ordered-pose correction is a local experiment, not a published D2 recipe.'
}
for version in (1, 2, 3):
    source = RASTER / f'source/raider-walk-se-v{version}.png'
    prompt = RASTER / f'prompts/raider-walk-se-v{version}.txt'
    refs = []
    if version > 1:
        prior = RASTER / f'source/raider-walk-se-v{version-1}.png'
        refs.append({**record(prior), 'role': 'EDIT TARGET: preserve raider upper body, axe hand, facing and cell placement; change legs.', 'inspected_before_call': True})
    if version == 3:
        refs.append({'path': external.as_posix(), 'sha256': sha(external), 'role': 'MOTION ONLY: corresponding eight ordered hip/knee/boot positions; do not copy costume, rendering, labels or background.', 'inspected_before_call': True, 'external_reference_not_copied_into_repository': True})
    refs.append({**record(idle_path), 'role': 'ORIGINAL IDENTITY: adult lower-right raider, anatomy/clothing/pixelstyle/camera and anatomical-right axe hand.', 'inspected_before_call': True})
    info = {
        'generation_tool': 'built-in image_gen', 'selected_model': 'Not exposed by tool',
        'generated_original': originals[version-1], 'source': record(source),
        'prompt': record(prompt), 'submitted_prompt': prompt.read_text(),
        'references': refs, 'recipe': recipe,
        'correction': ['Initial study.', 'Focused lower-body alternation repair after same-leg repetition.', 'Final focused lower-body repair using existing validated external SE ordered motion reference.'][version-1],
        'observed_source': {'dimensions': list(Image.open(source).size), 'mode': Image.open(source).mode,
            'actual_alpha': False, 'background': 'Opaque painted checker; transparency request failed.',
            'facing': 'Lower-right retained in all8 cells.',
            'handedness': 'Axe remains in anatomical-right/screen-left hand; opposite hand empty.',
            'phase_failure': 'Anatomical-left leg from screen-right hip leads every contact; anatomical-right/axe-side leg remains trailing/occluded. No accepted opposite contact.',
            'identity_drift': 'Broader adult body, simplified coarse costume masses and shorter/thicker axe compared with original SE idle.',
            'status': 'rejected_as_full_walk'},
        'candidate_only': True, 'runtime_promoted': False
    }
    (RASTER / f'prompts/raider-walk-se-v{version}.provenance.json').write_text(json.dumps(info, indent=2) + '\n')

dependencies = {
    'generation': {
        'tool': 'built-in image_gen', 'api_key_required': False, 'selectable_model': False,
        'new_sources': [record(RASTER / f'source/raider-walk-se-v{i}.png') for i in (1, 2, 3)],
        'exact_prompts': [record(RASTER / f'prompts/raider-walk-se-v{i}.txt') for i in (1, 2, 3)],
        'identity': record(idle_path),
        'external_motion_reference': {'path': external.as_posix(), 'sha256': sha(external), 'required_for_generation_reproduction_only': True, 'not_required_for_conversion': True},
        'inspected_prior_art_only': [record(RASTER / 'runtime/raider_walk4_sw.png'), record(RASTER / 'source/raider-walk-sw-axe-v3.png')]
    },
    'conversion_minimal': {
        'manifest': record(HERE / 'raider-walk-se-v3-import.json'),
        'source': record(RASTER / 'source/raider-walk-se-v3.png'),
        'palette_reference': record(idle_path),
        'importer': record(ROOT / 'native/tools/raster/import_assets.py'),
        'python_executable': 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe',
        'engine': report['engine'], 'report': record(report_path),
        'command': 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe native/tools/raster/import_assets.py native/tools/raster/raider-walk-se-candidates/raider-walk-se-v3-import.json',
        'no_installs_or_runtime_edits': True
    },
    'review_only': {'script': record(Path(__file__)), 'dependencies': ['Pillow', 'Python standard library']}
}
(HERE / 'raider-walk-se-dependencies.json').write_text(json.dumps(dependencies, indent=2) + '\n')
review = {
    'verdict': 'REJECT: incomplete SE gait; retain as bounded experiment evidence only',
    'generation_count': {'initial': 1, 'focused_corrections': 2, 'stopped_at_authorized_limit': True},
    'geometry': {'canvas': [80, 96], 'pivot': [40, 96], 'source_pitch': 6, 'source_cell': [384, 490], 'source_column_stride': 384, 'source_row_stride': 490, 'source_origin_within_cell': [192, 462], 'order': 'Row-major0..7, unchanged', 'per_pose_fit_or_resize': False, 'body_height_px': [60, 61], 'idle_body_height_px': 56},
    'palette_alpha': {'fixed_palette': record(idle_path), 'maximum_colors': 32, 'output_alpha': [0, 255], 'partial_alpha_pixels': 0, 'cleanup': 'Actual Pixel Respecter border fill229/75 plus8 individually identified enclosed underarm windows, exact rectangles/counts in importer provenance; no global gray deletion.'},
    'phase_review': phases,
    'potential_future_reference_only': {'source': record(RASTER / 'source/raider-walk-se-v3.png'), 'cell0_box': [0, 0, 384, 490], 'cell1_box': [384, 0, 768, 490], 'use': 'F0 left-contact and F1 left-support compression can describe a single transition. They are not accepted identity anchors; original SE idle must remain the appearance reference. Missing right-contact/right-support is the exact future repair target.'},
    'playback_evidence': {'assembled_count': 8, 'duration_ms': timing, 'cycle_duration_ms': sum(timing), 'native_and_3x_ordered_transition_viewed': True, 'smooth_browser_or_in_game_playback_observed': False, 'limitation': 'The strip visibly establishes same-leg repetition; GIF assembly and timing checks do not establish smooth game motion.'},
    'outputs': [record(p) for p in paths], 'runtime_promoted': False
}
(HERE / 'raider-walk-se-review.json').write_text(json.dumps(review, indent=2) + '\n')
print(json.dumps({'review': str(HERE / 'raider-walk-se-review.json'), 'timing': timing, 'engine_dependencies': report['engine']['dependencies']}, indent=2))
