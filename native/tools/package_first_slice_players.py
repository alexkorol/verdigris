"""Package reviewed player locomotion only; incomplete cast cannot enter this pack."""
import hashlib
import json
import shutil
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'
ART = Path('Z:/Code/.worktrees/wizard-art-player-guides/art_studies/starter-derivatives-v01')
OUT = ROOT / 'reviewed'

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def package():
    for folder in ('frames', 'references', 'originals', 'reports', 'blender', 'previews'):
        (OUT / folder).mkdir(parents=True, exist_ok=True)
    manifest = {'version': 1, 'status': 'agent-reviewed locomotion; not a complete game asset pack',
        'frame': [96, 96], 'anchor': [48, 80], 'pixels_per_metre': 48,
        'directions': ['front', 'right', 'back', 'left'], 'frames_per_cycle': 8,
        'clothing': 'Untrimmed natural flax linen; natural cord; open sandals; shorter practical female cut',
        'female_hair': 'One long brown braid centered down the back',
        'image_generation': {'method': 'ChatGPT image generation with individual Blender image attachments',
            'conversation': 'https://chatgpt.com/c/6aa8d012-1874-83e9-b462-94429c53de6a',
            'transparency': 'Requested background="transparent" in every generation prompt; native returned PNG alpha verified',
            'parameter_claim': 'Web requests; no hidden API parameter is claimed'},
        'not_included': ['idle', 'combat', 'equipped variants', 'diagonals', 'NPCs', 'monsters', 'environment', 'native game integration'],
        'clips': {}}
    for sex in ('male', 'female'):
        for gait in ('walk', 'sprint'):
            blend = (ART / 'paint-v2/blender' / f'male-{gait}-linen.blend' if sex == 'male'
                     else ROOT / 'blender-player' / f'female-{gait}-center-braid.blend')
            shutil.copy2(blend, OUT / 'blender' / blend.name)
            for direction in manifest['directions']:
                clip = f'{sex}-{gait}-{direction}'
                report_path = ROOT / 'review' / (clip+'.json')
                report = json.loads(report_path.read_text())
                raw = ROOT / 'source' / (clip+'.png')
                assert digest(raw) == report['source_sha256'], 'Stale report cannot publish a newer or rejected source'
                assert report['registered_mean_iou'] >= .70
                assert len(report['frames']) == 8
                with Image.open(raw) as im:
                    assert im.mode == 'RGBA' and im.size == (1536, 1024)
                    assert im.getchannel('A').getextrema()[0] == 0
                shutil.copy2(raw, OUT / 'originals' / raw.name)
                entries = []
                for i, entry in enumerate(report['frames']):
                    assert entry['silhouette_iou'] >= .65
                    frame = ROOT / 'candidates' / entry['file']
                    assert digest(frame) == entry['sha256']
                    with Image.open(frame) as im:
                        assert im.mode == 'RGBA' and im.size == (96, 96)
                        assert set(im.getchannel('A').getdata()) == {0, 255}
                        assert im.getbbox() == tuple(entry['bbox'])
                    reference = (ART if sex == 'male' else ROOT) / entry['reference']['src']
                    assert digest(reference) == entry['reference']['sha256']
                    reference_name = f'{clip}-{i:02d}.png'
                    shutil.copy2(frame, OUT / 'frames' / frame.name)
                    shutil.copy2(reference, OUT / 'references' / reference_name)
                    entries.append({'file': 'frames/'+frame.name, 'sha256': digest(frame),
                        'reference': 'references/'+reference_name, 'reference_sha256': digest(reference),
                        'bbox': entry['bbox'], 'silhouette_iou': entry['silhouette_iou']})
                report['visual_acceptance'] = 'agent inspected all 8 frames at native and 2x scale; owner acceptance not claimed'
                (OUT / 'reports' / report_path.name).write_text(json.dumps(report, indent=2))
                manifest['clips'][clip] = {'frames': entries, 'source': 'originals/'+raw.name,
                    'source_sha256': digest(raw), 'blender': 'blender/'+blend.name,
                    'blender_sha256': digest(blend), 'whole_cycle_translation': report['whole_cycle_translation']}
        shutil.copy2(ROOT / 'review' / f'{sex}-complete-2x.png', OUT / 'previews' / f'{sex}-complete-2x.png')
    (OUT / 'manifest.json').write_text(json.dumps(manifest, indent=2))
    print('Packaged 128 verified RGBA frames; 16 complete locomotion clips. Game integration remains incomplete.')

if __name__ == '__main__':
    package()
