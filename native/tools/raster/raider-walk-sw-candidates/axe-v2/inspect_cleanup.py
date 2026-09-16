from pathlib import Path
import json,sys
import cv2
import numpy as np
from PIL import Image

BASE=Path(__file__).resolve().parent
TOOLS=BASE.parents[1]
ROOT=TOOLS.parents[2]
sys.path.insert(0,str(TOOLS))
import import_assets
import_assets.load_engine(Path('Z:/Code/Python/pixel-perfecter'))
manifest_path=TOOLS/'raider-walk-sw-axe-v2-candidate.json'
manifest=json.loads(manifest_path.read_text())
source=np.array(Image.open(ROOT/'native/client/assets/raster/source/raider-walk-sw-axe-v2.png').convert('RGBA'))
cleaned,_=import_assets.remove_background(source,{'color':[210,210,210],'tolerance':95})
# Only these visually inspected hair-loop and axe/knee gaps are eligible.
# The mask/face and material highlights are excluded from every scope.
scopes=[]
for i in range(8):
    c,r=i%4,i//4
    scopes.append({'kind':'hair loop background','frame':i,'box':[205+c*406,138+r*435,240+c*406,220+r*435]})
scopes.extend([{'kind':'axe/knee background','frame':4,'box':[110,770,135,805]},
               {'kind':'axe/knee background','frame':5,'box':[520,780,534,805]}])
windows=[]
for scope in scopes:
    x0,y0,x1,y1=scope['box']
    region=cleaned[y0:y1,x0:x1]
    distances=np.linalg.norm(region[:,:,:3].astype(float)-210,axis=2)
    neutral=((distances<=95)&(region[:,:,3]>0)).astype(np.uint8)
    count,labels,stats,_=cv2.connectedComponentsWithStats(neutral,connectivity=4)
    for x,y,w,h,area in stats[1:]:
        window=[int(x0+x),int(y0+y),int(x0+x+w),int(y0+y+h)]
        windows.append(window)
    scope['retained_background_pixels']=int(neutral.sum())
manifest['sheets'][0]['border_background']['interior_windows']=windows
manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
(BASE/'cleanup-inspection.json').write_text(json.dumps({'scopes':scopes,'windows':windows,'selection':'Connected neutral components within visually reviewed hair-loop and axe/knee gaps only; used as explicit local fill windows by the existing importer.'},indent=2)+'\n')
print(json.dumps({'windows':windows,'pixels':sum(s['retained_background_pixels'] for s in scopes)}))
