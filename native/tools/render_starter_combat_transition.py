"""Versioned skeletal entry/recovery around existing club attack; no 2D shifts.

Run Blender background -- male. Original eight attack poses/files stay intact.
"""
import bpy, sys, json, math, hashlib
from pathlib import Path
from mathutils import Matrix, Vector
from mathutils.kdtree import KDTree
from bpy_extras.object_utils import world_to_camera_view

OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat'
args=sys.argv[sys.argv.index('--')+1:];sex=args[0];action=args[1] if len(args)>1 else 'attack';aligned='aligned' in args
DEST=OUT/'contact-v4-source'/f'{sex}-{action}' if aligned else OUT/'transition-v2'/sex
for d in ['references','inspection']:(DEST/d).mkdir(parents=True,exist_ok=True)
idlefile=OUT/'blender'/f'{sex}-{"idle" if action=="unarmed-attack" else "club-idle"}.blend'
attackfile=OUT/'blender'/f'{sex}-{action}.blend'
bpy.ops.wm.open_mainfile(filepath=str(idlefile))
bpy.context.scene.frame_set(1)
idle={b.name:b.matrix_basis.copy() for b in bpy.data.objects[f'MH_{sex}_Rig'].pose.bones}
idlecloth={o.name:[v.co.copy() for v in o.data.shape_keys.key_blocks['idle_00'].data] for o in bpy.context.scene.objects if o.type=='MESH' and o.data.shape_keys and 'idle_00' in o.data.shape_keys.key_blocks}
bpy.ops.wm.open_mainfile(filepath=str(attackfile))
s=bpy.context.scene;rig=bpy.data.objects[f'MH_{sex}_Rig'];rest={b.name:b.matrix_local.copy() for b in rig.data.bones}
attack=[]; landmarks=[]
def points():
 out={}
 for name in ['pelvis','spine_03','head','hand_r','calf_r','calf_l','foot_r','foot_l']:
  b=rig.pose.bones.get(name)
  if b:
   co=rig.matrix_world@b.head;p=world_to_camera_view(s,s.camera,co)
   out[name]={'world':list(co),'pixel':[p.x*128,(1-p.y)*128]}
 return out
for i in range(8):
 s.frame_set(1+i*4);bpy.context.view_layer.update()
 attack.append({b.name:b.matrix_basis.copy() for b in rig.pose.bones});landmarks.append(points())
# Recover the same rest-space cloth binding used by the original builder.
s.frame_set(1);bpy.context.view_layer.update();deps=bpy.context.evaluated_depsgraph_get()
skin={b.name:b.matrix@rest[b.name].inverted() for b in rig.pose.bones}
cloth=bpy.data.objects[f'{sex}_walk_baked_cloth_samples']
groups={g.index:g.name for g in cloth.vertex_groups};weights=[]
for v in cloth.data.vertices:
 w=[(groups[g.group],g.weight) for g in v.groups if groups[g.group] in rest and g.weight>0]
 total=sum(t for _,t in w);weights.append([(n,t/total) for n,t in w])
tree=KDTree(len(cloth.data.vertices))
for i,v in enumerate(cloth.evaluated_get(deps).data.vertices):tree.insert(cloth.matrix_world@v.co,i)
tree.balance();transport=[];originalcloth={}
for o in list(s.objects):
 if o.type!='MESH' or not o.data.shape_keys or o.hide_render or not(o==cloth or 'cloth_bound' in o.name or 'ropebelt_turn' in o.name):continue
 originalcloth[o.name]=[[v.co.copy() for v in o.data.shape_keys.key_blocks[f'{action}_{i:02d}'].data] for i in range(8)]
 bound=[]
 for i,v in enumerate(o.evaluated_get(deps).data.vertices):
  co=o.matrix_world@v.co;w=weights[i] if o==cloth else weights[tree.find(co)[1]]
  m=Matrix(((0,0,0,0),)*4)
  for n,t in w:m+=skin[n]*t
  bound.append((m.inverted()@rig.matrix_world.inverted()@co,w))
 o.shape_key_clear();o.animation_data_clear();o.shape_key_add(name='Basis');transport.append((o,bound))
rig.animation_data_clear()
def blend(a,b,t):
 out={}
 for n in a:
  al,aq,asc=a[n].decompose();bl,bq,bsc=b[n].decompose()
  out[n]=Matrix.LocRotScale(al.lerp(bl,t),aq.slerp(bq,t),asc.lerp(bsc,t))
 return out
