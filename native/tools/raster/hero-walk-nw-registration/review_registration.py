"""Fixed-origin NW registration comparison; no frame fitting or pixel resampling."""
from pathlib import Path
import hashlib
import json
import shutil

import numpy as np
from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
RUNTIME = HERE.parents[2] / 'client/assets/raster/runtime'
NAMES = [f'hero_walk{i}_nw' for i in range(8)]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def panel(image, label):
    # Same actor-to-ground proportions as main.cpp: height 1.75 tiles,
    # team ellipse radius .32 tiles, with vertical radius .30 of horizontal.
    tile = Image.new('RGB', (100, 128), (57, 49, 40))
    draw = ImageDraw.Draw(tile)
    draw.ellipse((32, 95, 68, 105), outline=(170, 183, 145))
    draw.line((48, 100, 52, 100), fill=(130, 143, 126))
    tile.paste(image, (10, 4), image)
    draw.text((3, 112), label, fill=(236, 228, 209))
    return tile


def main():
    baseline = HERE / 'baseline'
    candidate = HERE / 'candidate'
    baseline.mkdir(exist_ok=True)
    for name in ['hero_nw', *NAMES]:
        if not (baseline / f'{name}.png').exists():
            shutil.copy2(RUNTIME / f'{name}.png', baseline / f'{name}.png')
    before = [Image.open(baseline / f'{n}.png').convert('RGBA') for n in NAMES]
    after = [Image.open(candidate / f'{n}.png').convert('RGBA') for n in NAMES]
    idle = Image.open(baseline / 'hero_nw.png').convert('RGBA')
    rows = []
    for i, (old, new) in enumerate(zip(before, after)):
        dy = 1 if i < 4 else 5
        expected = np.zeros((96, 80, 4), dtype=np.uint8)
        expected[dy:] = np.asarray(old)[:-dy]
        assert np.array_equal(expected, np.asarray(new)), f'{NAMES[i]} changed beyond translation'
        assert np.count_nonzero(np.asarray(old)[:, :, 3]) == np.count_nonzero(np.asarray(new)[:, :, 3])
        assert set(np.unique(np.asarray(new)[:, :, 3])).issubset({0, 255})
        rows.append({'name': NAMES[i], 'dy_native': dy, 'old_bounds': old.getbbox(),
                     'new_bounds': new.getbbox(), 'sole_gap_before': 96-old.getbbox()[3],
                     'sole_gap_after': 96-new.getbbox()[3],
                     'before_sha256': sha(baseline/f'{NAMES[i]}.png'),
                     'after_sha256': sha(candidate/f'{NAMES[i]}.png'),
                     'exact_rgba_translation_no_clipping': True})
    sequences = [[idle, *before, idle], [idle, *after, idle]]
    labels = ['idle', *[f'walk{i}' for i in range(8)], 'idle']
    native = Image.new('RGB', (1000, 256))
    for r, seq in enumerate(sequences):
        for i, (frame, label) in enumerate(zip(seq, labels)):
            native.paste(panel(frame, label+(' before' if r == 0 else ' after')), (100*i, 128*r))
    native.save(HERE/'idle-cycle-idle-before-after-1x.png')
    native.resize((4000, 1024), Image.Resampling.NEAREST).save(HERE/'idle-cycle-idle-before-after-4x.png')
    # Single aligned sequence with a persistent ellipse, plus before/after loop.
    gif = []
    for index, label in enumerate(labels):
        pair = Image.new('RGB', (200, 128))
        for row in range(2):
            pair.paste(panel(sequences[row][index], label+(' before' if row == 0 else ' after')), (100*row, 0))
        gif.append(pair.resize((800, 512), Image.Resampling.NEAREST))
    gif[0].save(HERE/'idle-cycle-idle-before-after.gif', save_all=True, append_images=gif[1:],
                duration=[400]+[80]*8+[400], loop=0, disposal=2)
    grid = Image.new('RGB', (500, 256))
    for i, (frame, label) in enumerate(zip(sequences[1], labels)):
        grid.paste(panel(frame, label+' after'), ((i%5)*100, (i//5)*128))
    grid.resize((1500, 768), Image.Resampling.NEAREST).save(HERE/'idle-cycle-idle-after-contact-3x.png')
    report = {'baseline_commit':'f3318b059', 'clip_canvas':[80,96], 'grid_pitch_source':6,
              'old_anchor_source':[192,486], 'new_anchor_source_by_sheet_row':[[192,480],[192,456]],
              'translation_explanation':'Both rows receive1px clip root calibration; row1 receives4px additional sheet-packing correction. No individual pose fitting.',
              'idle_bounds':idle.getbbox(), 'idle_sha256':sha(baseline/'hero_nw.png'),
              'runtime_formula':'top=base.y-height; native pixel edge y maps to base.y+(y-96)*height/96',
              'frame019_asset':'hero_walk6_nw', 'frame019_old_gap_native':6, 'frame019_new_gap_native':1,
              'review_scope':'Asset registration preview only; new production capture and equipment socket shifts remain integration gates.',
              'preserved_limitations':['Walk body65..69px versus idle63px; redder tunic, wider stance and body shape drift remain.',
                                       'Head bob remains: corrected crown27..29px versus idle33px; no scaling or individual head alignment applied.',
                                       'Residual sole travel0..2px is retained, including lifted/crossing feet.'],
              'frames':rows}
    (HERE/'registration-review.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'exact_translation_checks':len(rows),'sole_gaps_before':[r['sole_gap_before'] for r in rows],
                      'sole_gaps_after':[r['sole_gap_after'] for r in rows], 'preview':str(HERE/'idle-cycle-idle-after-contact-3x.png')}, indent=2))


if __name__ == '__main__':
    main()
