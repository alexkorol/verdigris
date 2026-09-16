"""Candidate-only provenance and fixed-origin review; does not alter sprite pixels."""
from pathlib import Path
import hashlib
import json
import numpy as np
from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
PROMPTS = HERE.parent / 'prompts'
OUT = HERE / 'actors_hero-strike-ne-candidate'
IDENTITY = HERE.parent / 'runtime/hero_ne.png'
GENROOT = 'C:/Users/Alex/.codex/generated_images/01a0896d-2712-78b1-8ee3-5e362a202349/'

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def ref(path, role, supplied=None, note=None):
    item = {'path': str(path), 'sha256': sha(path), 'role': role, 'inspected_before_generation': True}
    if supplied:
        item['supplied_path_at_generation'] = supplied
    if note:
        item['note'] = note
    return item

def main():
    identity = ref(IDENTITY, 'original accepted character identity, camera and palette anchor; reference 2')
    windup = ref(HERE / 'actors_hero-strike-ne-windup-v2-cell12.png', 'reviewed preparation pose to edit for arm continuity; reference 1')
    commit = ref(HERE / 'actors_hero-strike-ne-reference-commit13.png', 'reviewed overhead left-arm pose to edit; reference 1', str(OUT / 'commit.png'), 'Archived by deterministic reconstruction of the original cell13 settings after the cycle scale changed to cell12.')
    contact = ref(HERE / 'actors_hero-strike-ne-reference-contact12.png', 'reviewed downward left-arm contact candidate; reference 1', str(OUT / 'contact.png'), 'Snapshot retained before the final shared cycle palette was applied.')
    follow = ref(HERE / 'actors_hero-strike-ne-reference-followthrough12.png', 'reviewed follow-through with local checker repair; reference 1', str(OUT / 'followthrough.png'), 'Snapshot retained before the final shared cycle palette was applied.')
    attempts = [
        ('contact-v2', 'exec-880148e2-2cc9-445f-93ee-b30f3a848b50.png', windup, 'rejected', 'Extended screen-right arm and moved bronze guard to screen-left; handedness failure.'),
        ('contact-v3', 'exec-fb7d0a06-5177-4168-8eba-e4c140cfc7ee.png', windup, 'candidate commit pose', 'Left arm and bronze right guard are coherent. Pose reads as overhead loading, so it was reclassified as commit rather than called contact.'),
        ('contact-v4', 'exec-570d12f3-7e94-48bc-8047-8ce98ca30be3.png', commit, 'rejected', 'Left arm remained coherent but head, boots and body turned screen-left; NE facing failure.'),
        ('contact-v5', 'exec-8393a980-a885-4678-bb72-4231feea0c3f.png', commit, 'candidate contact', 'Left-arm downswing stays on screen-left with bronze right guard. The outward left sweep does not yet establish a hit toward an NE target; little torso weight transfer.'),
        ('followthrough-v1', 'exec-6ba3690f-e7b9-4a9c-8f85-8698d3704258.png', contact, 'candidate follow-through', 'Left fist drops beside left hip and right guard remains. Pale wrap and torso details differ from preceding poses. Enclosed underarm checker repaired through two inspected Pixel Respecter windows.'),
        ('recovery-v1', 'exec-24727109-85c2-4c8b-a1d5-f8aaeaee957c.png', follow, 'candidate recovery', 'Arms settle toward idle with NE facing. Left wrist wrap becomes dark while right wrap is pale; material continuity is not accepted.')
    ]
    all_attempts = []
    for suffix, original, preceding, status, verdict in attempts:
        stem = 'actors_hero-strike-ne-' + suffix
        source, prompt = HERE / (stem + '.png'), PROMPTS / (stem + '.txt')
        im = Image.open(source)
        record = {'version': 1, 'tool': 'image_gen.imagegen', 'model': 'tool default; no selector or model identification exposed', 'calls': 1,
            'source': str(source), 'source_sha256': sha(source), 'source_size': list(im.size), 'source_mode': im.mode,
            'source_alpha_extrema': im.getchannel('A').getextrema() if 'A' in im.getbands() else None,
            'original_tool_output': GENROOT + original, 'prompt_file': str(prompt), 'prompt_sha256': sha(prompt),
            'reference_images': [preceding, identity],
            'published_workflow': {'url': 'https://www.flixly.ai/blog/stop-motion-chatgpt-images-2-5',
                'method': 'Previous reviewed frame first, original identity second, one focused physical change.',
                'adaptation': 'Single-frame adult NE melee phases; this is our repair experiment, not the original fox prompt or a verified published sprite recipe.'},
            'status': status, 'visual_verdict': verdict,
            'runtime_acceptance': False, 'conversion_manifest': str(PROMPTS / 'actors_hero-strike-ne-repair-candidate.json')}
        (PROMPTS / (stem + '.provenance.json')).write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
        all_attempts.append(record)

    names = ['original idle', 'ready', 'windup', 'commit', 'contact', 'followthrough', 'recovery', 'original idle']
    files = [IDENTITY, OUT / 'idle.png'] + [OUT / (name + '.png') for name in names[2:-1]] + [IDENTITY]
    frames = [Image.open(path).convert('RGBA') for path in files]
    assert {frame.size for frame in frames} == {(80, 96)}
    durations = [400, 50, 90, 60, 50, 70, 100, 400]
    scale, footer = 4, 24
    native = Image.new('RGB', (80 * len(frames), 116), (32, 31, 29))
    strip = Image.new('RGB', (320 * len(frames), 408), (32, 31, 29))
    grid = Image.new('RGB', (1280, 816), (32, 31, 29))
    panes, measurements = [], []
    for index, (name, path, frame) in enumerate(zip(names, files, frames)):
        enlarged = frame.resize((320, 384), Image.Resampling.NEAREST)
        pane = Image.new('RGB', (320, 408), (32, 31, 29))
        pane.paste(enlarged, (0, 0), enlarged)
        ImageDraw.Draw(pane).text((6, 390), name, fill=(230, 225, 208))
        panes.append(pane)
        strip.paste(pane, (index * 320, 0))
        grid.paste(pane, ((index % 4) * 320, (index // 4) * 408))
        native.paste(frame, (index * 80, 0), frame)
        ImageDraw.Draw(native).text((index * 80 + 2, 100), name, fill=(230, 225, 208))
        data = np.asarray(frame)
        measurements.append({'phase': name, 'file': str(path), 'sha256': sha(path), 'bounds': frame.getchannel('A').getbbox(),
            'alpha_values': np.unique(data[:, :, 3]).tolist(),
            'visible_colors': len(np.unique(data[:, :, :3][data[:, :, 3] > 0], axis=0))})
    prefix = OUT / 'actors_hero-strike-ne-idle-cycle-idle'
    native.save(str(prefix) + '-1x.png')
    strip.save(str(prefix) + '-4x.png')
    grid.save(str(prefix) + '-grid-4x.png')
    palette = strip.quantize(colors=256, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    indexed = [pane.quantize(palette=palette, dither=Image.Dither.NONE) for pane in panes]
    indexed[0].save(str(prefix) + '.gif', save_all=True, append_images=indexed[1:], duration=durations, loop=0, optimize=False, disposal=2)
    gif = Image.open(str(prefix) + '.gif')
    assert gif.n_frames == len(frames)
    recorded = []
    for index in range(gif.n_frames):
        gif.seek(index)
        recorded.append(gif.info['duration'])
    assert recorded == durations
    report = {'version': 1, 'status': 'complete six-phase candidate, not a runtime accepted NE attack',
        'attempt_count_this_repair': len(attempts), 'attempts': all_attempts,
        'sequence': names, 'duration_ms': durations, 'timing_scope': 'offline review pacing only; not measured game strike timing',
        'placement': 'Existing native80x96 canvases at fixed output pivot40,96; no per-frame preview fitting or alignment.',
        'processing': 'Actual Pixel Respecter shared cell12 for generated sources, palette reducer across six candidate phases to32 colors, crisp alpha. Original idle kept unmodified at both ends of transition review.',
        'manifest': str(PROMPTS / 'actors_hero-strike-ne-repair-candidate.json'),
        'engine_provenance': str(OUT / 'actors_hero-strike-ne-repair-candidate.provenance.json'),
        'measurements': measurements,
        'limitations': ['The contact arm moves toward screen-left, so an NE target hit and held-weapon trajectory need correction or explicit live verification.',
            'Linen wrap and dark leather pixels change between phases; the shared palette cannot repair material identity.',
            'Head/torso widths and crown heights shift slightly despite common reconstruction pitch and a fixed output pivot.',
            'The action has distinct arm phases but weak weight transfer; it must not be described as an accepted full-body melee action.',
            'No production renderer, equipment socket, live capture or gameplay gate was run for these candidate-only files.']}
    (OUT / 'actors_hero-strike-ne-review.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({'frames': len(frames), 'six_unique_candidate_hashes': len({sha(p) for p in files[1:-1]}), 'gif_durations': recorded, 'review': str(prefix), 'measurements': measurements}, indent=2))

if __name__ == '__main__':
    main()
