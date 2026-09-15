"""CC0 combat clips on existing anatomical dressed rigs, fixed 48 px/m.

Run Blender background with -- sex clip [preview]. Source files stay untouched.
Actions use 128px canvases; idle/hit use 96px at identical world pixel scale.
The rest skeleton already includes grip flexion: never apply arbitrary Euler
curls to those fingers. Baked clothing and belt are transported together.
"""
import bpy,addon_utils,math,json,sys,hashlib
from pathlib import Path
from mathutils import Matrix,Vector
from mathutils.kdtree import KDTree
from bpy_extras.object_utils import world_to_camera_view
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice';OUT=ROOT/'starter-combat'
args=sys.argv[sys.argv.index('--')+1:];sex,clip=args[:2];preview='preview' in args
for d in ['blender','references','inspection','provenance']:(OUT/d).mkdir(exist_ok=True,parents=True)
source=ROOT/('starter-v2' if sex=='male' else 'starter-v2-female-refine')/'blender'/f'{sex}-walk-exterior.blend'
bpy.ops.wm.open_mainfile(filepath=str(source));s=bpy.context.scene;rig=bpy.data.objects[f'MH_{sex}_Rig'];s.frame_set(72)
rest={b.name:b.matrix_local.copy() for b in rig.data.bones};oldskin={b.name:b.matrix@rest[b.name].inverted() for b in rig.pose.bones}
cloth=bpy.data.objects[f'{sex}_walk_baked_cloth_samples'];deps=bpy.context.evaluated_depsgraph_get()
groups={g.index:g.name for g in cloth.vertex_groups};weights=[]
for v in cloth.data.vertices:
 w=[(groups[g.group],g.weight) for g in v.groups if groups[g.group] in rest and g.weight>0];total=sum(t for _,t in w);weights.append([(n,t/total) for n,t in w])
clothpoints=[cloth.matrix_world@v.co for v in cloth.evaluated_get(deps).data.vertices]
tree=KDTree(len(clothpoints))
for i,v in enumerate(clothpoints):tree.insert(v,i)
tree.balance();transport=[]
for o in s.objects:
 if o.type!='MESH' or not o.data.shape_keys or o.hide_render or not (o==cloth or 'cloth_bound' in o.name or 'ropebelt_turn' in o.name):continue
 points=[o.matrix_world@v.co for v in o.evaluated_get(deps).data.vertices];bound=[]
 for i,co in enumerate(points):
  w=weights[i] if o==cloth else weights[tree.find(co)[1]]
  m=Matrix(((0,0,0,0),)*4)
  for n,t in w:m+=oldskin[n]*t
  bound.append((m.inverted()@rig.matrix_world.inverted()@co,w))
 o.shape_key_clear();o.animation_data_clear();o.shape_key_add(name='Basis');transport.append((o,bound))
rig.animation_data_clear()
def aim(name,direction):
 b=rig.pose.bones[name];h=b.head.copy();q=(b.tail-b.head).rotation_difference(direction);b.matrix=Matrix.Translation(h)@q.to_matrix().to_4x4()@Matrix.Translation(-h)@b.matrix;bpy.context.view_layer.update()

# Rest-space branch through the existing closed hand. The four knuckles define
# the grip axis, avoiding orientation guesses in pose space.
kn=[rig.data.bones[f'{f}_01_r'].head_local.copy() for f in ['index','middle','ring','pinky']]
axis=(kn[0]-kn[-1]).normalized();center=sum(kn,Vector())/4
# Inside the existing curved fingers: center of proximal/distal joint cloud.
joint=[rig.data.bones[f'{f}_{i:02d}_r'].head_local.copy() for f in ['index','middle','ring','pinky'] for i in [1,2,3]]
center=sum(joint,Vector())/len(joint)
u=axis.cross(Vector((1,0,0))).normalized();v=axis.cross(u)
verts=[];faces=[]
for j,(t,radius) in enumerate([(-.075,.015),(-.04,.014),(0,.014),(.08,.016),(.23,.025),(.37,.030),(.40,.025)]):
 for i in range(9):
  a=2*math.pi*i/9;co=center+axis*t+u*(radius*math.cos(a)*(1+.12*math.sin(i*3+j)))+v*radius*math.sin(a);verts.append(co)
 if j:
  for i in range(9):faces.append(((j-1)*9+i,(j-1)*9+(i+1)%9,j*9+(i+1)%9,j*9+i))
