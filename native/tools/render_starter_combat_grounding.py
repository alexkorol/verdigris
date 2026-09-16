"""Correct active female corpse floor penetration with bound cloth in 3D."""
import bpy,json,math,hashlib
from pathlib import Path
from mathutils import Matrix,Vector
OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat';sex='female';source=OUT/'transition-v3-skin/female-death/club-death.blend';dest=OUT/'contact-v4-repaired/female-death'
for d in ['references','inspection']:(dest/d).mkdir(parents=True,exist_ok=True)
bpy.ops.wm.open_mainfile(filepath=str(source));s=bpy.context.scene;rig=bpy.data.objects['MH_female_Rig'];changes=[]
def visible_meshes():
 return [o for o in s.objects if o.type=='MESH' and not o.hide_render and o.name.startswith(('female_','MH_female_','player_female_')) and any(not c.hide_render for c in o.users_collection)]
def low():
 dep=bpy.context.evaluated_depsgraph_get()
 return min((o.matrix_world@v.co).z for o in visible_meshes() for v in o.evaluated_get(dep).data.vertices)
for i in range(8):
 frame=1+4*i;s.frame_set(frame);bpy.context.view_layer.update();before=low();delta=max(0,.008-before)
 if delta>1e-6:
  movement=Vector((0,0,delta));local=rig.matrix_world.inverted().to_3x3()@movement;root=rig.pose.bones['Root'];root.matrix=Matrix.Translation(local)@root.matrix;root.keyframe_insert('location',frame=frame)
  for o in visible_meshes():
   if not o.data.shape_keys:continue
   key=o.data.shape_keys.key_blocks.get(f'death_{i:02d}')
   if key:
    shift=o.matrix_world.inverted().to_3x3()@movement
    for v in key.data:v.co+=shift
  bpy.context.view_layer.update()
 after=low();assert after>=.00798,(i,before,after)
 changes.append({'phase':i,'min_z_before':before,'world_z_translation':delta,'min_z_after':after})
transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']];frames=[]
for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
 for o,m in transforms:o.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@m
 for i in range(8):
  s.frame_set(1+4*i);s.render.resolution_x=s.render.resolution_y=128;p=dest/'references'/f'{direction}-{i:02d}.png';s.render.filepath=str(p);bpy.ops.render.render(write_still=True)
  frames.append({'path':str(p.relative_to(OUT)).replace('\\','/'),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'direction':direction,'phase':i})
  s.render.resolution_x=s.render.resolution_y=384;s.render.filepath=str(dest/'inspection'/f'{direction}-{i:02d}.png');bpy.ops.render.render(write_still=True)
for o,m in transforms:o.matrix_world=m
s.frame_set(1);s.render.resolution_x=s.render.resolution_y=128;bpy.ops.wm.save_as_mainfile(filepath=str(dest/'club-death.blend'),compress=True)
(dest/'grounding.json').write_text(json.dumps({'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'method':'World-Z translation of root and each bound-cloth pose key only where active-character geometry penetrates floor. No camera or horizontal shift.','changes':changes,'frame_size':[128,128],'anchor':[64,96],'pixels_per_metre':48,'frames':frames},indent=2))
repair=json.loads((source.parent/'repair.json').read_text());repair['grounding_correction']='grounding.json';(dest/'repair.json').write_text(json.dumps(repair,indent=2))
manifest=json.loads((source.parent/'manifest.json').read_text())
for c in manifest['clips']:
 for f in c['frames']:
  p=dest/'references'/f"{c['direction']}-{f['phase']:02d}.png";f['path']=str(p.relative_to(OUT)).replace('\\','/');f['sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
manifest['grounding_correction']='grounding.json';(dest/'manifest.json').write_text(json.dumps(manifest,indent=2));print('GROUNDED',changes)
