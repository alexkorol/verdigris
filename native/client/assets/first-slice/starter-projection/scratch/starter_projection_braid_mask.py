import bpy
from pathlib import Path
r=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-projection'
for stage in ['female-v1','female-v2']:
 bpy.ops.wm.open_mainfile(filepath=str(r/stage/'blender/female-walk-projection.blend'));s=bpy.context.scene;s.frame_set(72)
 for o in s.objects:
  if o.type not in ['MESH','CURVE'] or o.hide_render or not o.visible_get():continue
  white=o.type=='CURVE' and 'natural_long_braid_' in o.name;m=bpy.data.materials.new('BraidMask');m.use_nodes=True;n=m.node_tree.nodes;n.clear();output=n.new('ShaderNodeOutputMaterial');e=n.new('ShaderNodeEmission');e.inputs['Color'].default_value=(1,1,1,1) if white else (0,0,0,1);m.node_tree.links.new(e.outputs[0],output.inputs[0]);o.data.materials.clear();o.data.materials.append(m)
 s.cycles.samples=32;s.render.filepath=str(r/stage/'braid-mask.png');bpy.ops.render.render(write_still=True)