faces.extend([tuple(range(8,-1,-1)),tuple(range(54,63))])
mesh=bpy.data.meshes.new('branch_club');mesh.from_pydata(verts,[],faces);club=bpy.data.objects.new(f'{sex}_starter_branch_club',mesh);s.collection.objects.link(club)
mat=bpy.data.materials.new('combat_branch_bark');mat.diffuse_color=(.12,.067,.026,1);mat.use_nodes=True;mat.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value=mat.diffuse_color;mat.node_tree.nodes['Principled BSDF'].inputs['Roughness'].default_value=.94;mesh.materials.append(mat)
club.matrix_world=rig.matrix_world;vg=club.vertex_groups.new(name='hand_r');vg.add(list(range(len(verts))),1,'REPLACE');mod=club.modifiers.new('Hand grip','ARMATURE');mod.object=rig
club.hide_render=clip in ['idle','unarmed-attack']
frames=[0]*4;src=None;rotation=None;scale=1
if clip in ['attack','hit','death','unarmed-attack']:
 action={'attack':'Sword_Attack','hit':'Hit_Chest','death':'Death01','unarmed-attack':'Punch_Cross'}[clip]
 with bpy.data.libraries.load(str(OUT/'mocap/selected-animation-library.blend'),link=False) as (a,b):b.objects=['Rig'];b.actions=[action]
 src=b.objects[0];s.collection.objects.link(src);src.hide_render=True;src.animation_data_create();src.animation_data.action=b.actions[0]
 frames={'attack':[0,5,9,12,15,19,25,36],'hit':[0,1,2,3,4,5,7,8],'death':[0,7,14,21,28,37,47,57],'unarmed-attack':[0,3,6,9,12,15,19,24]}[clip]
 rotation=Matrix.Identity(3)
 scale=(rig.data.bones['thigh_l'].length+rig.data.bones['calf_l'].length)/(src.data.bones['DEF-thigh.L'].length+src.data.bones['DEF-shin.L'].length)
poses=[]
for phase,sourceframe in enumerate(frames):
 if src:s.frame_set(sourceframe);joints={b.name:rotation@b.head for b in src.pose.bones}
 for b in rig.pose.bones:b.matrix_basis=Matrix.Identity(4)
 bpy.context.view_layer.update()
 if src:
  # Preserve anatomical spine lengths/rest curvature, transfer measured tilt.
  tilt=Vector((0,0,1)).rotation_difference((joints['DEF-neck']-joints['DEF-hips']).normalized())
  for name in ['spine_01','spine_02','spine_03']:
   b=rig.data.bones[name];aim(name,tilt@(b.tail_local-b.head_local))
  aim('neck_01',joints['DEF-head']-joints['DEF-neck']);aim('head',rotation@(src.pose.bones['DEF-head'].tail-src.pose.bones['DEF-head'].head))
  for side,suffix in [('l','L'),('r','R')]:
   for name,a,b in [(f'thigh_{side}','thigh','shin'),(f'calf_{side}','shin','foot'),(f'foot_{side}','foot','toe'),(f'upperarm_{side}','upper_arm','forearm'),(f'lowerarm_{side}','forearm','hand')]:aim(name,joints[f'DEF-{b}.{suffix}']-joints[f'DEF-{a}.{suffix}'])
   # Anatomical palm frames transfer measured wrist orientation, not Euler
   # guesses. Existing curled target fingers retain their actual shape.
   def palm_basis(wrist,middle,index,pinky):
    x=(index-pinky).normalized();y=middle-wrist;y=(y-x*y.dot(x)).normalized();z=x.cross(y);return Matrix((x,y,z)).transposed()
   target_basis=palm_basis(rig.data.bones[f'hand_{side}'].head_local,rig.data.bones[f'middle_01_{side}'].head_local,rig.data.bones[f'index_01_{side}'].head_local,rig.data.bones[f'pinky_01_{side}'].head_local)
   source_basis=palm_basis(joints[f'DEF-hand.{suffix}'],joints[f'DEF-f_middle.01.{suffix}'],joints[f'DEF-f_index.01.{suffix}'],joints[f'DEF-f_pinky.01.{suffix}'])
   hand=rig.pose.bones[f'hand_{side}'];rot=source_basis@target_basis.inverted()@rest[hand.name].to_3x3();hand.matrix=Matrix.Translation(hand.head)@rot.to_4x4();bpy.context.view_layer.update()
 else:
  # Native anatomical rest pose, upright with arms at the sides. Modest breath
  # expands upper chest by a fraction of a degree without moving root/feet.
  rig.pose.bones['spine_03'].rotation_mode='XYZ';rig.pose.bones['spine_03'].rotation_euler.x=math.radians(.35*math.sin(phase*math.pi/2))
 bpy.context.view_layer.update()
 # Retarget lengths differ from actor proportions. Correct the lowest sole to
 # the shared floor without horizontal root drift or camera reframing.
 deps=bpy.context.evaluated_depsgraph_get()
 sole_z=min((o.matrix_world@v.co).z for o in s.objects if o.name.startswith(f'{sex}_sandal_sole_') or (clip=='death' and o.name==f'MH_{sex}_Body') for v in o.evaluated_get(deps).data.vertices)
 rig.pose.bones['Root'].location.z+=.008-sole_z
 bpy.context.view_layer.update()
 poses.append({b.name:b.matrix_basis.copy() for b in rig.pose.bones})

