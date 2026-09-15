"""Recover static NPC turnarounds through the shared strict Pixel Respecter gate."""
import argparse
import hashlib
import json
from pathlib import Path
import first_slice_art
import sys
import numpy as np
from PIL import Image

BASE = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'
OUT = BASE / 'npcs-v2'

def reference_path(identity, direction):
    """Use editable-work references locally, or the committed pack on a new clone."""
    name = f'{identity}-{direction}.png'
    working = BASE / 'blender-cast' / name
    return working if working.is_file() else OUT / 'accepted/references' / name

def prepare():
    guides = OUT / 'guides'
    guides.mkdir(parents=True, exist_ok=True)
    for identity in ['field-hand', 'scribe', 'scout', 'defender']:
        sheet = Image.new('RGBA', (256, 256))
        for i, direction in enumerate(['front', 'right', 'back', 'left']):
            a = np.array(Image.open(reference_path(identity, direction)))
            a[:, :, 3] = np.where(a[:, :, 3] >= 128, 255, 0)
            a[a[:, :, 3] == 0, :3] = 0
            sheet.paste(Image.fromarray(a), ((i % 2) * 128 + 16, (i // 2) * 128 + 16))
        sheet.resize((1024, 1024), Image.Resampling.NEAREST).save(guides / (identity + '.png'))

def recover_turnaround(raw, identity):
    sys.path.insert(0, 'Z:/Code/Python/pixel-perfecter')
    from pixel_perfecter.reconstructor import PixelArtReconstructor
    im = Image.open(raw)
    if im.mode != 'RGBA' or im.getchannel('A').getextrema()[0] != 0:
        raise ValueError('Native returned alpha required')
    a = np.array(im)
    a[:, :, 3] = np.where(a[:, :, 3] >= 128, 255, 0)
    a[a[:, :, 3] == 0, :3] = 0
    rec = PixelArtReconstructor(image=a)
    rec.use_hough = False
    rec.run()
    pitch = round(im.width / 256)
    if abs(im.width / 256 - pitch) > .12:
        raise ValueError('Noninteger source pixel pitch')
    grid = next(g for g in rec.last_fit_debug['candidates'] if g['size'] == pitch)
    if grid['gated_out']:
        raise ValueError('Source grid variance gate failed')
    rec.cell_size, rec.offset = pitch, tuple(grid['offset'])
    recovered = Image.fromarray(rec._empirical_pixel_reconstruction())
    cells = []
    refs = []
    directions = ['front', 'right', 'back', 'left']
    for i, direction in enumerate(directions):
        x = round(-grid['offset'][0] / pitch) + (i % 2) * 128
        y = round(-grid['offset'][1] / pitch) + (i // 2) * 128
        cells.append(recovered.crop((x, y, x + 128, y + 128)))
        refs.append(np.array(Image.open(reference_path(identity, direction)))[:, :, 3] >= 128)
    # These are independent static views, not animation phases. Register each
    # view to its actual Blender projection, without moving the rig anchor or
    # fitting the visible bounds. Animation import retains its one-cycle shift.
    registrations = []
    for cell, mask in zip(cells, refs):
        best = (-1, 0, 0)
        for dy in range(-8, 9):
            for dx in range(-8, 9):
                b = np.array(cell.crop((16-dx, 16-dy, 112-dx, 112-dy)))[:, :, 3] >= 128
                score = np.count_nonzero(b & mask) / max(1, np.count_nonzero(b | mask))
                if score > best[0]:
                    best = (float(score), dx, dy)
        registrations.append(best)
    report = {'identity': identity, 'raw_sha256': hashlib.sha256(Path(raw).read_bytes()).hexdigest(),
              'frame': [96, 96], 'anchor': [48, 80], 'pixels_per_metre': 48,
              'grid': grid, 'mean_iou': float(np.mean([r[0] for r in registrations])),
              'static_view_registration': [list(r[1:]) for r in registrations],
              'iou': [r[0] for r in registrations], 'visual_acceptance': False,
              'reference_sha256': [hashlib.sha256(reference_path(identity, d).read_bytes()).hexdigest() for d in directions],
              'registration_policy': 'Independent static views matched to actual Blender silhouettes; no animation, rescaling or bbox fitting'}
    (OUT / 'review').mkdir(parents=True, exist_ok=True)
    (OUT / 'review' / (identity + '.json')).write_text(json.dumps(report, indent=2))
    if report['mean_iou'] < .70 or min(report['iou']) < .65:
        raise ValueError(f'Silhouette drift: {registrations}')
    pending = []
    preview = Image.new('RGBA', (384, 96), (36, 42, 35, 255))
    for i, (cell, direction) in enumerate(zip(cells, directions)):
        dx, dy = registrations[i][1:]
        frame = cell.crop((16-dx, 16-dy, 112-dx, 112-dy))
        p = np.array(frame)
        p[:, :, 3] = np.where(p[:, :, 3] >= 128, 255, 0)
        p[p[:, :, 3] == 0, :3] = 0
        frame = Image.fromarray(p)
        if np.count_nonzero(np.array(cell)[:, :, 3] >= 128) != np.count_nonzero(p[:, :, 3]):
            raise ValueError('Foreground clipping')
        box = frame.getbbox()
        if not box or min(box[:2]) == 0 or max(box[2:]) >= 96:
            raise ValueError('Empty or clipped frame')
        preview.alpha_composite(frame, (i * 96, 0))
        pending.append((frame, direction))
    (OUT / 'candidates').mkdir(exist_ok=True)
    (OUT / 'source').mkdir(exist_ok=True)
    for frame, direction in pending:
        frame.save(OUT / 'candidates' / f'{identity}-idle-{direction}.png')
    report['frame_sha256'] = [hashlib.sha256((OUT / 'candidates' / f'{identity}-idle-{d}.png').read_bytes()).hexdigest() for d in directions]
    (OUT / 'review' / (identity + '.json')).write_text(json.dumps(report, indent=2))
    (OUT / 'source' / f'{identity}.png').write_bytes(Path(raw).read_bytes())
    preview.resize((768, 192), Image.Resampling.NEAREST).save(OUT / 'review' / f'{identity}-2x.png')
    print(json.dumps(report))

def recover(raw, sheet):
    OUT.mkdir(parents=True, exist_ok=True)
    records = json.loads((BASE / 'guides/cast.json').read_text())[sheet]
    refs = []
    for row in records:
        p = BASE / 'blender-cast' / row['file']
        refs.append({'src': 'blender-cast/' + row['file'],
                     'sha256': hashlib.sha256(p.read_bytes()).hexdigest()})
    manifest = {'clips': {sheet: {'frames': refs, 'anchor': [48, 80]}}}
    manifest_path = BASE / ('npc-' + sheet + '-references.json')
    manifest_path.write_text(json.dumps(manifest, indent=2))
    first_slice_art.ROOT = OUT
    first_slice_art.recover(raw, sheet, BASE, Path('Z:/Code/Python/pixel-perfecter'), manifest_path.name)

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('raw', nargs='?'); p.add_argument('sheet', nargs='?')
    p.add_argument('--prepare', action='store_true')
    a = p.parse_args()
    if a.prepare: prepare()
    elif a.sheet in ['villagers', 'defense']: recover(a.raw, a.sheet)
    else: recover_turnaround(a.raw, a.sheet)
