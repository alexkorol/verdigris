import bpy,math,json
from pathlib import Path
from mathutils import Matrix
ROOT=Path.cwd()/'native/client/assets/first-slice/starter-projection';out=ROOT/'scratch/coverage-probe';out.mkdir(exist_ok=True)
for sex in ['male','female']:
 bpy.ops.wm.open_mainfile(filepath=str(ROOT/'accepted'/sex/'scenes'/f'{sex}-idle-projection.blend'));s=bpy.context.scene
 report={}
 for mat in bpy.data.materials:
  if not mat.use_nodes:continue
  nodes=mat.node_tree.nodes;links=mat.node_tree.links
  output=next((n for n in nodes if n.type=='OUTPUT_MATERIAL'),None)
  if not output or not output.inputs['Surface'].links:continue
  final=output.inputs['Surface'].links[0].from_node
  em=nodes.new('ShaderNodeEmission')
  if final.type=='MIX_SHADER' and final.inputs[0].links:
   valid=final.inputs[0].links[0].from_socket
   mix=nodes.new('ShaderNodeMixRGB');mix.inputs[1].default_value=(1,0,0,1);mix.inputs[2].default_value=(0,1,0,1)
   links.new(valid,mix.inputs[0]);links.new(mix.outputs[0],em.inputs[0]);report[mat.name]='red fallback; green projected'
  else:em.inputs[0].default_value=(0,0,1,1)
  links.new(em.outputs[0],output.inputs['Surface'])
 cam=s.camera.matrix_world.copy()
 for d,a in [('front',0),('right',90)]:
  s.camera.matrix_world=Matrix.Rotation(math.radians(-a),4,'Z')@cam;s.frame_set(1);s.render.filepath=str(out/f'{sex}-{d}.png');bpy.ops.render.render(write_still=True)
 (out/f'{sex}-materials.json').write_text(json.dumps(report,indent=2))
print('COVERAGE_PROBE_COMPLETE')
