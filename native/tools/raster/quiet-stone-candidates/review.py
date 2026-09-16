from pathlib import Path
import json, hashlib, sys
import numpy as np
from PIL import Image, ImageDraw

BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[3]
OUT=BASE/(sys.argv[1] if len(sys.argv)>1 else 'v1')
runtime=ROOT/'native/client/assets/raster/runtime'
sources=[('Existing stone',runtime/'terrain_stone.png'),('Quiet earth',runtime/'terrain_quiet_earth.png'),('Quiet stone candidate',OUT/'terrain_quiet_stone.png')]
hero=Image.open(runtime/'hero_sw.png').convert('RGBA')
wight=Image.open(runtime/'wight_sw.png').convert('RGBA')
report={'note':'Mechanical native pixel/repetition/actor composition only, not a production-game capture. All texels shown1:1: existing32px stone repeats6x6 across192px; quiet64px textures repeat3x3. This tests native detail density, not world-coordinate sampling. No input pixels repainted.','actor_pivots':[[60,152],[144,126]],'actor_scale':1,'patches':[]}
native=Image.new('RGB',(576,222),(34,32,29));draw=ImageDraw.Draw(native)
repeats=Image.new('RGB',(576,222),(34,32,29));rd=ImageDraw.Draw(repeats)
for i,(label,path) in enumerate(sources):
    im=Image.open(path).convert('RGBA');a=np.array(im);rgb=a[:,:,:3].astype(float)
    assert im.width==im.height and im.width in (32,64)
    tile=Image.new('RGBA',(192,192))
    for row in range(192//im.height):
        for col in range(192//im.width):tile.paste(im,(col*im.width,row*im.height))
    repeats.paste(tile,(i*192,24));rd.text((i*192+4,5),label,fill=(235,231,221))
    for actor,(x,y) in ((hero,(60,152)),(wight,(144,126))):tile.alpha_composite(actor,(x-40,y-96))
    native.paste(tile,(i*192,24));draw.text((i*192+4,5),label,fill=(235,231,221))
    lum=rgb@np.array([.2126,.7152,.0722]);dx=np.diff(lum,axis=1);dy=np.diff(lum,axis=0)
    edges={}
    for name,x,y in [('left_right',rgb[:,0],rgb[:,-1]),('top_bottom',rgb[0],rgb[-1])]:
        delta=np.abs(x-y)
        edges[name]={'exact_equal_pixels':int(np.all(x==y,axis=1).sum()),'total_pixels':len(x),'rgb_mae':float(delta.mean()),'rgb_difference_p95':float(np.percentile(delta,95)),'rgb_max_difference':float(delta.max())}
    report['patches'].append({'name':label,'path':path.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'size':list(im.size),'alpha_range':[int(a[:,:,3].min()),int(a[:,:,3].max())],'visible_colors':len(np.unique(a[:,:,:3][a[:,:,3]>0],axis=0)),'luminance_mean':float(lum.mean()),'luminance_std':float(lum.std()),'adjacent_luminance_difference_mean':float(np.r_[np.abs(dx).ravel(),np.abs(dy).ravel()].mean()),'adjacent_luminance_difference_p95':float(np.percentile(np.r_[np.abs(dx).ravel(),np.abs(dy).ravel()],95)),'opposite_edges':edges})
    if i==2:
        im.save(OUT/'native-1x.png');im.resize((192,192),Image.Resampling.NEAREST).save(OUT/'native-3x.png')
        repeat=Image.new('RGBA',(192,192))
        for row in range(3):
            for col in range(3):repeat.paste(im,(col*64,row*64))
        repeat.save(OUT/'repeat-3x3-1x.png');repeat.resize((576,576),Image.Resampling.NEAREST).save(OUT/'repeat-3x3-3x.png')
native.save(OUT/'actors-comparison-1x.png');native.resize((1152,444),Image.Resampling.NEAREST).save(OUT/'actors-comparison-2x.png')
repeats.save(OUT/'repeat-comparison-1x.png');repeats.resize((1152,444),Image.Resampling.NEAREST).save(OUT/'repeat-comparison-2x.png')
(OUT/'metrics.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report))
