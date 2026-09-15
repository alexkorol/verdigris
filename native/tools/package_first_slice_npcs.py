"""Package only explicitly reviewed static NPC turnarounds and their provenance."""
import argparse
import hashlib
import json
import shutil
from pathlib import Path
from PIL import Image

BASE = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'
ROOT = BASE / 'npcs-v2'
DEST = ROOT / 'accepted'

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def package(identities):
    clips, frames = [], []
    for folder in ['sprites', 'references', 'source', 'blender', 'reports', 'previews', 'prompts']:
        (DEST / folder).mkdir(parents=True, exist_ok=True)
    native_contact = Image.new('RGBA', (384, 96 * len(identities)), (36, 42, 35, 255))
    for row, identity in enumerate(identities):
        report = json.loads((ROOT / 'review' / f'{identity}.json').read_text())
        if report.get('visual_acceptance') is not True:
            raise ValueError(f'{identity} has not been visually reviewed')
        original = ROOT / 'source' / f'{identity}.png'
        if sha(original) != report['raw_sha256']:
            raise ValueError('Source changed since reconstruction')
        with Image.open(original) as im:
            if im.mode != 'RGBA' or im.getchannel('A').getextrema()[0] != 0:
                raise ValueError('Original has no actual transparency')
            original_alpha = im.getchannel('A').getextrema()
        for source, destination in [
            (original, DEST / 'source' / original.name),
            (BASE / 'blender-cast' / f'{identity}.blend', DEST / 'blender' / f'{identity}.blend'),
            (ROOT / 'review' / f'{identity}.json', DEST / 'reports' / f'{identity}.json'),
            (ROOT / 'review' / f'{identity}-2x.png', DEST / 'previews' / f'{identity}-2x.png'),
            (ROOT / 'guides' / f'{identity}.png', DEST / 'references' / f'{identity}-input-4x.png'),
            (ROOT / 'prompts' / f'{identity}.txt', DEST / 'prompts' / f'{identity}.txt')]:
            shutil.copyfile(source, destination)
        for i, direction in enumerate(['front', 'right', 'back', 'left']):
            filename = f'{identity}-idle-{direction}.png'
            p = ROOT / 'candidates' / filename
            im = Image.open(p)
            if im.size != (96, 96) or im.mode != 'RGBA' or set(im.getchannel('A').getdata()) != {0, 255}:
                raise ValueError('Frame geometry/alpha violation')
            reference = BASE / 'blender-cast' / f'{identity}-{direction}.png'
            if sha(reference) != report['reference_sha256'][i] or sha(p) != report['frame_sha256'][i]:
                raise ValueError('Reference or candidate changed after reconstruction')
            shutil.copyfile(p, DEST / 'sprites' / filename)
            shutil.copyfile(reference, DEST / 'references' / reference.name)
            native_contact.alpha_composite(im, (i * 96, row * 96))
            clips.append({'identity': identity, 'action': 'idle', 'direction': direction,
                'frames': ['sprites/' + filename], 'frame': [96, 96], 'anchor': [48, 80],
                'pixels_per_metre': 48, 'fps': 1, 'loop': True})
            frames.append({'identity': identity, 'role': 'npc', 'action': 'idle', 'direction': direction,
                'phase': 0, 'file': 'sprites/' + filename, 'sha256': sha(p),
                'frame': [96, 96], 'anchor': [48, 80], 'pixels_per_metre': 48,
                'source': 'source/' + original.name, 'source_sha256': sha(original),
                'original_alpha_range': original_alpha,
                'reference': 'references/' + reference.name, 'reference_sha256': sha(reference),
                'generation_input': 'references/' + identity + '-input-4x.png',
                'generation_input_sha256': sha(ROOT / 'guides' / f'{identity}.png'),
                'source_grid_pitch': report['grid']['size'],
                'editable_blender': 'blender/' + identity + '.blend',
                'blender_sha256': sha(BASE / 'blender-cast' / f'{identity}.blend'),
                'silhouette_iou': report['iou'][i],
                'static_view_translation': report['static_view_registration'][i],
                'agent_visual_review': True})
    native_contact.save(DEST / 'previews/native-contact.png')
    native_contact.resize((768, 192 * len(identities)), Image.Resampling.NEAREST).save(DEST / 'previews/contact-2x.png')
    manifest = {'schema_version': 1, 'accepted': True, 'role': 'npc',
        'pipeline': 'Original distinct Blender meshes -> native96x96 thresholded reference -> integer enlargement -> built-in imagegen -> Pixel Respecter -> static-view registration -> visual review',
        'registration_policy': 'Each static view matches its own Blender projection via integer translation; no scaling or animation phase registration',
        'clips': clips, 'frames': frames,
        'scope': 'Four role-specific NPC static turnarounds; no locomotion or combat animation claimed',
        'npc_binding': 'Roles only. Do not silently bind to unrelated named town characters.'}
    (DEST / 'manifest.json').write_text(json.dumps(manifest, indent=2))
    print(json.dumps({'accepted_frames': len(frames), 'manifest': str(DEST / 'manifest.json')}))

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('identities', nargs='+')
    package(p.parse_args().identities)
