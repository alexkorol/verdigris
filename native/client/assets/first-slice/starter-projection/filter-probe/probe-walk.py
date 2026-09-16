import bpy,json
from pathlib import Path
from mathutils import Matrix
root=Path('Z:/Code/.worktrees/verdigris-first-slice-art/native/client/assets/first-slice/starter-projection');out=root/'filter-probe'
bpy.ops.wm.open_mainfile(filepath=str(root/'female-v2/blender/female-walk-projection.blend'));s=bpy.context.scene;s.frame_set(72)
print('FILTER_PROPERTIES',[(p.identifier,str(getattr(s.cycles,p.identifier))) for p in s.cycles.bl_rna.properties if 'filter' in p.identifier])
cams=json.loads((root/'female-v2/reports/female-walk-projection.json').read_text())['camera_matrices'];settings=[]
for name,kind,width in [('existing','BLACKMAN_HARRIS',1.5),('box','BOX',1.0)]:
 s.cycles.pixel_filter_type=kind;s.cycles.filter_width=width;s.camera.matrix_world=Matrix(cams[0])
 for i,frame in enumerate(range(72,101,4)):
  s.frame_set(frame);s.render.filepath=str(out/f'{name}-walk-{i:02d}.png');bpy.ops.render.render(write_still=True)
