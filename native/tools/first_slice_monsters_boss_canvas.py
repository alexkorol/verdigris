"""Expand boss canvas without rescaling geometry or changing48px/m projection."""
import bpy,json,hashlib
from pathlib import Path
from mathutils import Matrix,Vector
from bpy_extras.object_utils import world_to_camera_view
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice';OUT=ROOT/'monsters-v2'
m=json.loads((OUT/'reference-manifest.json').read_text())
# Death v2 already authors its final canvas and must not be overwritten here.
for action in ('idle','walk','attack','hit'):
    combat=action in ('attack','hit')
    source=OUT/'blender'/f'well-alpha-{action}.blend' if combat else ROOT/'blender-wolves/well-alpha-walking.blend'
    bpy.ops.wm.open_mainfile(filepath=str(source));s=bpy.context.scene
    s.render.resolution_x=128;s.render.resolution_y=96
    s.render.threads_mode='FIXED';s.render.threads=4
    if not combat:
        r=bpy.data.objects['WolfArmature'];a=bpy.data.actions['Idle' if action=='idle' else 'Walking'];r.animation_data.action=a
        if a.slots:r.animation_data.action_slot=a.slots[0]
    transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ('CAMERA','LIGHT')]
    for d,direction in enumerate(('front','right','back','left')):
        for o,t in transforms:o.matrix_world=Matrix.Rotation(-d*3.14159265359/2,4,'Z')@t
        record=m['clips'][f'well-alpha-{action}-{direction}'];record['frame']=[128,96];record['anchor']=[64,64]
        record['blender_source']=f'blender/well-alpha-{action}-canvas128.blend'
        for entry in record['frames']:
            s.frame_set(entry['blender_frame']);p=OUT/entry['src'];s.render.filepath=str(p);bpy.ops.render.render(write_still=True)
            entry['sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    for o,t in transforms:o.matrix_world=t
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'blender'/f'well-alpha-{action}-canvas128.blend'),compress=True)
(OUT/'reference-manifest.json').write_text(json.dumps(m,indent=2))
