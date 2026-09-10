"""Diagnostic only: actual connected fill and reviewed source crops for inspection."""
from pathlib import Path
import json
import numpy as np
import cv2
from PIL import Image, ImageDraw
from import_assets import load_engine, remove_background, DEFAULT_PROJECT

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / '../../client/assets/raster/source'
OUT = ROOT / 'hero-direction-candidates'
OUT.mkdir(exist_ok=True)
load_engine(DEFAULT_PROJECT)
records = []
for direction, color, tolerance in [('sw', [231,231,231], 75), ('ne',[179,179,179],90), ('nw',[170,170,170],90)]:
    original = np.asarray(Image.open(SOURCE / f'actors_hero-{direction}-idle-v1.png').convert('RGBA'))
    filled, report = remove_background(original, {'color':color, 'tolerance':tolerance})
    Image.fromarray(filled).save(OUT / f'{direction}-border-filled.png')
    # The neutral pixels below are ONLY diagnostic candidates; they are not erased.
    rgb = filled[:,:,:3].astype(float)
    neutral = ((rgb.max(axis=2)-rgb.min(axis=2)) < 10) & (rgb.mean(axis=2)>100) & (filled[:,:,3]>0)
    count, labels, stats, centers = cv2.connectedComponentsWithStats(neutral.astype('uint8'), 8)
    boxes=[]
    for x,y,w,h,area in stats[1:]:
        if area < 12: continue
        box=[int(x),int(y),int(x+w),int(y+h)]
        boxes.append({'box':box,'area':int(area)})
        crop=Image.fromarray(original).crop((max(0,x-12),max(0,y-12),min(original.shape[1],x+w+12),min(original.shape[0],y+h+12)))
        crop.save(OUT / f'{direction}-candidate-{len(boxes)}.png')
    records.append({'direction':direction,'border':report,'neutral_candidates':boxes})
(OUT/'diagnostics.json').write_text(json.dumps(records,indent=2))
print(json.dumps(records,indent=2))
