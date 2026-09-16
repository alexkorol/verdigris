from pathlib import Path
import hashlib,json
import numpy as np
from PIL import Image,ImageDraw

BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
names=[f'raider_strike{i}_sw' for i in range(6)]
labels=['ready','anticipation','windup','contact','follow-through','recovery']
durations=[100,120,90,50,100,140]
report=json.loads((BASE/'raider-strike-sw-candidate.provenance.json').read_text())
records={r['name']:r for r in report['assets']}
frames=[Image.open(BASE/f'{n}.png').convert('RGBA') for n in names]
idle=Image.open(ROOT/'native/client/assets/raster/runtime/raider_sw.png').convert('RGBA')
for name in names:
    assert hashlib.sha256((BASE/f'{name}.png').read_bytes()).hexdigest()==records[name]['sha256']
    assert records[name]['before_resize']==records[name]['placed_size']
    assert records[name]['anchor_px']==[40,96]
panels=[idle,*frames,idle]
strip=Image.new('RGB',(80*len(panels),116),(32,31,29))
large=Image.new('RGB',(320*len(panels),408),(32,31,29))
light=Image.new('RGB',large.size,(185,171,148))
for i,im in enumerate(panels):
    label='idle' if i in (0,len(panels)-1) else str(i-1)
    strip.paste(im,(80*i,0),im);ImageDraw.Draw(strip).text((80*i+4,100),label,fill='#ddd9ca')
    big=im.resize((320,384),Image.Resampling.NEAREST)
    for canvas in (large,light):
        canvas.paste(big,(320*i,0),big);ImageDraw.Draw(canvas).text((320*i+4,390),label,fill='#e0dcca' if canvas is large else '#211e19')
strip.save(BASE/'idle-strike-idle-1x.png');large.save(BASE/'idle-strike-idle-4x.png');light.save(BASE/'idle-strike-idle-light-4x.png')
contact=Image.new('RGB',(3*320,2*408),(32,31,29))
for i,im in enumerate(frames):
    big=im.resize((320,384),Image.Resampling.NEAREST);x,y=(i%3)*320,(i//3)*408;contact.paste(big,(x,y),big);ImageDraw.Draw(contact).text((x+6,y+390),f'{i} {labels[i]}',fill='#ddd9ca')
contact.save(BASE/'contact-4x.png')
def pane(im):
    p=Image.new('RGB',(320,384),(32,31,29));b=im.resize((320,384),Image.Resampling.NEAREST);p.paste(b,(0,0),b);return p
palette=large.quantize(colors=256,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE)
indexed=[pane(im).quantize(palette=palette,dither=Image.Dither.NONE) for im in frames]
indexed[0].save(BASE/'strike-preview.gif',save_all=True,append_images=indexed[1:],duration=durations,loop=0,optimize=False,disposal=2)
idle_p=pane(idle).quantize(palette=palette,dither=Image.Dither.NONE)
idle_p.save(BASE/'idle-strike-idle-preview.gif',save_all=True,append_images=[*indexed,idle_p],duration=[350,*durations,350],loop=0,optimize=False,disposal=2)
metrics={'canvas':[80,96],'anchor':[40,96],'cell_size':6,'shared_colors':report['shared_palette']['visible_colors_after'],'idle_bbox':idle.getchannel('A').getbbox(),'unique_frames':len({records[n]['sha256'] for n in names}),'preview_durations_ms':durations,'preview_contact_phase':3,'production_timing':'Not assigned; integration owner must synchronize contact with actual confirmed hit event.','frames':[]}
for i,(name,im) in enumerate(zip(names,frames)):
    a=np.asarray(im);b=np.asarray(frames[(i+1)%6]);rgb=a[:,:,:3].astype(int);bright=(a[:,:,3]>0)&(rgb.min(2)>170)&((rgb.max(2)-rgb.min(2))<25);yy,xx=np.where(bright)
    metrics['frames'].append({'name':name,'phase':labels[i],'bounds':im.getchannel('A').getbbox(),'changed_pixels_to_next':int(np.count_nonzero(np.any(a!=b,axis=2))),'partial_alpha':int(np.count_nonzero((a[:,:,3]>0)&(a[:,:,3]<255))),'bright_neutral_pixels_below_y65':[[int(x),int(y)] for y,x in zip(yy,xx) if y>65]})
(BASE/'cycle-review.json').write_text(json.dumps(metrics,indent=2)+'\n')
idle.save(BASE/'idle-reference.png')
html='''<!doctype html><meta charset="utf-8"><title>Raider SW strike candidate</title><style>body{background:#201f1d;color:#eee;font:16px system-ui}canvas{image-rendering:pixelated;border:1px solid #5d675c;margin:16px}button,input{margin:8px}</style><h1>Raider SW strike candidate</h1><button id="play">Pause</button><input id="phase" type="range" min="-1" max="5" value="-1"><span id="status"></span><br><canvas id="native" width="80" height="96"></canvas><canvas id="large" width="320" height="384"></canvas><script>const paths=['idle-reference.png',...Array.from({length:6},(_,i)=>'raider_strike'+i+'_sw.png')];const imgs=paths.map(p=>{let a=new Image();a.src=p;return a});const labels=['idle','ready','anticipation','windup','contact','follow-through','recovery'];const phases=[-1,0,1,2,3,4,5,-1],duration=[350,100,120,90,50,100,140,350];let playing=true,start=performance.now(),manual=-1;function draw(f){for(let id of ['native','large']){const c=document.getElementById(id),x=c.getContext('2d');x.imageSmoothingEnabled=false;x.fillStyle='#201f1d';x.fillRect(0,0,c.width,c.height);if(imgs[f+1].complete)x.drawImage(imgs[f+1],0,0,c.width,c.height)}document.getElementById('status').textContent=f<0?'IDLE':'Frame '+f+' '+labels[f+1];document.getElementById('phase').value=f}function tick(t){let f=manual;if(playing){let a=(t-start)%1300;for(let i=0;i<phases.length;i++){if(a<duration[i]){f=phases[i];break}a-=duration[i]}}draw(f);requestAnimationFrame(tick)}document.getElementById('play').onclick=()=>{playing=!playing;document.getElementById('play').textContent=playing?'Pause':'Play';start=performance.now()};document.getElementById('phase').oninput=e=>{manual=Number(e.target.value);playing=false;document.getElementById('play').textContent='Play'};requestAnimationFrame(tick)</script>'''
(BASE/'preview.html').write_text(html)
print(json.dumps(metrics,indent=2))
