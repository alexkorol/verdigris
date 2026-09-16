"""Close the empty striking hand using small palm-plane finger corrections."""
import bpy,sys,math,json,hashlib
from pathlib import Path
from mathutils import Matrix,Vector,Quaternion
OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat';args=sys.argv[sys.argv.index('--')+1:];sex=args[0];preview='preview' in args;source=OUT/'contact-v4-repaired'/f'{sex}-unarmed-attack'/'unarmed-attack-transition.blend';dest=OUT/'contact-v5-fist'/f'{sex}-unarmed-attack'
for d in ['references','inspection']:(dest/d).mkdir(parents=True,exist_ok=True)
bpy.ops.wm.open_mainfile(filepath=str(source));s=bpy.context.scene;rig=bpy.data.objects[f'MH_{sex}_Rig'];report=[]
for i in [8]:
 s.frame_set(33);bpy.context.view_layer.update();weight=1
 wrist=rig.pose.bones['hand_r'].head.copy();middle=rig.pose.bones['middle_01_r'].head.copy();axis=(rig.pose.bones['index_01_r'].head-rig.pose.bones['pinky_01_r'].head).normalized();forward=middle-wrist;forward=(forward-axis*forward.dot(axis)).normalized();normal=axis.cross(forward).normalized()
 if (rig.pose.bones['middle_03_r'].tail-wrist).dot(normal)<0:normal=-normal
 changes={}
 for finger in ['index','middle','ring','pinky']:
  names=[f'{finger}_{j:02d}_r' for j in [1,2,3]];tip=rig.pose.bones[names[-1]].tail.copy();distance=(tip-wrist).dot(normal);goal=tip-normal*max(0,distance-.014)*weight;angles={n:0 for n in names}
  for iteration in range(12):
   for n in reversed(names):
    bone=rig.pose.bones[n];head=bone.head.copy();a=rig.pose.bones[names[-1]].tail-head;b=goal-head;a-=axis*a.dot(axis);b-=axis*b.dot(axis)
    if a.length<1e-6 or b.length<1e-6:continue
    change=math.atan2(axis.dot(a.cross(b)),a.dot(b));change=max(-.12,min(.12,change));clamped=max(-.35,min(.35,angles[n]+change));change=clamped-angles[n];angles[n]=clamped
    bone.matrix=Matrix.Translation(head)@Quaternion(axis,change).to_matrix().to_4x4()@Matrix.Translation(-head)@bone.matrix;bpy.context.view_layer.update()
  changes[finger]={'tip_plane_before_m':distance,'tip_plane_after_m':(rig.pose.bones[names[-1]].tail-wrist).dot(normal),'joint_adjustments_degrees':{n:math.degrees(a) for n,a in angles.items()}}

 report.append({'phase':i,'closure_weight':weight,'fingers':changes})
closed={f'{finger}_{j:02d}_r':rig.pose.bones[f'{finger}_{j:02d}_r'].rotation_quaternion.copy() for finger in ['index','middle','ring','pinky'] for j in [1,2,3]}
for i in range(16):
 s.frame_set(1+4*i);weight=min(1,i/4,(15-i)/4)
 for name,q in closed.items():
  b=rig.pose.bones[name];b.rotation_quaternion=b.rotation_quaternion.slerp(q,weight);b.keyframe_insert('rotation_quaternion',frame=1+4*i)
transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']]
for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
 if preview and direction!='right':continue
 for o,m in transforms:o.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@m
 for i in ([8] if preview else range(16)):
  s.frame_set(1+4*i);s.render.resolution_x=s.render.resolution_y=128;s.render.filepath=str(dest/'references'/f'{direction}-{i:02d}.png');bpy.ops.render.render(write_still=True)
  s.render.resolution_x=s.render.resolution_y=1536 if preview else 384;s.render.filepath=str(dest/'inspection'/f'{direction}-{i:02d}.png');bpy.ops.render.render(write_still=True)
for o,m in transforms:o.matrix_world=m
s.frame_set(1);s.render.resolution_x=s.render.resolution_y=128;bpy.ops.wm.save_as_mainfile(filepath=str(dest/'unarmed-attack-transition.blend'),compress=True)
(dest/'fist.json').write_text(json.dumps({'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'method':'Existing curved fingers move gently toward a palm-plane contact target using bounded CCD corrections; each joint changes at most20.05degrees. Thumb remains outside. Closure ramps with preparation/recovery; endpoints unchanged.','changes':report,'contact_index':8,'normalized_contact_phase':.5,'equipment':'unarmed'},indent=2))
if not preview:
 repair=json.loads((source.parent/'repair.json').read_text());repair['fist_correction']='fist.json';(dest/'repair.json').write_text(json.dumps(repair,indent=2))
 manifest=json.loads((source.parent/'manifest.json').read_text())
 for c in manifest['clips']:
  for f in c['frames']:
   p=dest/'references'/f"{c['direction']}-{f['index']:02d}.png";f['path']=str(p.relative_to(OUT)).replace('\\','/');f['sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
 manifest['fist_correction']='fist.json';(dest/'manifest.json').write_text(json.dumps(manifest,indent=2))
