"""Build an alpha-only recipe from visually reviewed neutral gap components.

No source pixels are edited. The durable importer manifest calls the owner's
existing border-connected alpha API in exact neutral row runs.
"""
from pathlib import Path
import json, sys
import cv2, numpy as np
from PIL import Image

BASE = Path(__file__).resolve().parent
ROOT = BASE.parents[4]
sys.path.insert(0, str(ROOT / 'native/tools/raster'))
import import_assets
import_assets.load_engine(Path('Z:/Code/Python/pixel-perfecter'))
source = np.array(Image.open(ROOT / 'native/client/assets/raster/source/wight-walk-sw-v1.png').convert('RGBA'))
options = {'color': [210, 210, 255], 'tolerance': 70}
border, _ = import_assets.remove_background(source, options)
rgb = source[:, :, :3].astype(int)
neutral = (border[:, :, 3] > 0) & (rgb.min(2) >= 170) & ((rgb.max(2) - rgb.min(2)) <= 12)
_, labels, stats, _ = cv2.connectedComponentsWithStats(neutral.astype('uint8'), 8)
# These 20 components were individually viewed in neutral-gap-review.png.
# They are enclosed background between bones/arms/cloth, never bone highlights.
reviewed = [i for i, stat in enumerate(stats[1:], 1) if int(stat[4]) >= 20]
mask = np.isin(labels, reviewed)
windows = []
for y, row in enumerate(mask):
    edges = np.flatnonzero(np.diff(np.r_[False, row, False].astype(np.int8)))
    windows.extend([[int(a), y, int(b), y + 1] for a, b in zip(edges[::2], edges[1::2])])
options['interior_windows'] = windows
cleaned, cleanup = import_assets.remove_background(source, options)
local_removed = (border[:, :, 3] > 0) & (cleaned[:, :, 3] == 0)
assert not np.any(local_removed & ~mask)
assert np.max((rgb.max(2) - rgb.min(2))[local_removed]) <= 12
recipe = json.loads((BASE.parent / 'v1/import.json').read_text())
recipe['sheets'][0]['border_background'] = options
recipe['defaults']['acceptance'] = 'root_provisional_native_motion_review_pending_no_runtime_promotion'
destination = BASE.parent / 'selected'
destination.mkdir(exist_ok=True)
(destination / 'import.json').write_text(json.dumps(recipe, indent=2) + '\n')
record = {
    'source_untouched': True,
    'method': 'Actual Pixel Respecter border-connected fill; exact neutral-only row-run windows in 20 visually reviewed enclosed background components.',
    'reviewed_components': json.loads((BASE / 'neutral-components.json').read_text())['components'],
    'neutral_rule': 'RGB minimum170 and maximum channel spread12 after border fill; components>=20 source pixels, visually reviewed before selection.',
    'window_count': len(windows),
    'local_pixels_removed': int(local_removed.sum()),
    'local_chromatic_pixels_removed_above12': int((local_removed & ((rgb.max(2)-rgb.min(2)) > 12)).sum()),
    'border_chromatic_pixels_removed_above20': int(((border[:, :, 3] == 0) & ((rgb.max(2)-rgb.min(2)) > 20)).sum()),
    'foreground_caution': '12 chromatic matte-edge pixels removed by border fill, compared against source; old tolerance95 cleanup erased a pale hand and remains rejected in v1/.',
    'source_rgb_edits': 0,
}
(destination / 'alpha-recipe-review.json').write_text(json.dumps(record, indent=2) + '\n')
print(json.dumps({k:v for k,v in record.items() if k != 'reviewed_components'}))
