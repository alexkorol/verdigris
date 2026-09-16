import bpy,json
from pathlib import Path
r=Path(__file__).resolve().parents[1]/'client/assets/first-slice'
bpy.ops.wm.open_mainfile(filepath=str(r/'starter-combat/blender/male-idle.blend'))
s=bpy.context.scene;s.frame_set(1)
print('CAM',s.camera.data.type,s.camera.data.lens,s.camera.data.shift_y,list(s.camera.matrix_world),s.view_settings.view_transform)
for o in s.objects:
 if o.type in ['MESH','CURVE'] and not o.hide_render:print('OBJ',o.name,len(o.data.vertices) if o.type=='MESH' else 'curve',[(m.name,m.type,m.show_render) for m in o.modifiers])
