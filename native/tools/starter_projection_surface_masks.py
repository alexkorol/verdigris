"""Render semantic surface masks for color-continuity measurements, no geometry edits."""
import bpy,sys,math
from pathlib import Path
from mathutils import Matrix
args=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else sys.argv[1:]
sex=args[0] if args else 'male';cohort=args[1] if len(args)>1 else 'male-v7'
assert sex in ['male','female']
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-projection'/cohort
(ROOT/'masks').mkdir(exist_ok=True)
for gait in ['walk','sprint']:
 bpy.ops.wm.open_mainfile(filepath=str(ROOT/f'blender/{sex}-{gait}-projection.blend'));s=bpy.context.scene
 mats={}
 for name,color in [('skin',(0,1,0,1)),('cloth',(1,0,0,1)),('hair',(0,0,1,1)),('other',(1,1,1,1))]:
  m=bpy.data.materials.new('MASK_'+name);m.use_nodes=True;n=m.node_tree.nodes;n.clear();out=n.new('ShaderNodeOutputMaterial');e=n.new('ShaderNodeEmission');e.inputs['Color'].default_value=color;m.node_tree.links.new(e.outputs[0],out.inputs[0]);mats[name]=m
 for o in s.objects:
  if o.type not in ['MESH','CURVE'] or o.hide_render or not o.visible_get():continue
  kind='cloth' if 'baked_cloth_samples' in o.name else 'skin' if o.name==f'MH_{sex}_Body' else 'hair' if 'hair' in o.name or 'beard' in o.name else 'other'
  for i in range(len(o.data.materials)):o.data.materials[i]=mats[kind]
 s.cycles.samples=8;trans=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']]
 for d,a in [('front',0),('right',90),('back',180),('left',270)]:
  for o,m in trans:o.matrix_world=Matrix.Rotation(math.radians(-a),4,'Z')@m
  for k in [0,4]:s.frame_set(72+k*4);s.render.filepath=str(ROOT/f'masks/{sex}-{gait}-{d}-{k:02d}.png');bpy.ops.render.render(write_still=True)
