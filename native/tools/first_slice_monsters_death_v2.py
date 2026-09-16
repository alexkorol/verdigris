"""Grounded side fall with folded wolf limbs, on the existing Quaternius rig.

Writes an isolated reference handoff only. Never modifies approved paint or the
active monster reference manifest. All views show the same physical animation.
"""
import hashlib
import json
import math
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

BASE = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'
ROOT = BASE / 'monsters-v2'
OUT = ROOT / 'death-v2'
OUT.mkdir(parents=True, exist_ok=True)
manifest = {'description': 'Authored fall with folded elbows/hocks and grounded body; four cardinal views of the same rig pose', 'clips': {}}

for actor in ('pack-wolf', 'well-alpha'):
    source = (ROOT / 'accepted/blender/pack-wolf-walking.blend' if actor == 'pack-wolf'
              else ROOT / 'blender/well-alpha-idle-canvas128.blend')
    bpy.ops.wm.open_mainfile(filepath=str(source))
    scene = bpy.context.scene
    scene.render.threads_mode = 'FIXED'
    scene.render.threads = 3
    scene.cycles.samples = 32
    scene.render.film_transparent = True
    scene.render.image_settings.color_mode = 'RGBA'
    scene.render.resolution_x = 96 if actor == 'pack-wolf' else 128
    scene.render.resolution_y = 96
    # The falling forebody projects farther toward the camera than the standing
    # muzzle. Allocate four more bottom pixels in Blender, not by cropping or
    # fitting the generated result. The matching pivot keeps world placement.
    anchor = [48, 76] if actor == 'pack-wolf' else [64, 64]
    if actor == 'pack-wolf':
        scene.camera.data.shift_y -= 4 / 96
    rig = bpy.data.objects['WolfArmature']
    idle = bpy.data.actions['Idle']
    rig.animation_data.action = idle
    if idle.slots:
        rig.animation_data.action_slot = idle.slots[0]
    scene.frame_set(0)
    bpy.context.view_layer.update()
    basis = {bone.name: bone.matrix_basis.copy() for bone in rig.pose.bones}
    ik_pose = {bone.name: bone.matrix.copy() for bone in rig.pose.bones if bone.name.startswith('IK')}
    normalization = bpy.data.objects[actor + '_normalization']
    initial_normalization = normalization.matrix_basis.copy()
    normalization.animation_data_clear()
    rig.animation_data.action = None
    action = bpy.data.actions.new('Verdigris_' + actor + '_death_folded_v2')
    rig.animation_data.action = action
    geometry = [obj for obj in scene.objects if obj.type == 'MESH' and not obj.hide_render
                and any(mod.type == 'ARMATURE' and mod.object == rig for mod in obj.modifiers)]
    poses = []

    def rotate_bone(name, degrees):
        bone = rig.pose.bones[name]
        pivot = bone.head.copy()
        bone.matrix = Matrix.Translation(pivot) @ Matrix.Rotation(math.radians(degrees), 4, 'Y') @ Matrix.Translation(-pivot) @ bone.matrix
        bpy.context.view_layer.update()

    for phase in range(4):
        scene.frame_set(phase)
        for bone in rig.pose.bones:
            bone.matrix_basis = basis[bone.name]
        normalization.matrix_basis = initial_normalization
        bpy.context.view_layer.update()
        amount = (0., .3, .8, 1.)[phase]
        # Move authored IK targets rather than rotating constrained lower bones.
        # Forefeet tuck backward, hindfeet forward; both fold up toward belly.
        for name, rest in ik_pose.items():
            target = rest.copy()
            fore = 'Front' in name
            target.translation += Vector(((-.85 if fore else .75) * amount, 0., (1.25 if fore else 1.10) * amount))
            rig.pose.bones[name].matrix = target
        bpy.context.view_layer.update()
        rotate_bone('Bone.002', 30 * amount)
        rotate_bone('Bone.003', 12 * amount)
        normalization.matrix_basis = initial_normalization @ Matrix.Rotation(math.radians((0, 24, 67, 94)[phase]), 4, 'X')
        bpy.context.view_layer.update()
        deps = bpy.context.evaluated_depsgraph_get()
        vertices = [obj.evaluated_get(deps).matrix_world @ vertex.co
                    for obj in geometry for vertex in obj.evaluated_get(deps).data.vertices]
        lowest = min(vertex.z for vertex in vertices)
        normalization.location.z -= lowest
        bpy.context.view_layer.update()
        poses.append({'phase': phase, 'roll_degrees': (0, 24, 67, 94)[phase],
                      'fold_amount': amount, 'ground_lift': -lowest,
                      'body_height_metres': max(v.z for v in vertices) - lowest})
        for bone in rig.pose.bones:
            for prop in ('location', 'rotation_quaternion', 'rotation_euler', 'scale'):
                bone.keyframe_insert(prop, frame=phase)
        for prop in ('location', 'rotation_euler', 'scale'):
            normalization.keyframe_insert(prop, frame=phase)

    scene.frame_start, scene.frame_end = 0, 3
    transforms = [(obj, obj.matrix_world.copy()) for obj in scene.objects if obj.type in ('CAMERA', 'LIGHT')]
    for index, direction in enumerate(('front', 'right', 'back', 'left')):
        for obj, transform in transforms:
            obj.matrix_world = Matrix.Rotation(-index * math.pi / 2, 4, 'Z') @ transform
        frames = []
        for phase in range(4):
            scene.frame_set(phase)
            path = OUT / f'{actor}-death-{direction}-{phase:02d}.png'
            scene.render.filepath = str(path)
            bpy.ops.render.render(write_still=True)
            frames.append({'file': path.name, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'blender_frame': phase})
        manifest['clips'][f'{actor}-death-{direction}'] = {
            'actor': actor, 'action': 'death', 'direction': direction, 'fps': 8, 'loop': False,
            'frame': [scene.render.resolution_x, 96], 'anchor': anchor,
            'pixels_per_metre': 48, 'frames': frames, 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
            'blender': actor + '-death-v2.blend', 'authored_action': action.name, 'physical_poses': poses,
            'visual_acceptance': False}
    for obj, transform in transforms:
        obj.matrix_world = transform
    scene.frame_set(3)
    blend = OUT / f'{actor}-death-v2.blend'
    bpy.ops.wm.save_as_mainfile(filepath=str(blend), compress=True)
    for record in manifest['clips'].values():
        if record['actor'] == actor:
            record['blender_sha256'] = hashlib.sha256(blend.read_bytes()).hexdigest()
(OUT / 'manifest.json').write_text(json.dumps(manifest, indent=2))
print('FOLDED_DEATH_REFERENCES_COMPLETE', len(manifest['clips']))
