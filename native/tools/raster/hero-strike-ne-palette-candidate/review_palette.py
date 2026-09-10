"""Compare the fixed native NE sequence before/after reference palette snapping."""
from pathlib import Path
import hashlib
import json

import numpy as np
from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
MANIFEST = HERE.parent / 'hero-strike-ne-palette-candidate.json'


def main():
    manifest = json.loads(MANIFEST.read_bytes())
    provenance = json.loads((HERE / 'hero-strike-ne-palette-candidate.provenance.json').read_bytes())
    reference = np.array(provenance['palette_reference']['palette_rgb'], dtype=np.uint8)
    palette = {tuple(color) for color in reference}
    before, after, rows = [], [], []
    labels = ['ready', 'windup', 'commit', 'contact', 'followthrough', 'recovery']
    for i, sheet in enumerate(manifest['sheets']):
        src = (MANIFEST.parent / sheet['source']).resolve()
        dst = HERE / f'hero_strike{i}_ne.png'
        original = Image.open(src).convert('RGBA')
        snapped = Image.open(dst).convert('RGBA')
        a, b = np.asarray(original), np.asarray(snapped)
        assert original.size == snapped.size == (80, 96)
        assert np.array_equal(a[:, :, 3], b[:, :, 3])
        assert (b[b[:, :, 3] == 0, :3] == 0).all()
        mask = a[:, :, 3] > 0
        old_colors = {tuple(color) for color in np.unique(a[:, :, :3][mask], axis=0)}
        new_colors = {tuple(color) for color in np.unique(b[:, :, :3][mask], axis=0)}
        assert new_colors <= palette
        assert provenance['assets'][i]['anchor_px'] == [40, 96]
        sha = hashlib.sha256(dst.read_bytes()).hexdigest()
        assert sha == provenance['assets'][i]['sha256']
        difference = np.abs(a[:, :, :3].astype(int) - b[:, :, :3].astype(int))[mask]
        rows.append({'phase': labels[i], 'file': dst.name, 'source': sheet['source'],
                     'source_sha256': hashlib.sha256(src.read_bytes()).hexdigest(), 'sha256': sha,
                     'visible_pixels': int(mask.sum()),
                     'changed_visible_pixels': int(np.count_nonzero(np.any(difference > 0, axis=1))),
                     'mean_absolute_rgb_channel_change': float(difference.mean()),
                     'visible_colors_before': len(old_colors), 'visible_colors_after': len(new_colors),
                     'colors_outside_reference_before': len(old_colors - palette),
                     'colors_outside_reference_after': len(new_colors - palette),
                     'alpha_identical': True, 'source_bounds': original.getbbox(),
                     'candidate_bounds': snapped.getbbox(), 'anchor': [40, 96],
                     'canvas': [80, 96], 'partial_alpha_pixels': int(((b[:, :, 3]>0)&(b[:, :, 3]<255)).sum())})
        before.append(original)
        after.append(snapped)
    assert np.array_equal(np.asarray(before[0]), np.asarray(after[0])), 'Ready idle identity changed'
    def panel(frame, label):
        art = Image.new('RGB', (80, 114), (72, 69, 64))
        art.paste(frame, (0, 0), frame)
        draw = ImageDraw.Draw(art)
        draw.line((0, 96, 79, 96), fill=(125, 133, 116))
        draw.text((2, 101), label, fill=(235, 229, 214))
        return art
    # The same original canvas and alpha origin are used in every pane.
    sequences = [before + [before[0]], after + [after[0]]]
    captions = labels + ['idle exit']
    strip = Image.new('RGB', (560, 228))
    for row, sequence in enumerate(sequences):
        for col, (frame, label) in enumerate(zip(sequence, captions)):
            strip.paste(panel(frame, label), (col*80, row*114))
    strip.save(HERE / 'before-after-sequence-1x.png')
    strip.resize((1680, 684), Image.Resampling.NEAREST).save(HERE / 'before-after-sequence-3x.png')
    pairs = []
    for col, label in enumerate(captions):
        art = Image.new('RGB', (160, 114))
        for side in range(2):
            art.paste(panel(sequences[side][col], label), (side*80, 0))
        pairs.append(art.resize((480, 342), Image.Resampling.NEAREST))
    timing = [350, 100, 100, 100, 100, 100, 350]
    pairs[0].save(HERE / 'before-after-review.gif', save_all=True, append_images=pairs[1:],
                  duration=timing, loop=0, disposal=2)
    metrics = {'acceptance': 'candidate_only_pending_root_review',
               'view_layout': 'Top row before; bottom row snapped. All native 80x96 canvases at identical origin; 3x nearest-neighbor is inspection only.',
               'review_gif_layout': 'Left before, right snapped; 100ms action holds are slow review timing, not a production timing claim.',
               'reference': provenance['palette_reference'], 'ready_rgba_identical': True,
               'all_alpha_and_geometry_identical': True, 'frames': rows,
               'material_review_limit': 'Exact palette membership does not prove the same material receives the same reference color. Anatomy, body size and pose timing are unchanged.'}
    (HERE / 'palette-review.json').write_bytes((json.dumps(metrics, indent=2)+'\n').encode('utf-8'))
    print(json.dumps({'reference_colors': len(palette), 'ready_unchanged': True,
                      'changed_visible_pixels': [r['changed_visible_pixels'] for r in rows],
                      'after_color_counts': [r['visible_colors_after'] for r in rows]}, indent=2))


if __name__ == '__main__':
    main()
