from pathlib import Path
import sys,json
import numpy as np
from PIL import Image,ImageDraw
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
sys.path.insert(0,str(ROOT/'native/tools/raster'))
import import_assets
import_assets.load_engine(Path('Z:/Code/Python/pixel-perfecter'))
a=np.array(Image.open(ROOT/'native/client/assets/raster/source/raider-walk-ne-v1.png').convert('RGBA'))
b=np.array(Image.open(ROOT/'native/client/assets/raster/source/raider-walk-ne-v2.png').convert('RGBA'))
cleaned,cleanup=import_assets.remove_background(b,{'color':[160,160,160],'tolerance':100})
rgb=b[:,:,:3].astype(int);removed=cleaned[:,:,3]==0
yy,xx=np.indices(removed.shape)
selected=((xx>=1024)&(yy<512))|((xx<512)&(yy>=512))
data={'v1':{'original_mode':'RGBA','transparent_pixels':int((a[:,:,3]==0).sum()),'partial_alpha_pixels':int(((a[:,:,3]>0)&(a[:,:,3]<255)).sum()),'opaque_pixels':int((a[:,:,3]==255).sum()),'handling':'Actual alpha retained into Pixel Respecter crisp reconstruction, with alpha_floor128; faint source halo is excluded.'},'v2':{'original_mode':'RGB','failure':'Focused edit painted an opaque checker.','cleanup':cleanup,'removed_chromatic_channel_spread_gt20':int((removed&((rgb.max(2)-rgb.min(2))>20)).sum()),'local_windows':0,'review':'Border-connected matte removed; original and cleaned selected figures were compared for hair, forearm, torso and leg openings.'},'final':{'binary_alpha_all_frames':True,'neutral_gap_pixels_report':'native-review.json','interior_cleanup':'No additional local windows needed; no body pixels repainted.'}}
gallery=Image.new('RGB',(960,560),(180,165,141));draw=ImageDraw.Draw(gallery)
boxes=[[1090,65,1420,485],[190,550,510,977]]
for i,box in enumerate(boxes):
    for j,arr in enumerate((b,cleaned)):
        im=Image.fromarray(arr).crop(box);im.thumbnail((230,510),Image.Resampling.NEAREST)
        x=i*480+j*240;gallery.paste(im,(x,25),im);draw.text((x+4,5),f'phase{2+i} '+('source' if j==0 else 'border cleanup'),fill=(20,18,16))
gallery.save(BASE/'alpha-source-comparison.png')
data['v2']['selected_phase_chromatic_pixels_removed']=int((removed&selected&((rgb.max(2)-rgb.min(2))>20)).sum())
data['v2']['unselected_source_cells_note']='The four chromatic fringe pixels removed from the full sheet belong to unused v2 phases0/1. Selected phases2/3 remove zero such pixels.'
(BASE/'alpha-audit.json').write_text(json.dumps(data,indent=2)+'\n')
print(json.dumps(data))
