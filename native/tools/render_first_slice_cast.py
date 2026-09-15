"""Reuse the individual human meshes and Quaternius wolves, calibrated at48px/m."""
import bpy
import math
import json
import hashlib
from mathutils import Matrix, Vector
from bpy_extras.object_utils import world_to_camera_view
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'
ART = Path('Z:/Code/.worktrees/wizard-art-player-guides/art_studies')
OUT = ROOT / 'blender-cast'
OUT.mkdir(parents=True, exist_ok=True)
records = []
for name in ['field-hand', 'scribe', 'scout', 'defender', 'pack-wolf', 'well-alpha']:
    beast = 'wolf' in name or name == 'well-alpha'
    source = ART / ('starter-slice-v01' if beast else 'starter-slice-v02-identities') / 'sources' / (name + '.blend')
    bpy.ops.wm.open_mainfile(filepath=str(source))
    s = bpy.context.scene
    c = s.camera
    c.location = (0, -10.035423, 7.896877)
    c.rotation_euler = (math.radians(55), 0, 0)
    c.data.sensor_fit = 'VERTICAL'
    c.data.shift_x = c.data.shift_y = 0
    s.render.resolution_x = s.render.resolution_y = 96
    s.render.resolution_percentage = 100
    bpy.context.view_layer.update()
    depth = -(c.matrix_world.inverted() @ Vector((0, 0, 0))).z
    c.data.lens = 48 * depth * c.data.sensor_height / 96
    bpy.context.view_layer.update()
    origin = world_to_camera_view(s, c, Vector((0, 0, 0)))
    c.data.shift_y = origin.y - 16 / 96
    if beast:
        bpy.data.objects[name + '_normalization'].rotation_euler.z = math.radians(-90)
        bpy.data.objects[name + '_facing'].rotation_euler.z = 0
    transforms = [(o, o.matrix_world.copy()) for o in s.objects if o.type in ('CAMERA', 'LIGHT')]
    s.render.engine = 'CYCLES'
    s.cycles.samples = 48
    s.cycles.use_denoising = False
    s.render.film_transparent = True
    s.render.image_settings.color_mode = 'RGBA'
    files = []
    for i, direction in enumerate(['front', 'right', 'back', 'left']):
        rot = Matrix.Rotation(-i * math.pi / 2, 4, 'Z')
        for o, transform in transforms:
            o.matrix_world = rot @ transform
        p = OUT / f'{name}-{direction}.png'
        s.render.filepath = str(p)
        bpy.ops.render.render(write_still=True)
        files.append({'direction': direction, 'file': p.name, 'sha256': hashlib.sha256(p.read_bytes()).hexdigest()})
    for o, transform in transforms:
        o.matrix_world = transform
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT / (name + '.blend')), compress=True)
    records.append({'actor': name, 'source': str(source), 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
        'frame': [96, 96], 'anchor': [48, 80], 'pixels_per_metre': 48, 'frames': files,
        'actions': [a.name for a in bpy.data.actions]})
(OUT / 'manifest.json').write_text(json.dumps(records, indent=2))
print('FIRST_SLICE_CAST_COMPLETE', len(records))
