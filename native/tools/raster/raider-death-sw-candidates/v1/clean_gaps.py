from pathlib import Path
import sys,json,shutil
import numpy as np
from PIL import Image
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
TOOLS=ROOT/'native/tools/raster'
sys.path.insert(0,str(TOOLS))
import import_assets
import_assets.load_engine(Path('Z:/Code/Python/pixel-perfecter'))
manifest_path=TOOLS/'raider-death-sw-candidate.json'
manifest=json.loads(manifest_path.read_text())
source=np.array(Image.open(ROOT/'native/client/assets/raster/source/raider-death-sw-v1.png').convert('RGBA'))
border,_=import_assets.remove_background(source,{'color':[210,210,210],'tolerance':95})
scopes=json.loads((BASE/'gap-scopes.json').read_text())
windows=[]; audit=[]
for box in scopes:
    x0,y0,x1,y1=box
    region=border[y0:y1,x0:x1];rgb=region[:,:,:3].astype(float)
    neutral=(region[:,:,3]>0)&(np.linalg.norm(rgb-210,axis=2)<=95)&((rgb.max(2)-rgb.min(2))<=20)&(rgb.min(2)>=170)
    runs={}
    for row in range(neutral.shape[0]):
        cols=np.flatnonzero(neutral[row])
        if not len(cols):continue
        for run in np.split(cols,np.where(np.diff(cols)>1)[0]+1):
            a,b=int(run[0]),int(run[-1])+1;key=(a,b)
            if key in runs and runs[key][3]==y0+row:runs[key][3]+=1
            else:
                rect=[x0+a,y0+row,x0+b,y0+row+1];windows.append(rect);runs[key]=rect
    audit.append({'scope':box,'removed_neutral_pixels':int(neutral.sum())})
archive=BASE/'border-only'
if not archive.exists():
    archive.mkdir()
    for name in [*[f'raider_death{i}_sw.png' for i in range(4)],'raider-death-sw-candidate.provenance.json','native-review.json','contact-3x.png']:
        shutil.copy2(BASE/name,archive/name)
    shutil.copy2(manifest_path,archive/'manifest.json')
manifest['sheets'][0]['border_background']['interior_windows']=windows
manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
cleaned,_=import_assets.remove_background(source,manifest['sheets'][0]['border_background'])
removed=(border[:,:,3]>0)&(cleaned[:,:,3]==0)
rgb=source[:,:,:3].astype(int)
chromatic=int((removed&((rgb.max(2)-rgb.min(2))>20)).sum())
assert chromatic==0
data={'method':'Exact neutral-only row-run rectangles inside nine visually reviewed background-gap scopes; no painted foreground changes.','windows':len(windows),'removed_pixels':int(removed.sum()),'chromatic_pixels_removed_by_local_cleanup':chromatic,'scopes':audit,'preserved_border_only_failure':'border-only/contact-3x.png'}
(BASE/'alpha-audit.json').write_text(json.dumps(data,indent=2)+'\n')
print(json.dumps(data))
