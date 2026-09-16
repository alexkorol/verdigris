from pathlib import Path
import hashlib,json
import numpy as np
from PIL import Image,ImageDraw
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
frames=[Image.open(BASE/f'raider_walk{i}_ne.png').convert('RGBA') for i in range(6)]
idle=Image.open(ROOT/'native/client/assets/raster/runtime/raider_ne.png').convert('RGBA');idle.save(BASE/'idle-reference.png')
panels=[idle,*frames,idle]
for scale in (1,3):
    for light in (False,True):
        c=Image.new('RGB',(80*len(panels)*scale,96*scale+20),(180,165,141) if light else (32,31,29));d=ImageDraw.Draw(c)
        for i,im in enumerate(panels):
            big=im.resize((80*scale,96*scale),Image.Resampling.NEAREST);c.paste(big,(i*80*scale,0),big);d.text((i*80*scale+5,96*scale+4),'idle' if i in (0,7) else str(i-1),fill=(20,18,16) if light else (230,225,215))
        c.save(BASE/f'idle-walk-idle-{scale}x{"-light" if light else ""}.png')
contact=Image.new('RGB',(720,616),(32,31,29));d=ImageDraw.Draw(contact)
for i,im in enumerate(frames):
    big=im.resize((240,288),Image.Resampling.NEAREST);x,y=(i%3)*240,(i//3)*308;contact.paste(big,(x,y),big);d.text((x+5,y+290),str(i),fill=(230,225,215))
contact.save(BASE/'contact-3x.png')
palette=Image.open(BASE/'idle-walk-idle-3x.png').quantize(colors=256,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE)
indexed=[]
for im in panels:
    p=Image.new('RGB',(240,288),(32,31,29));big=im.resize(p.size,Image.Resampling.NEAREST);p.paste(big,(0,0),big);indexed.append(p.quantize(palette=palette,dither=Image.Dither.NONE))
indexed[0].save(BASE/'idle-walk-idle-preview.gif',save_all=True,append_images=indexed[1:],duration=[300,*([100]*6),300],loop=0,optimize=False,disposal=2)
indexed[1].save(BASE/'walk-preview.gif',save_all=True,append_images=indexed[2:7],duration=100,loop=0,optimize=False,disposal=2)
data={'canvas':[80,96],'pivot':[40,96],'source_origin':[315,483],'source_column_stride':455,'source_row_stride':490,'shared_cell_size':7,'idle_bbox':idle.getchannel('A').getbbox(),'preview_frame_ms':100,'frames':[]}
colors=set()
for i,im in enumerate(frames):
    a=np.array(im);rgb=a[:,:,:3].astype(int);colors.update(map(tuple,a[:,:,:3][a[:,:,3]>0]));neutral=(a[:,:,3]>0)&(rgb.min(2)>100)&((rgb.max(2)-rgb.min(2))<10);yy,xx=np.where(neutral)
    data['frames'].append({'name':f'raider_walk{i}_ne','bounds':im.getchannel('A').getbbox(),'partial_alpha_pixels':int(((a[:,:,3]>0)&(a[:,:,3]<255)).sum()),'neutral_pixels':[[int(x),int(y)] for y,x in zip(yy,xx)],'sha256':hashlib.sha256((BASE/f'raider_walk{i}_ne.png').read_bytes()).hexdigest()})
data['shared_colors']=len(colors)
(BASE/'native-review.json').write_text(json.dumps(data,indent=2)+'\n')
print(json.dumps(data))
