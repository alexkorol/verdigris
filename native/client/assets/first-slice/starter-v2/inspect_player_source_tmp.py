import bpy,json
from pathlib import Path
p=Path('Z:/Code/.worktrees/verdigris-first-slice-art/native/client/assets/first-slice/starter-v2/blender/female-walk-exterior.blend')
bpy.ops.wm.open_mainfile(filepath=str(p));o=bpy.data.objects['MH_female_Body']
for m in o.modifiers:
 if m.type=='MASK':
  g=o.vertex_groups[m.vertex_group];print('MASK',m.name,m.vertex_group,m.invert_vertex_group);vs=[v.co for v in o.data.vertices if any(vg.group==g.index and vg.weight>.5 for vg in v.groups)];print('BOUNDS',[(min(v[i] for v in vs),max(v[i] for v in vs)) for i in range(3)],len(vs))
