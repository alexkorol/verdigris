"""Revise approved locomotion milestones to the owner's September starter exterior.

Keeps the recorded motion, camera and simulated cloth. All hem edits interpolate
on the existing simulated surface; rope duplicates retain the cloth shape keys.
"""
import bpy,math,json,hashlib,sys
from pathlib import Path
from mathutils import Vector,Matrix
from bpy_extras.object_utils import world_to_camera_view
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice'
OUT=ROOT/'starter-v2'
if '--female-refine' in sys.argv: OUT=ROOT/'starter-v2-female-refine'
for d in ['blender','references','inspection','guides']:(OUT/d).mkdir(parents=True,exist_ok=True)
args=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else ['male','walk']
sex,gait=args[:2]
src=ROOT/'reviewed/blender'/ (f'male-{gait}-linen.blend' if sex=='male' else f'female-{gait}-center-braid.blend')
dst=OUT/'blender'/f'{sex}-{gait}-exterior.blend'
if dst.exists():raise RuntimeError('Existing milestone: '+str(dst))
bpy.ops.wm.open_mainfile(filepath=str(src));s=bpy.context.scene;rig=bpy.data.objects[f'MH_{sex}_Rig'];s.frame_set(72)
# Save the exact starting checkpoint before any edits.
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'blender'/f'{sex}-{gait}-before-exterior.blend'),compress=True)
linen=bpy.data.materials[f'player-{sex}_linen_single_shell_material'];p=linen.node_tree.nodes['Principled BSDF'];p.inputs['Base Color'].default_value=(.36,.30,.23,1);linen.diffuse_color=(.36,.30,.23,1)
p.inputs['Roughness'].default_value=.97
rope=bpy.data.materials[f'player-{sex}_russet_binding'];rope.diffuse_color=(.23,.17,.10,1);rope.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value=rope.diffuse_color
# Three substantial cord turns, transported by precisely the same baked keys.
cord=bpy.data.objects[f'player-{sex}_waist_cord_cloth_bound']
for k,z in enumerate([-.014,.014]):
 o=cord.copy();o.data=cord.data.copy();o.name=f'{sex}_ropebelt_turn_{k}';s.collection.objects.link(o)
 for key in o.data.shape_keys.key_blocks:
  for v in key.data:v.co.z+=z
 o['appearance']='Three-turn natural rope belt matching approved starter concept'
# Increase the two knot ends' thickness without changing their cloth transport.
ends=bpy.data.objects[f'player-{sex}_belt_ends_cloth_bound']
for key in ends.data.shape_keys.key_blocks:
 for v in key.data:v.co.x*=1.2
cloth=bpy.data.objects[f'{sex}_{gait}_baked_cloth_samples']
if sex=='female':
 # Raise one hip's corner into an asymmetric athletic opening; retain all baked folds.
 for key in cloth.data.shape_keys.key_blocks:
  original=[v.co.copy() for v in key.data]
  for side in range(2):
   for row in range(10):
    for col in range(33):
     lift=2.5*(col/32)**3*(1-row/10)**2
     at=row+lift;low=math.floor(at);t=at-low;i=side*37*33+row*33+col;a=side*37*33+low*33+col
     key.data[i].co=original[a].lerp(original[a+33],t)
 # Reveal the skin newly exposed by the raised athletic hem; preserve the helper mask.
 body=bpy.data.objects[f'MH_{sex}_Body'];visible=body.vertex_groups[f'player-{sex}_visible_skin']
 visible.add([v.index for v in body.data.vertices if v.co.z<.80],1,'REPLACE')
 # A single long braid starts at the nape and drapes around the shoulder.
 # Three small interwoven strands form one braid, not three tails.
 controls=[Vector((.01,.047,1.61)),Vector((-.072,.045,1.54)),Vector((-.13,-.035,1.465)),Vector((-.13,-.145,1.36)),Vector((-.13,-.18,1.245))]
 for strand,o in enumerate([o for o in s.objects if o.name.startswith('female_centered_back_braid_')]):
  for spline in o.data.splines:
   count=len(spline.points)-1
   for i,p in enumerate(spline.points):
    t=i/count;seg=min(3,int(t*4));u=t*4-seg;center=controls[seg].lerp(controls[seg+1],u);angle=t*math.pi*15+strand*math.pi*2/3;radius=.013*(1-.65*t)
    p.co=(center.x+radius*math.cos(angle),center.y+radius*math.sin(angle),center.z,1)
  o.name=o.name.replace('centered_back','natural_long')
