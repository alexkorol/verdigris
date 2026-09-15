"""Export existing editable village models at the same 48px/m player plane."""
import bpy
from bpy_extras.object_utils import world_to_camera_view
from mathutils import Vector
from pathlib import Path
import json
import hashlib

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'
SOURCE = Path('Z:/Code/.worktrees/wizard-art-player-guides/art_studies/starter-derivatives-v01/blender/village-parity.blend')
OUT = ROOT / 'blender-props'
OUT.mkdir(parents=True, exist_ok=True)
bpy.ops.wm.open_mainfile(filepath=str(SOURCE))
s = bpy.data.scenes['Village_player_plane_calibration']
bpy.context.window.scene = s
c = s.camera
c.data.sensor_fit = 'VERTICAL'
camera_matrix = c.matrix_world.copy()
meshes = [o for o in s.objects if o.type == 'MESH']
specs = [('hut', 'parity_dwelling_west', (-3.15, 3.3, 0), (240, 240), 56),
         ('well', 'parity_well', (1.45, 1.6, 0), (96, 128), 28),
         ('fence', 'parity_fence', (-.3, 4.25, 0), (192, 96), 16),
         ('tree', 'parity_tree_west', (-4.55, .6, 0), (240, 240), 16)]
records = []
for name, prefix, pivot, size, bottom_margin in specs:
    for o in meshes:
        o.hide_render = not o.name.startswith(prefix)
        o.visible_camera = True
    c.matrix_world = camera_matrix.copy()
    c.location += Vector(pivot)
    c.data.shift_x = c.data.shift_y = 0
    s.render.resolution_x, s.render.resolution_y = size
    s.render.resolution_percentage = 100
    bpy.context.view_layer.update()
    depth = -(c.matrix_world.inverted() @ Vector(pivot)).z
    c.data.lens = 48 * depth * c.data.sensor_height / size[1]
    bpy.context.view_layer.update()
    p = world_to_camera_view(s, c, Vector(pivot))
    c.data.shift_y = p.y - bottom_margin / size[1]
    s.render.film_transparent = True
    s.render.image_settings.color_mode = 'RGBA'
    s.render.engine = 'CYCLES'
    s.cycles.samples = 64
    s.cycles.use_denoising = False
    s.render.filepath = str(OUT / f'{name}.png')
    bpy.ops.render.render(write_still=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT / f'{name}.blend'), compress=True)
    records.append({'id': name, 'size': size, 'anchor': [size[0] // 2, size[1] - bottom_margin],
                    'pixels_per_metre': 48, 'source_objects': prefix,
                    'pivot': pivot, 'camera_depth': depth})
(OUT / 'manifest.json').write_text(json.dumps({'source': str(SOURCE),
    'source_sha256': hashlib.sha256(SOURCE.read_bytes()).hexdigest(), 'assets': records}, indent=2))
print('FIRST_SLICE_PROPS_COMPLETE', len(records))
