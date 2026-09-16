from pathlib import Path
import json,sys
import cv2,numpy as np
from PIL import Image,ImageDraw
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
sys.path.insert(0,str(ROOT/'native/tools/raster'))
import import_assets
import_assets.load_engine(Path('Z:/Code/Python/pixel-perfecter'))
path=ROOT/'native/client/assets/raster/source/wight-walk-sw-v1.png'
original=Image.open(path)
source=np.array(original.convert('RGBA'))
cleaned,cleanup=import_assets.remove_background(source,{'color':[210,210,210],'tolerance':95})
rgb=source[:,:,:3].astype(int)
removed=cleaned[:,:,3]==0
chromatic=removed&((rgb.max(2)-rgb.min(2))>20)
n,labels,stats,centers=cv2.connectedComponentsWithStats(chromatic.astype('uint8'),8)
components=[{'box':[int(x),int(y),int(x+w),int(y+h)],'pixels':int(area)} for x,y,w,h,area in stats[1:] if area>=3]
gallery=Image.new('RGB',(1600,620),(180,165,141));draw=ImageDraw.Draw(gallery)
for i in range(8):
    x0=(i%4)*405;y0=0 if i<4 else 500
    box=(x0+70,y0+(80 if i<4 else 40),min(x0+330,1619),min(y0+480,971))
    for j,arr in enumerate((source,cleaned)):
        im=Image.fromarray(arr).crop(box);im.thumbnail((190,280),Image.Resampling.NEAREST)
        x=(i%4)*400+j*200;y=(i//4)*310;gallery.paste(im,(x,y+20),im);draw.text((x+4,y+3),f'{i} '+('source' if j==0 else 'cleaned'),fill=(20,18,16))
gallery.save(BASE/'alpha-source-comparison.png')
data={'source_mode':original.mode,'source_dimensions':list(original.size),'source_true_transparent_pixels':0,'source_partial_alpha_pixels':0,'failure':'Source requested real alpha but returned an opaque painted checker.','cleanup':cleanup,'removed_chromatic_pixels':int(chromatic.sum()),'removed_chromatic_components_at_least3px':components,'foreground_review':'Compare source and cleaned bones/fingers/cloth before acceptance; chromatic matte-edge pixels are not automatically proof of anatomy loss.','local_gap_windows':0}
(BASE/'alpha-audit.json').write_text(json.dumps(data,indent=2)+'\n')
print(json.dumps({'removed_chromatic_pixels':int(chromatic.sum()),'components':components}))
