"""Package repaired native refs and high-resolution torso review sheets."""
from PIL import Image
from pathlib import Path
import numpy as np,json,hashlib
OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat'
for dest in (OUT/'transition-v3-skin').iterdir():
 if not dest.is_dir() or not (dest/'repair.json').exists():continue
 repair=json.loads((dest/'repair.json').read_text());sex=repair.get('sex',dest.name);action=repair.get('action','attack');count=repair.get('frame_count',16);size=repair['frame_size'][0];old=OUT/'transition-v2'/sex
 clips=[];changes=[]
 for direction in ['front','right','back','left']:
  contact=Image.new('RGBA',(size*count,size),(36,42,31,255));review=Image.new('RGBA',(704,200*((count+3)//4)),(36,42,31,255));animation=[];frames=[]
  for i in range(count):
   p=dest/'references'/f'{direction}-{i:02d}.png';im=Image.open(p).convert('RGBA');a=np.array(im);m=a[:,:,3]>=128;ys,xs=np.where(m)
   assert min(xs)>0 and min(ys)>0 and max(xs)<size-1 and max(ys)<size-1
   contact.alpha_composite(im,(size*i,0));hi=Image.open(dest/'inspection'/f'{direction}-{i:02d}.png').convert('RGBA');review.alpha_composite(hi.crop((104,64,280,264)),((i%4)*176,(i//4)*200))
   bg=Image.new('RGBA',(size,size),(36,42,31,255));bg.alpha_composite(im);animation.append(bg.resize((size*4,size*4),Image.Resampling.NEAREST).convert('RGB'))
   prevpath=sorted((old/'references').glob(f'{direction}-{i:02d}-*.png'))[0] if action=='attack' else OUT/'references'/f'{sex}-{action}-{direction}-{i:02d}.png'
   previous=np.array(Image.open(prevpath).convert('RGBA'));delta=np.abs(a.astype(int)-previous.astype(int));changes.append({'direction':direction,'phase':i,'alpha_pixels_added':int((m&(previous[:,:,3]<128)).sum()),'alpha_pixels_removed':int(((~m)&(previous[:,:,3]>=128)).sum()),'rgba_pixels_changed_gt4':int((delta.max(axis=2)>4).sum())})
   frames.append({'path':str(p.relative_to(OUT)).replace('\\','/'),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'phase':i,'duration_ms':50})
  contact.save(dest/f'{direction}-native-contact.png');review.save(dest/f'{direction}-torso-review.png');animation[0].save(dest/f'{direction}-loop.gif',save_all=True,append_images=animation[1:],duration=50,loop=0)
  clips.append({'sex':sex,'action':'club-'+action,'direction':direction,'frame_size':[size,size],'anchor':repair['anchor'],'pixels_per_metre':48,'duration_ms':count*50,'frames':frames})
 (dest/'manifest.json').write_text(json.dumps({'status':'source repair candidate; see repair and visual QA','clips':clips,'changes_from_transition_v2':changes},indent=2))
 print(sex,action,'native reference bounds passed; changed mask range',min(x['alpha_pixels_added'] for x in changes),max(x['alpha_pixels_added'] for x in changes))
