"""Author short wolf combat clips on the existing skinned Quaternius rig.
Transforms act in armature space; no image warps or animation-frame relabelling.
"""
import bpy, math, json, hashlib
from pathlib import Path
from mathutils import Matrix, Vector
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice'
OUT=ROOT/'monsters-v2';REF=OUT/'references';BLEND=OUT/'blender';BLEND.mkdir(exist_ok=True)
manifest=json.loads((OUT/'reference-manifest.json').read_text())
for actor in ('pack-wolf','well-alpha'):
    bpy.ops.wm.open_mainfile(filepath=str(ROOT/'blender-wolves'/f'{actor}-walking.blend'))
    s=bpy.context.scene;s.render.threads_mode='FIXED';s.render.threads=4
    rig=bpy.data.objects['WolfArmature'];rig.animation_data.action=bpy.data.actions['Idle']
    if rig.animation_data.action.slots:rig.animation_data.action_slot=rig.animation_data.action.slots[0]
    s.frame_set(0);bpy.context.view_layer.update()
    base={b.name:b.matrix_basis.copy() for b in rig.pose.bones}
    rig.animation_data.action=None
    norm=bpy.data.objects[actor+'_normalization'];normbase=norm.matrix_basis.copy()
    camera=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ('CAMERA','LIGHT')]
    # Grounded folded-leg death is authored separately by first_slice_monsters_death_v2.py.
    for action_name in ('attack','hit'):
        rig.animation_data.action=None;norm.animation_data_clear();norm.matrix_basis=normbase
        act=bpy.data.actions.new(f'Verdigris_{actor}_{action_name}')
        rig.animation_data.action=act
        for phase in range(4):
            s.frame_set(phase)
            for b in rig.pose.bones:b.matrix_basis=base[b.name]
            norm.matrix_basis=normbase
            bpy.context.view_layer.update()
            def rotate(name,angle):
                b=rig.pose.bones[name];pivot=b.head.copy()
                b.matrix=Matrix.Translation(pivot)@Matrix.Rotation(math.radians(angle),4,'Y')@Matrix.Translation(-pivot)@b.matrix
                bpy.context.view_layer.update()
            if action_name=='attack':
                # Anticipation, low neck extension, contact, recover. Paws stay planted.
                rotate('Bone.002',(-8,14,22,0)[phase]);rotate('Bone.003',(0,-8,-12,0)[phase])
                torso=rig.pose.bones['Bone.001'];m=torso.matrix.copy();m.translation.x+=(0,.14,.23,0)[phase];torso.matrix=m
            elif action_name=='hit':
                rotate('Bone.002',(0,-11,-5,0)[phase]);rotate('Bone.003',(0,8,4,0)[phase])
                torso=rig.pose.bones['Bone.001'];m=torso.matrix.copy();m.translation.x+=(0,-.20,-.10,0)[phase];torso.matrix=m
            for b in rig.pose.bones:
                b.keyframe_insert('location',frame=phase);b.keyframe_insert('rotation_quaternion',frame=phase);b.keyframe_insert('rotation_euler',frame=phase);b.keyframe_insert('scale',frame=phase)
            norm.keyframe_insert('location',frame=phase);norm.keyframe_insert('rotation_euler',frame=phase);norm.keyframe_insert('scale',frame=phase)
        for d,direction in enumerate(('front','right','back','left')):
            for o,m in camera:o.matrix_world=Matrix.Rotation(-d*math.pi/2,4,'Z')@m
            clip=f'{actor}-{action_name}-{direction}';frames=[]
            for phase in range(4):
                s.frame_set(phase);p=REF/f'{clip}-{phase:02d}.png';s.render.filepath=str(p);bpy.ops.render.render(write_still=True)
                frames.append({'src':'references/'+p.name,'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'blender_frame':phase})
            manifest['clips'][clip]={'actor':actor,'action':action_name,'direction':direction,'fps':8,'loop':False,'frame':[96,96],'anchor':[48,64 if actor=='well-alpha' else 80],'pixels_per_metre':48,'frames':frames,'authored_action':act.name,'blender_source':f'blender/{actor}-{action_name}.blend'}
        for o,m in camera:o.matrix_world=m
        bpy.ops.wm.save_as_mainfile(filepath=str(BLEND/f'{actor}-{action_name}.blend'),compress=True)
(OUT/'reference-manifest.json').write_text(json.dumps(manifest,indent=2))
print('WOLF_COMBAT_REFERENCES_COMPLETE')
