"""Retarget one recorded CMU 02_05 punch onto the existing dressed player rigs.

Uses recorded limb directions and preserves the anatomical rest spine. The
already simulated cloth is transported through its skin weights, then reviewed.
"""
import bpy, addon_utils, math, json, hashlib
from pathlib import Path
from mathutils import Matrix, Vector

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'
ART = Path('Z:/Code/.worktrees/wizard-art-player-guides/art_studies/starter-derivatives-v01')
OUT = ROOT / 'blender-player'
SOURCE = ROOT / 'mocap/02_05.bvh'
PHASES = [120, 134, 148, 162, 176, 190, 204, 218]
addon_utils.enable('io_anim_bvh')
for sex in ('male', 'female'):
    source = (ART / 'paint-v2/blender/male-walk-linen.blend' if sex == 'male'
              else OUT / 'female-walk-center-braid.blend')
    bpy.ops.wm.open_mainfile(filepath=str(source))
    s = bpy.context.scene
    rig = bpy.data.objects[f'MH_{sex}_Rig']
    cloth = bpy.data.objects[f'{sex}_walk_baked_cloth_samples']
    s.frame_set(72)
    deps = bpy.context.evaluated_depsgraph_get()
    cloth_eval = cloth.evaluated_get(deps)
    old_points = [v.co.copy() for v in cloth_eval.data.vertices]
    rest = {b.name: b.matrix_local.copy() for b in rig.data.bones}
    old_skin = {b.name: b.matrix @ rest[b.name].inverted() for b in rig.pose.bones}
    groups = {g.index: g.name for g in cloth.vertex_groups}
    weights = []
    for v in cloth.data.vertices:
        w = [(groups[g.group], g.weight) for g in v.groups if groups[g.group] in rest and g.weight > 0]
        total = sum(t for _, t in w)
        weights.append([(n, t / total) for n, t in w])
    to_rig = rig.matrix_world.inverted() @ cloth.matrix_world
    from_rig = to_rig.inverted()
    neutral_cloth = []
    for co, w in zip(old_points, weights):
        matrix = Matrix(((0, 0, 0, 0),) * 4)
        for n, t in w:
            matrix += old_skin[n] * t
        neutral_cloth.append(matrix.inverted() @ to_rig @ co)
    rig.animation_data.action = None
    bpy.ops.import_anim.bvh(filepath=str(SOURCE), axis_forward='-Z', axis_up='Y',
                            global_scale=.0254, use_fps_scale=False)
    src = bpy.context.object
    src.name = 'CMU_02_05_recorded_punch'
    src.hide_render = True
    s.frame_set(170)
    lateral = src.pose.bones['LeftArm'].head - src.pose.bones['RightArm'].head
    rotation = Matrix.Rotation(-math.atan2(lateral.y, lateral.x), 3, 'Z')
    scale = (rig.data.bones['thigh_l'].length + rig.data.bones['calf_l'].length) / (src.data.bones['LeftUpLeg'].length + src.data.bones['LeftLeg'].length)
    mapping = []
    for side, prefix in [('l', 'Left'), ('r', 'Right')]:
        mapping += [(f'thigh_{side}', prefix+'UpLeg', prefix+'Leg'),
                    (f'calf_{side}', prefix+'Leg', prefix+'Foot'),
                    (f'foot_{side}', prefix+'Foot', prefix+'ToeBase'),
                    (f'upperarm_{side}', prefix+'Arm', prefix+'ForeArm'),
                    (f'lowerarm_{side}', prefix+'ForeArm', prefix+'Hand')]

    def aim(name, direction):
        b = rig.pose.bones[name]
        h = b.head.copy()
        q = (b.tail-b.head).rotation_difference(direction)
        b.matrix = Matrix.Translation(h) @ q.to_matrix().to_4x4() @ Matrix.Translation(-h) @ b.matrix
        bpy.context.view_layer.update()

    poses, feet = [], []
    for frame in PHASES:
        s.frame_set(frame)
        joints = {b.name: rotation @ b.head for b in src.pose.bones}
        for b in rig.pose.bones:
            b.matrix_basis = Matrix.Identity(4)
        hip_z = (joints['LeftUpLeg'].z+joints['RightUpLeg'].z)*.5*scale
        rig.pose.bones['Root'].location.z = hip_z-(rest['thigh_l'].translation.z+rest['thigh_r'].translation.z)*.5
        bpy.context.view_layer.update()
        tilt = Vector((0, 0, 1)).rotation_difference((joints['Neck']-joints['Hips']).normalized())
        for name in ['spine_01', 'spine_02', 'spine_03']:
            b = rig.data.bones[name]
            aim(name, tilt @ (b.tail_local-b.head_local))
        for name in ['neck_01', 'head']:
            b = rig.data.bones[name]
            aim(name, b.tail_local-b.head_local)
        for name, a, b in mapping:
            aim(name, joints[b]-joints[a])
        for side in ('l', 'r'):
            fore = rig.pose.bones['lowerarm_'+side]
            aim('hand_'+side, fore.tail-fore.head)
            for finger in ('index', 'middle', 'ring', 'pinky'):
                for link in (1, 2, 3):
                    b = rig.pose.bones[f'{finger}_{link:02d}_{side}']
                    b.rotation_mode = 'XYZ'
                    b.rotation_euler = (math.radians(65 if side == 'l' else -65), 0, 0)
        bpy.context.view_layer.update()
        poses.append({b.name: b.matrix_basis.copy() for b in rig.pose.bones})
        feet.append(min(rig.pose.bones['foot_'+side].tail.z for side in ('l', 'r')))
    ground = .025-min(feet)
    cloth.shape_key_clear()
    cloth.animation_data_clear()
    cloth.name = f'{sex}_attack_baked_cloth_samples'
    cloth.shape_key_add(name='Basis')
    for i, pose in enumerate(poses):
        s.frame_set(72+i*4)
        for b in rig.pose.bones:
            b.matrix_basis = pose[b.name]
            if b.name == 'Root':
                b.location.z += ground
        bpy.context.view_layer.update()
        skin = {b.name: b.matrix @ rest[b.name].inverted() for b in rig.pose.bones}
        key = cloth.shape_key_add(name=f'Recorded_punch_{i+1}')
        for v, co, w in zip(key.data, neutral_cloth, weights):
            matrix = Matrix(((0, 0, 0, 0),) * 4)
            for n, t in w:
                matrix += skin[n] * t
            v.co = from_rig @ matrix @ co
        for phase in range(8):
            key.value = 1 if phase == i else 0
            key.keyframe_insert('value', frame=72+phase*4)
        for b in rig.pose.bones:
            b.keyframe_insert('location', frame=72+i*4)
            b.keyframe_insert('rotation_quaternion' if b.rotation_mode == 'QUATERNION' else 'rotation_euler', frame=72+i*4)
    rig.animation_data.action.name = f'{sex}_CMU_02_05_punch'
    cloth['motion_source'] = 'CMU 02_05 punch/strike; weighted transport of existing cloth'
    renderer = ART / 'blender/render_baked_frames.py'
    dst = OUT / f'{sex}-attack-CMU.blend'
    exec(compile(renderer.read_text(), str(renderer), 'exec'), {'__file__': str(renderer),
        'SEX': sex, 'GAIT': 'attack', 'OUTPUT_DIR': str(OUT), 'SAVE_PATH': str(dst),
        'FRAMING_PATH': str(OUT / f'{sex}-attack-framing.json')})
    (OUT / f'{sex}-attack-source.json').write_text(json.dumps({'source': str(SOURCE),
        'sha256': hashlib.sha256(SOURCE.read_bytes()).hexdigest(), 'source_frames': PHASES,
        'source_fps': 120, 'scale': scale, 'ground_translation': ground,
        'review_status': 'unreviewed'}, indent=2))
