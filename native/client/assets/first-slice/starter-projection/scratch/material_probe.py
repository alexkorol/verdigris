import bpy,json
from pathlib import Path
ROOT=Path.cwd()/'native/client/assets/first-slice/starter-projection';out=ROOT/'scratch/material-probe';out.mkdir(exist_ok=True)
for sex in ['male','female']:
 for mode in ['strongest','front-only']:
  bpy.ops.wm.open_mainfile(filepath=str(ROOT/f'{sex}-strongest-proof/blender/{sex}-walk-projection.blend'));s=bpy.context.scene;s.frame_set(72);s.cycles.pixel_filter_type='BOX';s.cycles.filter_width=1.0
  if mode=='front-only':
   for mat in bpy.data.materials:
    if not mat.use_nodes:continue
    n=mat.node_tree.nodes;l=mat.node_tree.links;tex=None
    for node in n:
     if node.type=='TEX_IMAGE' and node.inputs['Vector'].links and getattr(node.inputs['Vector'].links[0].from_node,'uv_map','')=='Projection0':tex=node;break
    if tex:
     em=n.new('ShaderNodeEmission');l.new(tex.outputs['Color'],em.inputs[0]);output=next(x for x in n if x.type=='OUTPUT_MATERIAL');l.new(em.outputs[0],output.inputs['Surface'])
  s.render.filepath=str(out/f'{sex}-{mode}.png');bpy.ops.render.render(write_still=True)
# Portability check: archived scene with all referenced image paths made invalid;
# the packed bytes must be sufficient to reproduce the same native frame.
bpy.ops.wm.open_mainfile(filepath=str(ROOT/'accepted/male/scenes/male-walk-projection.blend'));s=bpy.context.scene;s.frame_set(72);images=[]
for im in bpy.data.images:
 if im.type=='RENDER_RESULT' or im.source!='FILE':continue
 assert im.packed_file, im.name
 images.append({'name':im.name,'packed_bytes':im.packed_file.size});im.filepath='//nonexistent-portability-check/'+im.name
s.render.filepath=str(out/'portable-male-walk.png');bpy.ops.render.render(write_still=True)
(out/'portability.json').write_text(json.dumps(images,indent=2))
print('MATERIAL_PROBE_COMPLETE')
