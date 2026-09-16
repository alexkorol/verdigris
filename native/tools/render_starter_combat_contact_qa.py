"""Package normalized-phase attacks; preview FPS is not gameplay timing."""
from pathlib import Path
from PIL import Image,ImageDraw
import numpy as np,json,hashlib,sys
OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat'
for sex in ['male','female']:
 for action in (['unarmed-attack'] if 'fist' in sys.argv else ['attack','unarmed-attack']):
  dest=OUT/('contact-v5-fist' if 'fist' in sys.argv else 'contact-v4-repaired')/f'{sex}-{action}';source=OUT/'contact-v4-source'/f'{sex}-{action}'
  if not (dest/'repair.json').exists():continue
  repair=json.loads((dest/'repair.json').read_text());source_info=json.loads((source/'diagnosis.json').read_text());clips=[]
  for direction in ['front','right','back','left']:
   contact=Image.new('RGBA',(1024,256),(36,42,31,255));review=Image.new('RGBA',(1024,256),(36,42,31,255));animation=[];frames=[]
   for i in range(16):
    p=dest/'references'/f'{direction}-{i:02d}.png';im=Image.open(p).convert('RGBA');a=np.array(im);m=a[:,:,3]>=128;ys,xs=np.where(m)
    assert im.size==(128,128) and min(xs)>0 and min(ys)>0 and max(xs)<127 and max(ys)<127,p
    contact.alpha_composite(im,((i%8)*128,(i//8)*128));bg=Image.new('RGBA',(128,128),(36,42,31,255));bg.alpha_composite(im);animation.append(bg.resize((512,512),Image.Resampling.NEAREST).convert('RGB'))
    frames.append({'path':str(p.relative_to(OUT)).replace('\\','/'),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'index':i,'normalized_effect_phase':i/16,'role':'contact' if i==8 else 'preparation' if i<8 else 'recovery','source_pose':source_info['transition_landmarks'][i]['label']})
   a=np.array(Image.open(dest/'references'/f'{direction}-00.png'));b=np.array(Image.open(dest/'references'/f'{direction}-15.png'));assert np.array_equal(a,b),(sex,action,direction,'endpoint image mismatch')
   contact.resize((2048,512),Image.Resampling.NEAREST).save(dest/f'{direction}-contact.png');animation[0].save(dest/f'{direction}-preview.gif',save_all=True,append_images=animation[1:],duration=50,loop=0)
   clips.append({'sex':sex,'action':'club-attack' if action=='attack' else 'unarmed-attack','equipment':'branch-club' if action=='attack' else 'unarmed','direction':direction,'frame_size':[128,128],'anchor':[64,96],'pixels_per_metre':48,'playback_control':'normalized effect phase; frame=floor(phase*16), clamped0..15','contact_index':8,'normalized_contact_phase':.5,'preview_fps_only':20,'frames':frames})
  (dest/'manifest.json').write_text(json.dumps({'status':'source QA pending full visual review','gameplay_timing':'No gameplay changes. Native melee effect300ms, sweep400ms; contact normalized.5. Preview20fps is only a review aid.','source_motion':'Sword_Attack' if action=='attack' else 'Punch_Cross','clips':clips},indent=2));print(sex,action,'64 native frame bounds and4identicalidle endpoints passed')
