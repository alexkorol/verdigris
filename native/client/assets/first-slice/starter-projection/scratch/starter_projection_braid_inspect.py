import bpy
from pathlib import Path
r=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-projection/female-v1/blender/female-walk-projection.blend';bpy.ops.wm.open_mainfile(filepath=str(r));s=bpy.context.scene
for o in s.objects:
 if 'braid' in o.name and o.visible_get():print('BRAID',o.name,o.data.name,[(sp.material_index,len(sp.points)) for sp in o.data.splines],[(m.name,[n.inputs['Color'].default_value[:] for n in m.node_tree.nodes if n.type=='EMISSION']) for m in o.data.materials])
