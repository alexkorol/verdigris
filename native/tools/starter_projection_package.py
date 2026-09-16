"""Export unchanged native projected pixels using the game's binary-alpha policy."""
from pathlib import Path
from PIL import Image,ImageDraw
import numpy as np,json,hashlib
import sys
sex='female' if '--female' in sys.argv else 'male'
cohort=sys.argv[sys.argv.index('--cohort')+1] if '--cohort' in sys.argv else ('female-v2' if sex=='female' else 'v5')
assert '/' not in cohort and '\\' not in cohort and cohort not in ['.','..']
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-projection'/cohort;OUT=ROOT/'candidate-binary';OUT.mkdir(exist_ok=True);dirs=['front','right','back','left'];clips={};changes=[];allpreviews=[]
reports=[json.loads(p.read_text()) for p in sorted((ROOT/'reports').glob(sex+'-*-projection.json'))]
for report in reports:
 gait=report['gait'];count=len(report.get('source_frames',range(4 if 'idle' in gait else 8)));size=report['camera']['frame'][0];anchor=report['camera']['anchor']
 contact=Image.new('RGBA',(size*count,4*(size+16)),(37,43,40,255));draw=ImageDraw.Draw(contact);cycle=[]
 for j,d in enumerate(dirs):
  key=f'{sex}-{gait}-{d}';clip={'frame_size':[size,size],'anchor':anchor,'direction':d,'action':gait,'equipment':report.get('equipment','branch-club' if gait.startswith('club-') else 'unarmed'),'loop':any(t in gait for t in ['idle','walk','sprint']),'frames':[],'cohort':cohort,'source_cohort':report.get('copied_from_cohort',report.get('cohort',cohort)),'source_blend_sha256':report.get('target_sha256'),'attack_timing':report.get('attack_timing')};clips[key]=clip
  clip['raster_pass']=report.get('raster_pass');clip['attack_source_manifest']=report.get('attack_source_manifest')
  draw.text((2,j*(size+16)),key,fill='white')
  for k in range(count):
   src=ROOT/'frames'/f'{key}-{k:02d}.png';im=Image.open(src).convert('RGBA');a=np.array(im);assert im.size==(size,size);nonbinary=int(((a[:,:,3]>0)&(a[:,:,3]<255)).sum());a[:,:,3]=np.where(a[:,:,3]>=128,255,0);a[a[:,:,3]==0,:3]=0;final=Image.fromarray(a);box=final.getbbox();assert box and 0<box[0]<box[2]<size and 0<box[1]<box[3]<size
   dst=OUT/src.name;final.save(dst);clip['geometry_sha256']=report['geometry_after'];clip['weapon_appearance']=report.get('weapon_appearance');clip['intentional_braid_geometry_change']={k:v for k,v in report.get('intentional_braid_geometry_change',{}).items() if k not in ['before','after']};clip['appearance_atlas_sha256']=report['source_sha256'];clip['frames'].append({'src':dst.name,'sha256':hashlib.sha256(dst.read_bytes()).hexdigest(),'source_sha256':hashlib.sha256(src.read_bytes()).hexdigest(),'bbox':box});changes.append({'file':dst.name,'nonbinary_source_pixels':nonbinary});contact.alpha_composite(final,(k*size,j*(size+16)+16))
 contact.save(OUT/f'{sex}-{gait}-contact-native.png');contact.resize((contact.width*2,contact.height*2),Image.Resampling.NEAREST).save(OUT/f'{sex}-{gait}-contact-2x.png')
 for k in range(count):
  frame=Image.new('RGBA',(size*2,size*2),(37,43,40,255))
  for j,d in enumerate(dirs):frame.alpha_composite(Image.open(OUT/f'{sex}-{gait}-{d}-{k:02d}.png'),((j%2)*size,(j//2)*size))
  cycle.append(frame.convert('RGB').resize((frame.width*3,frame.height*3),Image.Resampling.NEAREST))
 cycle[0].save(OUT/f'{sex}-{gait}-four-directions.gif',save_all=True,append_images=cycle[1:],duration=150 if 'idle' in gait else 100 if 'walk' in gait else 50 if 'attack' in gait else 70 if 'sprint' in gait else 100,loop=0,disposal=2)
manifest={'status':'candidate-pending-independent-native-review','technique':'ImageGen-painted surface appearance projected onto animated Blender meshes; female braid width has an explicit geometry revision. These are rerenders, not independently generated animation sheets.','base_cell':48,'pixels_per_metre':48,'alpha_policy':'threshold128; zeroRGB under alpha0; no resizing, no recentering','clips':clips,'source_directory':'../frames','changed_alpha_counts':changes};(OUT/'manifest.json').write_text(json.dumps(manifest,indent=2));print('pack frames',sum(len(c['frames']) for c in clips.values()))
