from pathlib import Path
import json,sys
import cv2
import numpy as np
from PIL import Image

BASE=Path(__file__).resolve().parent
TOOLS=BASE.parents[1]
sys.path.insert(0,str(TOOLS))
import import_assets
import_assets.load_engine(Path('Z:/Code/Python/pixel-perfecter'))
manifest_path=TOOLS/'raider-strike-sw-candidate.json'
manifest=json.loads(manifest_path.read_text())
# Source crops in source-gap-inspection.png were visually reviewed first.
# These scopes cover background inside hair loops and recovery arm gaps,
# excluding bone mask and hand/forearm highlights.
scopes={
    0:[[260,120,324,156],[292,156,323,206]],
    1:[[738,126,806,161],[773,161,808,205]],
    2:[[1245,126,1319,158],[1283,158,1318,206]],
    3:[[247,633,309,666],[279,666,311,709]],
    4:[[754,631,818,664],[791,664,819,708]],
    5:[[1234,625,1318,659],[1287,659,1320,708],[1208,764,1256,828],[1340,770,1370,820]]
}
by_source={}
for sheet in manifest['sheets']:
    source_path=sheet['source']
    if source_path in by_source:
        continue
    phases=[1] if source_path.endswith('v2.png') else [0,2,3,4,5]
    source=np.array(Image.open((TOOLS/source_path).resolve()).convert('RGBA'))
    cleaned,_=import_assets.remove_background(source,{'color':[210,210,210],'tolerance':95})
    windows=[];inspected=[]
    for phase in phases:
        for box in scopes[phase]:
            x0,y0,x1,y1=box;region=cleaned[y0:y1,x0:x1]
            rgb=region[:,:,:3].astype(float)
            neutral=(np.linalg.norm(rgb-210,axis=2)<=95)&((rgb.max(2)-rgb.min(2))<=20)&(region[:,:,3]>0)
            # Exact neutral-only row runs prevent a bounding rectangle from
            # also selecting pale colored skin at the edge of a background gap.
            runs={}
            for row in range(neutral.shape[0]):
                columns=np.flatnonzero(neutral[row])
                if len(columns)==0:
                    continue
                splits=np.split(columns,np.where(np.diff(columns)>1)[0]+1)
                for run in splits:
                    left,right=int(run[0]),int(run[-1])+1
                    key=(left,right)
                    if key in runs and runs[key][3]==y0+row:
                        runs[key][3]=y0+row+1
                    else:
                        rectangle=[x0+left,y0+row,x0+right,y0+row+1]
                        windows.append(rectangle);runs[key]=rectangle
            inspected.append({'phase':phase,'scope':box,'retained_background_pixels':int(neutral.sum())})
    by_source[source_path]={'windows':windows,'inspected_scopes':inspected,'selection':'Exact rectangles of inspected neutral pixels only; channel spread at most20. Pale chromatic skin retained.'}
for sheet in manifest['sheets']:
    sheet['border_background']['interior_windows']=by_source[sheet['source']]['windows']
manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
(BASE/'cleanup-inspection.json').write_text(json.dumps(by_source,indent=2)+'\n')
print(json.dumps({key:{'windows':len(value['windows']),'background_pixels':sum(s['retained_background_pixels'] for s in value['inspected_scopes'])} for key,value in by_source.items()}))
