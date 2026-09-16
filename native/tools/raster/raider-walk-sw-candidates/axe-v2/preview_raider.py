from pathlib import Path
import hashlib,json,sys
import numpy as np
from PIL import Image,ImageDraw

BASE=Path(__file__).resolve().parent
TOOLS=BASE.parents[1]
root=TOOLS.parents[2]
report=json.loads((BASE/"raider-walk-sw-axe-v2.provenance.json").read_text())
records={r["name"]:r for r in report["assets"]}
names=[f"raider_walk{i}_sw" for i in range(8)]
frames=[Image.open(BASE/f"{name}.png").convert("RGBA") for name in names]
idle_path=root/"native/client/assets/raster/runtime/raider_sw.png"
idle=Image.open(idle_path).convert("RGBA")
for name in names:
 assert hashlib.sha256((BASE/f"{name}.png").read_bytes()).hexdigest()==records[name]["sha256"]
assert all(records[n]["before_resize"]==records[n]["placed_size"] for n in names)
panels=[idle,*frames]
native=Image.new("RGB",(80*9,116),(32,31,29))
large=Image.new("RGB",(80*4*9,96*4+24),(32,31,29))
light=Image.new("RGB",(80*4*9,96*4+24),(185,171,148))
for i,im in enumerate(panels):
 native.paste(im,(i*80,0),im)
 label="idle" if i==0 else str(i-1)
 ImageDraw.Draw(native).text((i*80+5,99),label,fill=(230,225,208))
 big=im.resize((320,384),Image.Resampling.NEAREST)
 for canvas in (large,light):
  canvas.paste(big,(i*320,0),big)
  ImageDraw.Draw(canvas).text((i*320+5,390),label,fill=(230,225,208) if canvas is large else (30,25,20))
native.save(BASE/"idle-and-cycle-1x.png")
large.save(BASE/"idle-and-cycle-4x.png")
light.save(BASE/"idle-and-cycle-light-4x.png")
def pane(im):
 p=Image.new("RGB",(320,384),(32,31,29));b=im.resize((320,384),Image.Resampling.NEAREST);p.paste(b,(0,0),b);return p
gif_frames=[pane(im) for im in frames]
palette=large.quantize(colors=256,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE)
indexed=[im.quantize(palette=palette,dither=Image.Dither.NONE) for im in gif_frames]
indexed[0].save(BASE/"cycle-preview.gif",save_all=True,append_images=indexed[1:],duration=80,loop=0,optimize=False,disposal=2)
transitions=[pane(idle),*gif_frames,*gif_frames,pane(idle)]
ti=[im.quantize(palette=palette,dither=Image.Dither.NONE) for im in transitions]
ti[0].save(BASE/"idle-transition-preview.gif",save_all=True,append_images=ti[1:],duration=[400]+[80]*16+[400],loop=0,optimize=False,disposal=2)
metrics={"canvas":[80,96],"cell_size":6,"shared_colors":report["shared_palette"]["visible_colors_after"],"idle_bbox":idle.getchannel("A").getbbox(),"idle_sha256":hashlib.sha256(idle_path.read_bytes()).hexdigest(),"unique_frames":len({records[n]["sha256"] for n in names}),"frames":[],"timing_ms":80,"geometry":"Common source grid and row origin; no independent frame resampling or alignment"}
for i,(name,im) in enumerate(zip(names,frames)):
 a=np.asarray(im);b=np.asarray(frames[(i+1)%8]);rgb=a[:,:,:3].astype(int)
 bright=(a[:,:,3]>0)&(rgb.min(2)>170)&((rgb.max(2)-rgb.min(2))<25)
 yy,xx=np.where(bright)
 metrics["frames"].append({"name":name,"bounds":im.getchannel("A").getbbox(),"changed_pixels_to_next":int(np.count_nonzero(np.any(a!=b,axis=2))),"partial_alpha":int(np.count_nonzero((a[:,:,3]>0)&(a[:,:,3]<255))),"bright_neutral_pixels_below_y68":[[int(x),int(y)] for y,x in zip(yy,xx) if y>68]})
(BASE/"cycle-review.json").write_text(json.dumps(metrics,indent=2)+"\n")
idle.save(BASE/"idle-reference.png")
html="""<!doctype html><meta charset="utf-8"><title>Raider SW axe repair candidate review</title><style>body{background:#201f1d;color:#eee;font:16px system-ui}canvas{image-rendering:pixelated;border:1px solid #5d675c;margin:16px}button,input{margin:8px}p{max-width:800px}</style><h1>Raider SW axe repair candidate</h1><button id="play">Pause</button><button id="mode">Idle transition: on</button><input id="phase" type="range" min="-1" max="7" value="-1"><span id="status"></span><br><canvas id="native" width="80" height="96"></canvas><canvas id="large" width="320" height="384"></canvas><script>const paths=['idle-reference.png',...Array.from({length:8},(_,i)=>'raider_walk'+i+'_sw.png')];const imgs=paths.map(p=>{let a=new Image();a.src=p;return a});let playing=true,transition=true,start=performance.now(),manual=-1;function draw(f){for(let id of ['native','large']){const c=document.getElementById(id),x=c.getContext('2d');x.imageSmoothingEnabled=false;x.fillStyle='#201f1d';x.fillRect(0,0,c.width,c.height);if(imgs[f+1].complete)x.drawImage(imgs[f+1],0,0,c.width,c.height)}document.getElementById('status').textContent=f<0?'IDLE':'Frame '+f;document.getElementById('phase').value=f}function tick(t){let f=manual;if(playing){let elapsed=t-start;if(transition){let a=elapsed%2080;f=a<400||a>=1680?-1:Math.floor((a-400)/80)%8}else f=Math.floor(elapsed/80)%8}draw(f);requestAnimationFrame(tick)}document.getElementById('play').onclick=()=>{playing=!playing;document.getElementById('play').textContent=playing?'Pause':'Play';start=performance.now()};document.getElementById('mode').onclick=()=>{transition=!transition;document.getElementById('mode').textContent='Idle transition: '+(transition?'on':'off');start=performance.now()};document.getElementById('phase').oninput=e=>{manual=Number(e.target.value);playing=false;document.getElementById('play').textContent='Play'};requestAnimationFrame(tick)</script>"""
(BASE/"preview.html").write_text(html)
print(json.dumps(metrics,indent=2))

