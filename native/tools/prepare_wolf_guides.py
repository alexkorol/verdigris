import hashlib
import json
from pathlib import Path
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'
records = json.loads((ROOT / 'blender-wolves/manifest.json').read_text())
manifest = {'clips': {}}
preview = Image.new('RGBA', (768, 384), (36, 42, 35, 255))
for record in records:
    sheet = Image.new('RGBA', (384, 256))
    entries = []
    for i, frame in enumerate(record['frames']):
        path = ROOT / 'blender-wolves' / frame['file']
        im = Image.open(path).convert('RGBA')
        a = np.array(im)
        a[:, :, 3] = np.where(a[:, :, 3] >= 128, 255, 0)
        a[a[:, :, 3] == 0, :3] = 0
        im = Image.fromarray(a)
        box = im.getbbox()
        if box and (box[0] == 0 or box[2] == 96 or box[1] == 0 or box[3] == 96):
            raise ValueError(f'Clipped source: {frame["file"]}, {box}')
        sheet.paste(im, ((i % 4) * 96, (i // 4) * 128 + 16))
        entries.append(dict(frame, src='blender-wolves/' + frame['file'],
                            sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
        if record['clip'] in ('pack-wolf-walk-right', 'well-alpha-walk-right'):
            preview.alpha_composite(im, ((i % 4) * 192 + (i // 4) * 96,
                                         0 if record['clip'].startswith('pack') else 192))
    sheet.resize((1536, 1024), Image.Resampling.NEAREST).save(ROOT / 'guides' / (record['clip'] + '.png'))
    manifest['clips'][record['clip']] = {'frames': entries, 'anchor': record['anchor']}
    print(record['clip'], len(set(f['sha256'] for f in entries)), 'unique poses')
(ROOT / 'wolf-reference-manifest.json').write_text(json.dumps(manifest, indent=2))
preview.save(ROOT / 'review/wolf-motion-native.png')
