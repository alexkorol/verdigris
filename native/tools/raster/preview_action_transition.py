"""Idle/action/idle review on unchanged native canvases and one fixed pivot."""
from pathlib import Path
import hashlib
import json

import numpy as np
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent
RUNTIME = ROOT / '../../client/assets/raster/runtime'


def main():
    catalog = json.loads((RUNTIME / 'catalog.json').read_text(encoding='utf-8'))
    entries = {entry['name']: entry for entry in catalog['assets']}
    names = ['hero_se'] + [f'hero_strike{i}_se' for i in range(6)] + ['hero_se']
    frames = []
    for name in names:
        path = RUNTIME / f'{name}.png'
        if hashlib.sha256(path.read_bytes()).hexdigest() != entries[name]['sha256']:
            raise ValueError(f'Active catalog hash mismatch: {name}')
        frames.append(Image.open(path).convert('RGBA'))
    if len({frame.size for frame in frames}) != 1:
        raise ValueError('Idle and strike must share the entire runtime canvas')
    if len({tuple(entries[name]['anchor_px']) for name in names}) != 1:
        raise ValueError('Idle and strike do not share the output pivot')
    width, height = frames[0].size
    scale, footer = 4, 26
    strip = Image.new('RGB', (width * scale * len(frames), height * scale + footer), (32, 31, 29))
    native = Image.new('RGB', (width * len(frames), height + 20), (32, 31, 29))
    grid = Image.new('RGB', (width * scale * 4, (height * scale + footer) * 2), (32, 31, 29))
    displayed, geometry = [], []
    for index, (name, frame) in enumerate(zip(names, frames)):
        enlarged = frame.resize((width * scale, height * scale), Image.Resampling.NEAREST)
        pane = Image.new('RGB', (width * scale, height * scale + footer), (32, 31, 29))
        pane.paste(enlarged, (0, 0), enlarged)
        draw = ImageDraw.Draw(pane)
        draw.line((0, height * scale, width * scale, height * scale), fill=(93, 103, 92))
        draw.text((6, height * scale + 8), name, fill=(230, 225, 208))
        displayed.append(pane)
        strip.paste(pane, (index * width * scale, 0))
        grid.paste(pane, ((index % 4) * width * scale, (index // 4) * (height * scale + footer)))
        native.paste(frame, (index * width, 0), frame)
        ImageDraw.Draw(native).text((index * width + 2, height + 4), 'idle' if name == 'hero_se' else name.replace('hero_', ''), fill=(230,225,208))
        mask = np.asarray(frame)[:, :, 3] > 0
        rows = []
        # Source-independent occupied-row spans expose head/feet displacement;
        # these are measurements of silhouettes, not anatomical labels.
        for y in range(height):
            xs = np.flatnonzero(mask[y])
            if len(xs): rows.append({'y': y, 'left': int(xs[0]), 'right_exclusive': int(xs[-1]) + 1})
        geometry.append({'name':name,'sha256':entries[name]['sha256'],
                         'visible_bounds':frame.getchannel('A').getbbox(),
                         'anchor_px':entries[name]['anchor_px'], 'occupied_row_spans': rows})
        top = rows[0]['y']
        crown = rows[:6]
        geometry[-1]['crown_band'] = {'top':top,'rows':6,
            'left':min(row['left'] for row in crown),
            'right_exclusive':max(row['right_exclusive'] for row in crown),
            'width':max(row['right_exclusive'] for row in crown)-min(row['left'] for row in crown)}
    native.save(ROOT / 'hero-idle-strike-idle-strip-1x.png')
    strip.save(ROOT / 'hero-idle-strike-idle-strip-4x.png')
    grid.save(ROOT / 'hero-idle-strike-idle-contact-4x.png')
    palette = strip.quantize(colors=256, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    indexed = [frame.quantize(palette=palette, dither=Image.Dither.NONE) for frame in displayed]
    duration = [400] + [50] * 6 + [400]
    indexed[0].save(ROOT / 'hero-idle-strike-idle-preview.gif', save_all=True,
                    append_images=indexed[1:], duration=duration, loop=0, optimize=False, disposal=2)
    (ROOT / 'hero-idle-strike-idle-review.json').write_text(json.dumps({
        'sequence':names,'duration_ms':duration,'canvas':[width,height],
        'placement':'Exact runtime canvas, fixed origin,4x nearest neighbor; no individual alignment, fitting or retiming of source geometry.',
        'acceptance':'transition_discontinuity_observed',
        'observations':[
            'Full height and ground pivot remain stable:63px idle/ready/recovery and62px extension, bottom row95.',
            'Strike crown band is13px wide versus10px idle; heavier dark head outline and wider guard stance appear instantly.',
            'Return to idle narrows the head and drops raised hands without a bridging pose; live motion acceptance remains pending.'
        ], 'frames':geometry}, indent=2) + '\n', encoding='utf-8')
    print('Wrote fixed-origin idle/strike0..5/idle native and4x previews,50ms action slots,400ms idle holds.')


if __name__ == '__main__':
    main()
