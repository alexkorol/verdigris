"""Build lossless four-phase paint guides from death-v2 Blender references."""
import hashlib
import json
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice/monsters-v2'
SOURCE = ROOT / 'death-v2'
GUIDES = SOURCE / 'guides'
GUIDES.mkdir(parents=True, exist_ok=True)
manifest = json.loads((SOURCE / 'manifest.json').read_text())
output = {'clips': {}, 'source_manifest': 'death-v2/manifest.json'}

for name, original in manifest['clips'].items():
    clip = dict(original)
    # Imagegen preserves the supported 1536x1024 landscape canvas. Square
    # 1024 requests may become 1254x1254 and no longer preserve this 4px grid.
    inset = [48, 16] if clip['frame'][0] == 96 else [32, 16]
    canvas = Image.new('RGBA', (384, 256))
    clip['frames'] = []
    for index, frame in enumerate(original['frames']):
        path = SOURCE / frame['file']
        assert hashlib.sha256(path.read_bytes()).hexdigest() == frame['sha256']
        pixels = np.array(Image.open(path).convert('RGBA'))
        pixels[:, :, 3] = np.where(pixels[:, :, 3] >= 128, 255, 0)
        pixels[pixels[:, :, 3] == 0, :3] = 0
        native = Image.fromarray(pixels)
        canvas.alpha_composite(native, ((index % 2) * 192 + inset[0],
                                        (index // 2) * 128 + inset[1]))
        clip['frames'].append({'src': path.relative_to(ROOT).as_posix(),
                               'sha256': frame['sha256'],
                               'blender_frame': frame['blender_frame']})
    guide = GUIDES / f'{name}.png'
    canvas.resize((1536, 1024), Image.Resampling.NEAREST).save(guide)
    clip['layout'] = {'columns': 2, 'slot': [192, 128], 'inset': inset,
                      'scale': 4, 'output': [1536, 1024]}
    clip['guide'] = guide.relative_to(ROOT).as_posix()
    clip['guide_sha256'] = hashlib.sha256(guide.read_bytes()).hexdigest()
    clip['blender'] = 'death-v2/' + clip['blender']
    output['clips'][name] = clip

(SOURCE / 'transfer-manifest-v4.json').write_text(json.dumps(output, indent=2) + '\n')
print(f'Prepared {len(output["clips"])} true-alpha 1536x1024 guides')
