"""Package and inspect versioned transition refs without altering sprite pixels."""
from pathlib import Path
from PIL import Image, ImageDraw
import numpy as np
import json,hashlib
OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat'
for sex in ['male','female']:
 dest=OUT/'transition-v2'/sex
 if not (dest/'diagnosis.json').exists():continue
 diagnosis=json.loads((dest/'diagnosis.json').read_text());clips=[];checks=[]
 for direction in ['front','right','back','left']:
  files=sorted((dest/'references').glob(f'{direction}-*.png'))
  if len(files)!=16:continue
  contact=Image.new('RGBA',(128*8,128*2),(36,42,31,255));animation=[];frames=[]
  for i,p in enumerate(files):
   im=Image.open(p).convert('RGBA');a=np.array(im);assert im.size==(128,128)
   mask=a[:,:,3]>=128;ys,xs=np.where(mask)
   assert len(xs) and min(xs)>0 and min(ys)>0 and max(xs)<127 and max(ys)<127,p
   contact.alpha_composite(im,((i%8)*128,(i//8)*128))
   frame=Image.new('RGBA',(128,128),(36,42,31,255));frame.alpha_composite(im)
   animation.append(frame.resize((512,512),Image.Resampling.NEAREST).convert('RGB'))
   frames.append({'path':str(p.relative_to(OUT)).replace('\\','/'),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'phase':i,'duration_ms':50,'source_phase':i-4 if 4<=i<=11 else None})
   if 4<=i<=11:
    ref=np.array(Image.open(OUT/'references'/f'{sex}-attack-{direction}-{i-4:02d}.png').convert('RGBA'))
    checks.append({'direction':direction,'phase':i,'source_phase':i-4,'changed_pixels':int(np.any(ref!=a,axis=2).sum()),'mask_xor_pixels':int(((ref[:,:,3]>=128)!=mask).sum())})
  contact.resize((2048,512),Image.Resampling.NEAREST).save(dest/f'{direction}-contact.png')
  animation[0].save(dest/f'{direction}-loop.gif',save_all=True,append_images=animation[1:],duration=50,loop=0)
  clips.append({'sex':sex,'action':'club-attack','direction':direction,'frame_size':[128,128],'anchor':[64,96],'pixels_per_metre':48,'duration_ms':800,'frames':frames})
 (dest/'manifest.json').write_text(json.dumps({'status':'source-candidate; requires cloth/skin clearance review before final projection','clips':clips,'interior_rerender_comparison':checks,'notes':['Original eight input files are untouched. Skeletal samples and cloth keys are preserved; minute raster differences may arise from floating-point skeletal matrix conversion.','Entry/recovery uses quaternion pose blends and floor height correction. Horizontal foot planting is not constrained.','Foreground bounds checked at native scale; no resizing or framewise registration.']},indent=2))
 print(sex,'frames',sum(len(c['frames']) for c in clips),'max interior mask XOR',max(c['mask_xor_pixels'] for c in checks))

dest=OUT/'transition-v2/male';j=json.loads((dest/'diagnosis.json').read_text());result={}
contact=Image.new('RGBA',(512,128),(36,42,31,255))
for k,i in enumerate([1,2]):
 ref=Image.open(OUT/f'references/male-attack-front-{i:02d}.png').convert('RGBA')
 paint=Image.open(OUT/f'recovery/male-attack-front-0-3-imagegen-v1/male-club-attack-front-{i:02d}.png').convert('RGBA')
 for t,im in enumerate([ref,paint]):
  contact.alpha_composite(im,((k*2+t)*128,0));d=ImageDraw.Draw(contact)
  for name,col in [('pelvis','red'),('spine_03','yellow'),('head','cyan')]:
   x,y=j['source_attack_landmarks'][i][name]['pixel'];x+=(k*2+t)*128
   d.line((x-3,y,x+3,y),fill=col);d.line((x,y-3,x,y+3),fill=col)
contact.resize((2048,512),Image.Resampling.NEAREST).save(dest/'phase1-2-landmarks.png')
for name,(rx,yt,yb) in {'head':(8,-9,5),'spine_03':(10,-5,5),'pelvis':(12,-3,6)}.items():
 reg=[]
 for i in [1,2]:
  x,y=j['source_attack_landmarks'][i][name]['pixel'];x=round(x);y=round(y);roi=(x-rx,y+yt,x+rx+1,y+yb+1);cs=[]
  for p in [OUT/f'references/male-attack-front-{i:02d}.png',OUT/f'recovery/male-attack-front-0-3-imagegen-v1/male-club-attack-front-{i:02d}.png']:
   a=np.array(Image.open(p).convert('RGBA').crop(roi))[:,:,3]>=128;yy,xx=np.where(a);cs.append([float(xx.mean()+roi[0]),float(yy.mean()+roi[1])])
  reg.append({'phase':i,'roi':roi,'reference_centroid':cs[0],'paint_centroid':cs[1]})
 err=(np.array(reg[1]['paint_centroid'])-reg[0]['paint_centroid'])-(np.array(reg[1]['reference_centroid'])-reg[0]['reference_centroid'])
 result[name]={'samples':reg,'motion_error_px':float(np.linalg.norm(err))}
(dest/'phase1-2-regions.json').write_text(json.dumps({'method':'Alpha centroids in windows anchored to projected Blender bones. These are region diagnostics, not automatically detected anatomical landmarks.','regions':result},indent=2))