# Foot coverings copy the sole's attachment transform. Narrow woven strips arch
# over the forefoot, preserving the toe profile instead of creating blocky shoes.
for side in [-1,1]:
 sole=bpy.data.objects[f'{sex}_sandal_sole_{side}'];vs=[v.co for v in sole.data.vertices]
 xmin,xmax=min(v.x for v in vs),max(v.x for v in vs);ymin,ymax=min(v.y for v in vs),max(v.y for v in vs);zmax=max(v.z for v in vs)
 for strip in range(7):
  y=ymin+.018+strip*(ymax-ymin-.07)/6
  curve=bpy.data.curves.new('woven_upper','CURVE');curve.dimensions='3D';curve.bevel_depth=.0045;curve.bevel_resolution=1
  spline=curve.splines.new('POLY');spline.points.add(12)
  for i,p in enumerate(spline.points):
   t=i/12;p.co=(xmin+(xmax-xmin)*t,y,zmax+.010+.049*math.sin(math.pi*t),1)
  o=bpy.data.objects.new(f'{sex}_woven_upper_{side}_{strip}',curve);s.collection.objects.link(o);curve.materials.append(sole.data.materials[0])
  o.parent=sole.parent;o.parent_type=sole.parent_type;o.parent_bone=sole.parent_bone;o.matrix_parent_inverse=sole.matrix_parent_inverse.copy();o.matrix_basis=sole.matrix_basis.copy()
s['appearance_reference_sha256']=hashlib.sha256((ROOT/'concept/starter-player-exterior.png').read_bytes()).hexdigest()
s['starter_wardrobe']='Untrimmed coarse flax, three-turn rope belt, woven forefoot, natural long female braid, asymmetric athletic female hem'
s.render.engine='CYCLES';s.cycles.samples=32;s.cycles.use_denoising=False;s.render.film_transparent=True;s.render.image_settings.color_mode='RGBA'
c=s.camera;transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']]
records={}
for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
 rot=Matrix.Rotation(math.radians(-angle),4,'Z')
 for o,m in transforms:o.matrix_world=rot@m
 for i,frame in enumerate(range(72,101,4)):
  s.frame_set(frame);s.render.resolution_x=s.render.resolution_y=96;s.render.resolution_percentage=100
  path=OUT/'references'/f'{sex}-{gait}-{direction}-{i:02d}.png';s.render.filepath=str(path);bpy.ops.render.render(write_still=True)
  records.setdefault(f'{sex}-{gait}-{direction}',{'anchor':[48,80],'frames':[]})['frames'].append({'src':str(path.relative_to(OUT)).replace('\\','/'),'sha256':hashlib.sha256(path.read_bytes()).hexdigest()})
  if i==0:
   s.render.resolution_x=s.render.resolution_y=384;s.render.filepath=str(OUT/'inspection'/f'{sex}-{gait}-{direction}.png');bpy.ops.render.render(write_still=True)
for o,m in transforms:o.matrix_world=m
s.frame_set(72);s.render.resolution_x=s.render.resolution_y=96
bpy.ops.wm.save_as_mainfile(filepath=str(dst),compress=True)
(OUT/f'{sex}-{gait}-references.json').write_text(json.dumps({'clips':records,'source':str(src),'source_sha256':hashlib.sha256(src.read_bytes()).hexdigest()},indent=2))
print('EXTERIOR_COMPLETE',sex,gait)
