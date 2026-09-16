from pathlib import Path
import json,sys
import cv2,numpy as np
from PIL import Image,ImageDraw
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
sys.path.insert(0,str(ROOT/'native/tools/raster'))
import import_assets
import_assets.load_engine(Path('Z:/Code/Python/pixel-perfecter'))
source=np.array(Image.open(ROOT/'native/client/assets/raster/source/wight-walk-sw-v1.png').convert('RGBA'))
cleaned,record=import_assets.remove_background(source,{'color':[210,210,255],'tolerance':70})
rgb=source[:,:,:3].astype(int)
neutral=(cleaned[:,:,3]>0)&(rgb.min(2)>=170)&((rgb.max(2)-rgb.min(2))<=12)
n,labels,stats,centers=cv2.connectedComponentsWithStats(neutral.astype('uint8'),8)
components=[(k,[int(x),int(y),int(x+w),int(y+h)],int(area)) for k,(x,y,w,h,area) in enumerate(stats[1:],1) if area>=20]
cols=8;rows=(len(components)+cols-1)//cols
gallery=Image.new('RGB',(cols*180,rows*150),(180,165,141));draw=ImageDraw.Draw(gallery)
for i,(k,box,count) in enumerate(components):
    x0,y0,x1,y1=box;crop=[max(0,x0-6),max(0,y0-6),min(source.shape[1],x1+6),min(source.shape[0],y1+6)]
    im=Image.fromarray(source).crop(crop);im.thumbnail((170,115),Image.Resampling.NEAREST)
    if im.width<85 and im.height<57:im=im.resize((im.width*2,im.height*2),Image.Resampling.NEAREST)
    x,y=(i%cols)*180,(i//cols)*150;gallery.paste(im,(x,y+32),im);draw.text((x+3,y+2),f'{i} {box}\n{count} neutral pixels',fill=(20,18,16))
gallery.save(BASE/'neutral-gap-review.png')
(BASE/'neutral-components.json').write_text(json.dumps({'mask':'Remaining alpha, RGB minimum170, maximum channel spread12','components':[{'id':i,'box':b,'pixels':c} for i,(_,b,c) in enumerate(components)]},indent=2)+'\n')
print(json.dumps({'neutral_pixels':int(neutral.sum()),'components':len(components)}))
