from pathlib import Path
import json,hashlib
import numpy as np
from PIL import Image,ImageDraw
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
frames=[Image.open(BASE/f'wight_walk{i}_sw.png').convert('RGBA') for i in range(8)]
idle=Image.open(ROOT/'native/client/assets/raster/runtime/wight_sw.png').convert('RGBA');idle.save(BASE/'idle-reference.png')
panels=[idle,*frames,idle]
for scale in (1,3):
    for light in (False,True):
        canvas=Image.new('RGB',(80*len(panels)*scale,96*scale+20),(180,165,141) if light else (32,31,29));draw=ImageDraw.Draw(canvas)
        for i,im in enumerate(panels):
            big=im.resize((80*scale,96*scale),Image.Resampling.NEAREST);canvas.paste(big,(i*80*scale,0),big);draw.text((i*80*scale+5,96*scale+3),'idle' if i in (0,9) else str(i-1),fill=(20,18,16) if light else (230,225,215))
        canvas.save(BASE/f'idle-cycle-idle-{scale}x{"-light" if light else ""}.png')
contact=Image.new('RGB',(960,616),(32,31,29));draw=ImageDraw.Draw(contact)
for i,im in enumerate(frames):
    big=im.resize((240,288),Image.Resampling.NEAREST);x,y=(i%4)*240,(i//4)*308;contact.paste(big,(x,y),big);draw.text((x+5,y+290),str(i),fill=(230,225,215))
contact.save(BASE/'contact-3x.png')
palette=Image.open(BASE/'idle-cycle-idle-3x.png').quantize(colors=256,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE)
indexed=[]
for im in panels:
    p=Image.new('RGB',(240,288),(32,31,29));big=im.resize(p.size,Image.Resampling.NEAREST);p.paste(big,(0,0),big);indexed.append(p.quantize(palette=palette,dither=Image.Dither.NONE))
indexed[0].save(BASE/'idle-cycle-idle.gif',save_all=True,append_images=indexed[1:],duration=[320,*([100]*8),320],loop=0,disposal=2,optimize=False)
indexed[1].save(BASE/'walk-loop.gif',save_all=True,append_images=indexed[2:9],duration=100,loop=0,disposal=2,optimize=False)
metrics={'canvas':[80,96],'pivot':[40,96],'cell_size':6,'source_origin':[210,486],'column_stride':405,'row_stride':450,'reference_idle_bounds':idle.getchannel('A').getbbox(),'palette_reference':'runtime/wight_sw.png','frame_ms':100,'frames':[]}
colors=set()
for i,im in enumerate(frames):
    a=np.array(im);colors.update(map(tuple,a[:,:,:3][a[:,:,3]>0]));metrics['frames'].append({'name':f'wight_walk{i}_sw','bounds':im.getchannel('A').getbbox(),'sha256':hashlib.sha256((BASE/f'wight_walk{i}_sw.png').read_bytes()).hexdigest(),'partial_alpha_pixels':int(((a[:,:,3]>0)&(a[:,:,3]<255)).sum())})
metrics['shared_colors']=len(colors)
(BASE/'native-review.json').write_text(json.dumps(metrics,indent=2)+'\n')
print(json.dumps(metrics))
