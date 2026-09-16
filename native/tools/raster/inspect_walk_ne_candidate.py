"""Review-only NE v2 source diagnostics. Does not write the runtime library."""
from pathlib import Path
import json
import cv2
import numpy as np
from PIL import Image,ImageDraw
from import_assets import load_engine,remove_background,DEFAULT_PROJECT

ROOT=Path(__file__).resolve().parent
OUT=ROOT/'walk-ne-candidates'
OUT.mkdir(exist_ok=True)
source=ROOT/'../../client/assets/raster/source/actors_hero-walk-ne-sheet-v2.png'
load_engine(DEFAULT_PROJECT)
pixels=np.asarray(Image.open(source).convert('RGBA'))
filled,background=remove_background(pixels,{'color':[232,232,232],'tolerance':85})
rgb=filled[:,:,:3].astype(float)
neutral=((rgb.max(2)-rgb.min(2))<10)&(rgb.mean(2)>160)&(filled[:,:,3]>0)
_,labels,stats,_=cv2.connectedComponentsWithStats(neutral.astype('uint8'),8)
records=[]
for x,y,w,h,area in stats[1:]:
    if area<50: continue
    box=[int(x),int(y),int(x+w),int(y+h)]
    records.append({'box':box,'area':int(area)})
    Image.fromarray(pixels).crop((x-12,y-12,x+w+12,y+h+12)).save(OUT/f'neutral-{len(records)}.png')
Image.fromarray(filled).save(OUT/'border-filled.png')
(OUT/'diagnostics.json').write_text(json.dumps({'background':background,'candidates':records},indent=2))
print(json.dumps(records))
