"""Candidate-source alpha diagnostics; neutral masks only locate review crops."""
from pathlib import Path
import json
import cv2
import numpy as np
from PIL import Image
from import_assets import load_engine,remove_background,DEFAULT_PROJECT

ROOT=Path(__file__).resolve().parent
OUT=ROOT/'motion-source-candidates'
OUT.mkdir(exist_ok=True)
load_engine(DEFAULT_PROJECT)
records=[]
for name,file,color,tolerance,region in [
    ('nw4','actors_hero-walk-nw-sheet-v1.frame4.retry.png',[173,173,173],95,None),
    ('strike4','actors_hero-strike-se-v2.png',[173,173,173],95,[440,687,780,1374])]:
    source=ROOT/'../../client/assets/raster/source'/file
    pixels=np.asarray(Image.open(source).convert('RGBA'))
    filled,background=remove_background(pixels,{'color':color,'tolerance':tolerance})
    rgb=filled[:,:,:3].astype(float)
    neutral=((rgb.max(2)-rgb.min(2))<10)&(rgb.mean(2)>100)&(filled[:,:,3]>0)
    if region:
        valid=np.zeros(neutral.shape,dtype=bool)
        valid[region[1]:region[3],region[0]:region[2]]=True
        neutral &= valid
    _,labels,stats,_=cv2.connectedComponentsWithStats(neutral.astype('uint8'),8)
    candidates=[]
    for x,y,w,h,area in stats[1:]:
        if area<40: continue
        box=[int(x),int(y),int(x+w),int(y+h)]
        candidates.append({'box':box,'area':int(area)})
        Image.fromarray(pixels).crop((x-12,y-12,x+w+12,y+h+12)).save(OUT/f'{name}-neutral-{len(candidates)}.png')
    records.append({'name':name,'background':background,'candidates':candidates})
    Image.fromarray(filled).save(OUT/f'{name}-border-filled.png')
(OUT/'diagnostics.json').write_text(json.dumps(records,indent=2))
print(json.dumps(records))