samples=[('idle',idle),('entry-1',blend(idle,attack[0],.25)),('entry-2',blend(idle,attack[0],.5)),('entry-3',blend(idle,attack[0],.75))]
samples += [(f'attack-{i}',p) for i,p in enumerate(attack)]
samples += [('recovery-1',blend(attack[-1],idle,.25)),('recovery-2',blend(attack[-1],idle,.5)),('recovery-3',blend(attack[-1],idle,.75)),('idle-end',idle)]
if aligned:
 samples=[('idle',idle),('entry-1',blend(idle,attack[0],.25)),('entry-2',blend(idle,attack[0],.5)),('entry-3',blend(idle,attack[0],.75))]
 if action=='attack':
  # Maximum club forward extension is original sample5/sourceframe19.
  # The near-duplicate source12 sample is omitted during preparation.
  samples += [(f'attack-{i}',attack[i]) for i in [0,1,2,4,5,6,7]]
  samples += [(f'recovery-{i}',blend(attack[-1],idle,i/5)) for i in range(1,5)]+[('idle-end',idle)]
 else:
  # Punch_Cross sample3/sourceframe9 is maximum fist extension. Two actual
  # skeletal travel poses replace the redundant extended-punch hold samples.
  samples += [('attack-0',attack[0]),('attack-1',attack[1]),('punch-travel-1',blend(attack[1],attack[3],1/3)),('punch-travel-2',blend(attack[1],attack[3],2/3)),('attack-3',attack[3]),('attack-5',attack[5]),('attack-6',attack[6]),('attack-7',attack[7])]
  samples += [(f'recovery-{i}',blend(attack[-1],idle,i/4)) for i in range(1,4)]+[('idle-end',idle)]
 assert len(samples)==16 and samples[8][0]==('attack-5' if action=='attack' else 'attack-3')
allpoints=[]
for i,(label,pose) in enumerate(samples):
 s.frame_set(1+i*4)
 for b in rig.pose.bones:
  b.matrix_basis=pose[b.name];b.rotation_mode='QUATERNION'
 bpy.context.view_layer.update()
 # Only new transition poses need sole contact correction. Existing samples
 # are untouched so they continue to match the current source/paint hashes.
 if not label.startswith('attack-'):
  deps=bpy.context.evaluated_depsgraph_get()
  z=min((o.matrix_world@v.co).z for o in s.objects if o.name.startswith(f'{sex}_sandal_sole_') for v in o.evaluated_get(deps).data.vertices)
  rig.pose.bones['Root'].location.z+=.008-z;bpy.context.view_layer.update()
 skin={b.name:b.matrix@rest[b.name].inverted() for b in rig.pose.bones}
 for o,bound in transport:
  key=o.shape_key_add(name=label);to=o.matrix_world.inverted()@rig.matrix_world
  for point,(co,w) in zip(key.data,bound):
   m=Matrix(((0,0,0,0),)*4)
   for n,t in w:m+=skin[n]*t
   point.co=to@m@co
  # Existing cloth samples are copied exactly, avoiding inverse/re-forward
  # skinning roundoff or changed nearest-cloth bindings at approved phases.
  exact=originalcloth[o.name][int(label.split('-')[1])] if label.startswith('attack-') else idlecloth.get(o.name) if label in ['idle','idle-end'] else None
  if exact:
   for point,co in zip(key.data,exact):point.co=co
  for j in range(len(samples)):key.value=int(j==i);key.keyframe_insert('value',frame=1+j*4)
 for b in rig.pose.bones:
  b.keyframe_insert('location',frame=1+i*4);b.keyframe_insert('rotation_quaternion',frame=1+i*4)
 allpoints.append({'phase':i,'label':label,'landmarks':points()})
s.frame_start=1;s.frame_end=1+4*(len(samples)-1);s.render.fps=80;s.render.resolution_x=s.render.resolution_y=128
s.render.resolution_percentage=100;s.cycles.samples=24;s.render.film_transparent=True
transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']]
for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
 for o,m in transforms:o.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@m
 for i,(label,_) in enumerate(samples):
  s.frame_set(1+i*4);s.render.resolution_x=s.render.resolution_y=128
  s.render.filepath=str(DEST/'references'/f'{direction}-{i:02d}-{label}.png');bpy.ops.render.render(write_still=True)
  if i in [0,2,4,11,13,15]:
   s.render.resolution_x=s.render.resolution_y=384;s.render.filepath=str(DEST/'inspection'/f'{direction}-{i:02d}-{label}.png');bpy.ops.render.render(write_still=True)
for o,m in transforms:o.matrix_world=m
s.frame_set(1);s.render.resolution_x=s.render.resolution_y=128
bpy.ops.wm.save_as_mainfile(filepath=str(DEST/('unarmed-attack-transition.blend' if action=='unarmed-attack' else 'club-attack-transition.blend')),compress=True)
(DEST/'diagnosis.json').write_text(json.dumps({'action':action,'equipment':'unarmed' if action=='unarmed-attack' else 'branch-club','source_motion':'Punch_Cross' if action=='unarmed-attack' else 'Sword_Attack','source_sha256':hashlib.sha256(attackfile.read_bytes()).hexdigest(),'idle_sha256':hashlib.sha256(idlefile.read_bytes()).hexdigest(),'frame_size':[128,128],'anchor':[64,96],'pixels_per_metre':48,'method':'Quaternion skeletal blends and source sample remapping; sole height contact. No raster shifts. Horizontal foot planting is not constrained.','normalized_contact_phase':.5 if aligned else None,'contact_index':8 if aligned else None,'contact_source_index':(3 if action=='unarmed-attack' else 5) if aligned else None,'source_attack_landmarks':landmarks,'transition_landmarks':allpoints,'status':'candidate-needs-visual-review'},indent=2))
