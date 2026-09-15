"""Equip reviewed source locomotion/idle without changing its skeleton or cloth."""
import bpy,math,json,sys,hashlib
from mathutils import Matrix,Vector
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice';OUT=ROOT/'starter-combat'
sex,gait=sys.argv[sys.argv.index('--')+1:][:2];unarmed=gait.startswith('unarmed-');action=gait.replace('unarmed-','')
source=OUT/'blender'/f'{sex}-{action}.blend' if action in ['idle','hit','death'] else ROOT/('starter-v2' if sex=='male' else 'starter-v2-female-refine')/'blender'/f'{sex}-{action}-exterior.blend'
bpy.ops.wm.open_mainfile(filepath=str(source));s=bpy.context.scene;rig=bpy.data.objects[f'MH_{sex}_Rig'];clip=gait if unarmed else 'club-'+gait;size=s.render.resolution_x;anchor=[64,96] if size==128 else [48,80]
club=bpy.data.objects.get(f'{sex}_starter_branch_club')
if club:bpy.data.objects.remove(club,do_unlink=True)
# Copy the reviewed skinned club from combat checkpoint, linked only to this
# rig. Its coordinates are anatomical rest coordinates shared by gait files.
with bpy.data.libraries.load(str(OUT/'blender'/f'{sex}-attack.blend'),link=False) as (a,b):b.objects=[f'{sex}_starter_branch_club']
club=b.objects[0];s.collection.objects.link(club)
for m in club.modifiers:
 if m.type=='ARMATURE':m.object=rig
club.matrix_world=rig.matrix_world
club.hide_render=unarmed
frames=list(range(1,14,4)) if action=='idle' else list(range(1,30,4)) if action in ['hit','death'] else list(range(72,101,4))
s.render.engine='CYCLES';s.cycles.samples=24;s.cycles.use_denoising=False;s.render.film_transparent=True;s.render.image_settings.color_mode='RGBA'
transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']];records=[]
for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
 rot=Matrix.Rotation(math.radians(-angle),4,'Z')
 for o,m in transforms:o.matrix_world=rot@m
 for i,f in enumerate(frames):
  s.frame_set(f);bpy.context.view_layer.update();s.render.resolution_x=s.render.resolution_y=size
  p=OUT/'references'/f'{sex}-{clip}-{direction}-{i:02d}.png';s.render.filepath=str(p);bpy.ops.render.render(write_still=True)
  records.append({'src':str(p.relative_to(OUT)).replace('\\','/'),'direction':direction,'phase':i,'source_frame':f,'anchor':anchor})
  if i in [0,3]:
   s.render.resolution_x=s.render.resolution_y=384;s.render.filepath=str(OUT/'inspection'/f'{sex}-{clip}-{direction}-{i:02d}.png');bpy.ops.render.render(write_still=True)
for o,m in transforms:o.matrix_world=m
s.frame_set(frames[0]);s.render.resolution_x=s.render.resolution_y=size
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'blender'/f'{sex}-{clip}.blend'),compress=True)
(OUT/'provenance'/f'{sex}-{clip}.json').write_text(json.dumps({'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'equipment':'unarmed' if unarmed else 'branch-club','hand':None if unarmed else 'right','frame_size':[size,size],'anchor':anchor,'pixels_per_metre':48,'frames':records,'status':'candidate-needs-visual-review'},indent=2))
