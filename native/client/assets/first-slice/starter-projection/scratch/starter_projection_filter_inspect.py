import bpy
from pathlib import Path
r=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-projection';bpy.ops.wm.open_mainfile(filepath=str(r/'female-v2/blender/female-walk-projection.blend'));s=bpy.context.scene
for group in [s.cycles,s.render]:
 for p in group.bl_rna.properties:
  if 'filter' in p.identifier:print(type(group).__name__,p.identifier,getattr(group,p.identifier))