for phase,pose in enumerate(poses):
 frame=1+phase*4;s.frame_set(frame)
 for b in rig.pose.bones:b.matrix_basis=pose[b.name]
 bpy.context.view_layer.update()
 skin={b.name:b.matrix@rest[b.name].inverted() for b in rig.pose.bones}
 for o,bound in transport:
  key=o.shape_key_add(name=f'{clip}_{phase:02d}');to=o.matrix_world.inverted()@rig.matrix_world
  for point,(co,w) in zip(key.data,bound):
   m=Matrix(((0,0,0,0),)*4)
   for n,t in w:m+=skin[n]*t
   point.co=to@m@co
  for k in range(len(poses)):key.value=int(k==phase);key.keyframe_insert('value',frame=1+k*4)
 for b in rig.pose.bones:
  b.keyframe_insert('location',frame=frame);b.keyframe_insert('rotation_quaternion' if b.rotation_mode=='QUATERNION' else 'rotation_euler',frame=frame)
s.frame_start=1;s.frame_end=1+(len(poses)-1)*4
s.render.engine='CYCLES';s.cycles.samples=24;s.cycles.use_denoising=False;s.render.film_transparent=True;s.render.image_settings.color_mode='RGBA'
size=128 if clip in ['attack','death','unarmed-attack'] else 96;anchor_px=[64,96] if size==128 else [48,80]
# Larger action margins preserve physical pixel scale; do not fit/rescale poses.
if size==128:
 s.render.resolution_x=s.render.resolution_y=size;s.camera.data.lens*=96/size;s.camera.data.shift_x=0;s.camera.data.shift_y=0;bpy.context.view_layer.update()
 origin=world_to_camera_view(s,s.camera,Vector((0,0,0)));s.camera.data.shift_y=origin.y-(1-anchor_px[1]/size)
transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']];records=[]
for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
 rot=Matrix.Rotation(math.radians(-angle),4,'Z')
 for o,m in transforms:o.matrix_world=rot@m
 indices=[0,3,7] if preview and clip in ['attack','death','hit'] else [0] if preview else range(len(poses))
 for i in indices:
  s.frame_set(1+i*4);bpy.context.view_layer.update();s.render.resolution_x=s.render.resolution_y=size;s.render.resolution_percentage=100
  print('POSE_CHECK',direction,i,list(rig.pose.bones['hand_r'].head),flush=True)
  anchor=world_to_camera_view(s,s.camera,Vector((0,0,0)));assert abs(anchor.x*size-anchor_px[0])<.01 and abs((1-anchor.y)*size-anchor_px[1])<.01
  p=OUT/'references'/f'{sex}-{clip}-{direction}-{i:02d}.png';s.render.filepath=str(p);bpy.ops.render.render(write_still=True)
  records.append({'src':str(p.relative_to(OUT)).replace('\\','/'),'anchor':anchor_px,'source_frame':frames[i]})
  if i in [0,3]:
   s.render.resolution_x=s.render.resolution_y=384;s.render.filepath=str(OUT/'inspection'/f'{sex}-{clip}-{direction}-{i:02d}.png');bpy.ops.render.render(write_still=True)
for o,m in transforms:o.matrix_world=m
s.frame_set(1);s.render.resolution_x=s.render.resolution_y=size
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'blender'/f'{sex}-{clip}.blend'),compress=True)
(OUT/'provenance'/f'{sex}-{clip}.json').write_text(json.dumps({'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'motion':'Quaternius Universal Animation Library / '+action if src else 'Anatomical rest with subtle breathing','source_frames':frames,'frame_size':[size,size],'anchor':anchor_px,'pixels_per_metre':48,'equipment':'branch-club' if not club.hide_render else 'unarmed','frames':records,'status':'candidate-needs-visual-review'},indent=2))
print('COMBAT_REFERENCE_COMPLETE',sex,clip)
