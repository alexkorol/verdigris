"""Pixel Respecter import for static calibrated props; no ink-height resizing."""
from pathlib import Path
import argparse
import hashlib
import json
import sys
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'

def run(raw, asset, respect_root):
    sys.path.insert(0, respect_root)
    from pixel_perfecter.reconstructor import PixelArtReconstructor
    spec = next(a for a in json.loads((ROOT / 'guides/props.json').read_text()) if a['id'] == asset)
    im = Image.open(raw)
    if im.mode != 'RGBA' or im.getchannel('A').getextrema()[0] != 0:
        raise ValueError('Original native-alpha PNG required')
    data = np.array(im)
    data[:, :, 3] = np.where(data[:, :, 3] >= 128, 255, 0)
    data[data[:, :, 3] == 0, :3] = 0
    rec = PixelArtReconstructor(image=data)
    rec.use_hough = False
    rec.run()
    pitch = round(im.width / spec['canvas'][0])
    candidate = next(a for a in rec.last_fit_debug['candidates'] if a['size'] == pitch)
    if candidate['gated_out']:
        raise ValueError('Expected grid failed variance gate')
    rec.cell_size, rec.offset = pitch, tuple(candidate['offset'])
    recovered = Image.fromarray(rec._empirical_pixel_reconstruction())
    a = np.array(recovered)
    a[:, :, 3] = np.where(a[:, :, 3] >= 128, 255, 0)
    a[a[:, :, 3] == 0, :3] = 0
    recovered = Image.fromarray(a)
    box = recovered.getbbox()
    if not box:
        raise ValueError('Empty prop')
    target_box = spec['bbox']
    # Static prop pivot follows the base plane, not arbitrary visible-height fit.
    dx = round((target_box[0] + target_box[2] - box[0] - box[2]) / 2)
    dy = target_box[3] - box[3]
    output = Image.new('RGBA', tuple(spec['canvas']))
    output.paste(recovered, (dx, dy))
    if np.count_nonzero(np.array(output)[:, :, 3]) != np.count_nonzero(a[:, :, 3]):
        raise ValueError('Clipped foreground during whole-prop registration')
    for folder in ('source', 'candidates', 'review'):
        (ROOT / folder).mkdir(parents=True, exist_ok=True)
    (ROOT / 'source' / (asset + '.png')).write_bytes(Path(raw).read_bytes())
    output.save(ROOT / 'candidates' / (asset + '.png'))
    bg = Image.new('RGBA', output.size, (36, 42, 35, 255))
    bg.alpha_composite(output)
    bg.resize((output.width * 2, output.height * 2), Image.Resampling.NEAREST).save(ROOT / 'review' / (asset + '-2x.png'))
    report = {'asset': asset, 'raw_sha256': hashlib.sha256(Path(raw).read_bytes()).hexdigest(),
              'source_reference': spec, 'returned_size': im.size,
              'grid_candidates': rec.last_fit_debug['candidates'], 'selected_grid': candidate,
              'registration_translation': [dx, dy], 'frame': output.size, 'anchor': spec['anchor'],
              'pixel_resizing': False, 'alpha_policy': 'Native alpha threshold128; no color key',
              'visual_acceptance': False, 'bbox': output.getbbox()}
    (ROOT / 'review' / (asset + '.json')).write_text(json.dumps(report, indent=2))
    print(json.dumps({'asset': asset, 'frame': output.size, 'grid': candidate, 'bbox': output.getbbox()}))

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('raw'); p.add_argument('asset'); p.add_argument('--respect-root', required=True)
    a = p.parse_args()
    run(a.raw, a.asset, a.respect_root)
