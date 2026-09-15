"""Render the source wolf's authored walking animation, never invented frames."""
import bpy
import math
import json
from pathlib import Path
from mathutils import Matrix, Vector
from bpy_extras.object_utils import world_to_camera_view

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'
OUT = ROOT / 'blender-wolves'
OUT.mkdir(parents=True, exist_ok=True)
records = []
for name in ('pack-wolf', 'well-alpha'):
    bpy.ops.wm.open_mainfile(filepath=str(ROOT / 'blender-cast' / (name + '.blend')))
    s = bpy.context.scene
    anchor_y = 64 if name == 'well-alpha' else 80
    s.camera.data.shift_y = 0
    bpy.context.view_layer.update()
    origin = world_to_camera_view(s, s.camera, Vector((0, 0, 0)))
    s.camera.data.shift_y = origin.y - (96 - anchor_y) / 96
    rig = next(o for o in s.objects if o.type == 'ARMATURE' and not o.name.startswith('MH_'))
    action = bpy.data.actions['Walking']
    rig.animation_data_create()
    rig.animation_data.action = action
    if action.slots:
        rig.animation_data.action_slot = action.slots[0]
    transforms = [(o, o.matrix_world.copy()) for o in s.objects if o.type in ('CAMERA', 'LIGHT')]
    start, end = action.frame_range
    print(name, 'range', start, end, 'bones', [b.name for b in rig.pose.bones], flush=True)
    for d, direction in enumerate(('front', 'right', 'back', 'left')):
        rot = Matrix.Rotation(-d * math.pi / 2, 4, 'Z')
        for o, m in transforms:
            o.matrix_world = rot @ m
        frames = []
        for i in range(8):
            frame = round(start + (end - start) * i / 8)
            s.frame_set(frame)
            p = OUT / f'{name}-walk-{direction}-{i:02d}.png'
            s.render.filepath = str(p)
            bpy.ops.render.render(write_still=True)
            frames.append({'file': p.name, 'blender_frame': frame})
        records.append({'clip': f'{name}-walk-{direction}', 'frames': frames,
            'frame': [96, 96], 'anchor': [48, anchor_y], 'pixels_per_metre': 48})
    for o, m in transforms:
        o.matrix_world = m
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT / (name + '-walking.blend')), compress=True)
(OUT / 'manifest.json').write_text(json.dumps(records, indent=2))
print('WOLF_WALK_RENDER_COMPLETE', len(records))
