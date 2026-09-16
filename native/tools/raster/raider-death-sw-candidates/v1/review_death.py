from pathlib import Path
import json, hashlib
import numpy as np
from PIL import Image, ImageDraw

BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
frames=[Image.open(BASE/f'raider_death{i}_sw.png').convert('RGBA') for i in range(4)]
idle=Image.new('RGBA',(128,112))
idle_source=Image.open(ROOT/'native/client/assets/raster/runtime/raider_sw.png').convert('RGBA')
idle.paste(idle_source,(24,0))
idle.save(BASE/'idle-reference.png')
phases=['idle','recoil','buckle','fall','settled']
allframes=[idle,*frames]
for scale in (1,3):
    for light in (False,True):
        color=(180,165,141) if light else (32,31,29)
        strip=Image.new('RGB',(128*5*scale,112*scale+20),color)
        for i,im in enumerate(allframes):
            big=im.resize((128*scale,112*scale),Image.Resampling.NEAREST)
            strip.paste(big,(i*128*scale,0),big)
            ImageDraw.Draw(strip).text((i*128*scale+5,112*scale+3),phases[i],fill=(20,18,16) if light else (235,230,220))
        strip.save(BASE/f'idle-death-{scale}x{"-light" if light else ""}.png')
contact=Image.new('RGB',(768,712),(32,31,29))
for i,im in enumerate(frames):
    big=im.resize((384,336),Image.Resampling.NEAREST)
    x,y=(i%2)*384,(i//2)*356
    contact.paste(big,(x,y),big)
    ImageDraw.Draw(contact).text((x+5,y+338),f'{i} {phases[i+1]}',fill=(235,230,220))
contact.save(BASE/'contact-3x.png')
panes=[]
for im in allframes:
    p=Image.new('RGB',(384,336),(32,31,29))
    big=im.resize(p.size,Image.Resampling.NEAREST);p.paste(big,(0,0),big);panes.append(p)
pal=Image.open(BASE/'idle-death-3x.png').quantize(colors=256,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE)
indexed=[p.quantize(palette=pal,dither=Image.Dither.NONE) for p in panes]
durations=[400,100,130,110,1000]
indexed[0].save(BASE/'idle-death-preview.gif',save_all=True,append_images=indexed[1:],duration=durations,loop=0,optimize=False,disposal=2)
report={'canvas':[128,112],'pivot':[64,96],'idle_translation':[24,0],'idle_body_height':idle_source.getchannel('A').getbbox()[3]-idle_source.getchannel('A').getbbox()[1],'shared_cell':8,'preview_durations_ms':durations,'source_alpha':'RGB opaque checker; cleanup required','production_accepted':False,'frames':[]}
for i,im in enumerate(frames):
    a=np.asarray(im);rgb=a[:,:,:3].astype(int);neutral=(a[:,:,3]>0)&(rgb.min(2)>170)&((rgb.max(2)-rgb.min(2))<25)
    y,x=np.where(neutral)
    report['frames'].append({'phase':phases[i+1],'bounds':im.getchannel('A').getbbox(),'sha256':hashlib.sha256((BASE/f'raider_death{i}_sw.png').read_bytes()).hexdigest(),'partial_alpha':int(((a[:,:,3]>0)&(a[:,:,3]<255)).sum()),'bright_neutral_positions':[[int(xx),int(yy)] for yy,xx in zip(y,x)]})
(BASE/'native-review.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report))
