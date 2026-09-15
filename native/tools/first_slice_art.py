"""Recover painted Blender sheets without per-frame resizing or recentering.

The source frame geometry is authoritative. Generated art remains a candidate
until native-scale and animation review; the report is not visual approval.
"""
import argparse
import hashlib
import json
from pathlib import Path
import sys

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'


def recover(raw, clip, reference_root, respect_root, manifest_name='manifest-linen.json', registration='sheet'):
    sys.path.insert(0, str(respect_root))
    from pixel_perfecter.reconstructor import PixelArtReconstructor
    raw = Path(raw)
    refroot = Path(reference_root)
    manifest = json.loads((refroot / manifest_name).read_text())
    refs = manifest['clips'][clip]['frames']
    for folder in ('source', 'candidates', 'review'):
        (ROOT / folder).mkdir(parents=True, exist_ok=True)
    source = Image.open(raw)
    if source.mode != 'RGBA' or source.size != (1536, 1024):
        raise ValueError('Expected original 1536x1024 RGBA PNG')
    data = np.array(source)
    alpha = data[:, :, 3]
    if alpha.min() != 0 or alpha.max() < 250:
        raise ValueError('Native transparent and opaque alpha required')
    destination = ROOT / 'source' / (clip + '.png')
    destination.write_bytes(raw.read_bytes())
    data[:, :, 3] = np.where(alpha >= 128, 255, 0)
    data[data[:, :, 3] == 0, :3] = 0
    rec = PixelArtReconstructor(image=data)
    rec.use_hough = False
    rec.run()
    report = {'clip': clip, 'source_sha256': hashlib.sha256(raw.read_bytes()).hexdigest(),
              'frame_size': [96, 96], 'anchor': manifest['clips'][clip].get('anchor', [48, 80]), 'base_cell': 48,
              'alpha_policy': 'Returned alpha threshold 128; no RGB key',
              'grid_candidates': rec.last_fit_debug['candidates'], 'visual_acceptance': False,
              'frames': []}
    # Match the authored fourfold grid. Never resize a detector output to 96.
    candidate = next(c for c in rec.last_fit_debug['candidates'] if c['size'] == 4)
    if candidate['gated_out']:
        raise ValueError('Fourfold grid fails Pixel Respecter variance gate')
    rec.cell_size = 4
    rec.offset = tuple(candidate['offset'])
    recovered = Image.fromarray(rec._empirical_pixel_reconstruction())
    recovered.save(ROOT / 'review' / (clip + '-recovered.png'))
    report['selected_grid'] = candidate
    # The default is one translation for the whole cycle. Opt-in row recovery
    # corrects a shared sheet-layout offset for four poses together. Neither
    # policy can recenter individual poses, resize a figure, or erase gait bob.
    ox, oy = candidate['offset']
    slots = []
    masks = []
    for i, ref in enumerate(refs):
        x = round(-ox / 4) + (i % 4) * 96
        y = round(-oy / 4) + (i // 4) * 128
        slots.append(recovered.crop((x, y, x + 96, y + 128)))
        masks.append(np.array(Image.open(refroot / ref['src']).convert('RGBA'))[:, :, 3] >= 128)
    if registration not in ('sheet', 'row'):
        raise ValueError('Registration must be sheet or row; per-frame fitting is forbidden')
    groups = [list(range(len(slots)))] if registration == 'sheet' else [list(range(j, min(j+4, len(slots)))) for j in range(0, len(slots), 4)]
    translations = [None] * len(slots)
    registrations = []
    for group in groups:
        best = (-1, 0, 0, [])
        for dy in range(-12, 13):
            for dx in range(-12, 13):
                scores = []
                for j in group:
                    a = np.array(slots[j].crop((-dx, 16-dy, 96-dx, 112-dy)))[:, :, 3] >= 128
                    scores.append(np.count_nonzero(a & masks[j]) / max(1, np.count_nonzero(a | masks[j])))
                score = sum(scores) / len(scores)
                if score > best[0]:
                    best = (score, dx, dy, scores)
        score, dx, dy, scores = best
        registrations.append({'frames': group, 'translation': [dx, dy], 'mean_iou': score, 'frame_iou': scores})
        for j in group:
            translations[j] = (dx, dy)
    report['registration_policy'] = registration
    report['registrations'] = registrations
    report['whole_cycle_translation'] = list(translations[0]) if registration == 'sheet' else None
    report['registered_mean_iou'] = sum(r['mean_iou']*len(r['frames']) for r in registrations)/len(slots)
    if any(r['mean_iou'] < .70 or min(r['frame_iou']) < .65 for r in registrations):
        (ROOT / 'review' / (clip + '-rejected.json')).write_text(json.dumps(report, indent=2))
        raise ValueError(f'Silhouette drift after {registration} registration: mean IoU={report["registered_mean_iou"]:.3f}')
    preview = Image.new('RGBA', (768, 384), (36, 42, 35, 255))
    pending = []
    for i, ref in enumerate(refs):
        shift_x, shift_y = translations[i]
        refpath = refroot / ref['src']
        if hashlib.sha256(refpath.read_bytes()).hexdigest() != ref['sha256']:
            raise ValueError('Changed Blender reference')
        # Fixed slot geometry and shared sheet/row origin; no per-pose shifts.
        frame = slots[i].crop((-shift_x, 16-shift_y, 96-shift_x, 112-shift_y))
        pixels = np.array(frame)
        pixels[:, :, 3] = np.where(pixels[:, :, 3] >= 128, 255, 0)
        pixels[pixels[:, :, 3] == 0, :3] = 0
        frame = Image.fromarray(pixels)
        box = frame.getbbox()
        if not box or box[0] == 0 or box[2] >= 96 or box[1] == 0 or box[3] >= 96:
            raise ValueError(f'{clip} phase {i}: empty or clipped frame')
        if np.count_nonzero(np.array(slots[i])[:, :, 3] >= 128) != np.count_nonzero(pixels[:, :, 3]):
            raise ValueError(f'{clip} phase {i}: registration would discard foreground pixels')
        output = ROOT / 'candidates' / f'{clip}-{i:02d}.png'
        pending.append((frame, output))
        refpixels = np.array(Image.open(refpath).convert('RGBA'))
        a, b = pixels[:, :, 3] > 0, refpixels[:, :, 3] > 0
        union = np.count_nonzero(a | b)
        frame_iou = np.count_nonzero(a & b) / max(1, union)
        if frame_iou < .65:
            raise ValueError(f'{clip} phase {i}: silhouette drift IoU={frame_iou:.3f}')
        report['frames'].append({'file': output.name, 'bbox': box,
            'reference': ref, 'silhouette_iou': np.count_nonzero(a & b) / union,
            'slot_origin': [(i % 4) * 96, (i // 4) * 128],
            'registration_translation': [shift_x, shift_y], 'output_anchor': report['anchor']})
        preview.alpha_composite(frame.resize((192, 192), Image.Resampling.NEAREST),
                                ((i % 4) * 192, (i // 4) * 192))
    # Publish candidates only after every frame passes the same checks.
    for (frame, output), entry in zip(pending, report['frames']):
        frame.save(output)
        entry['sha256'] = hashlib.sha256(output.read_bytes()).hexdigest()
    preview.save(ROOT / 'review' / (clip + '-2x.png'))
    (ROOT / 'review' / (clip + '.json')).write_text(json.dumps(report, indent=2))
    print(json.dumps({'clip': clip, 'grid': candidate,
        'iou': [round(f['silhouette_iou'], 3) for f in report['frames']],
        'preview': str(ROOT / 'review' / (clip + '-2x.png'))}))


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('raw')
    p.add_argument('clip')
    p.add_argument('--reference-root', required=True)
    p.add_argument('--respect-root', required=True)
    p.add_argument('--manifest', default='manifest-linen.json')
    args = p.parse_args()
    recover(args.raw, args.clip, args.reference_root, args.respect_root, args.manifest)
