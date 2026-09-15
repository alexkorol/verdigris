"""Render authored Quaternius wolf idle actions with the established48px/m camera."""
import bpy, math, json, hashlib, shutil
from pathlib import Path
from mathutils import Matrix
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice'
OUT=ROOT/'monsters-v2'
REF=OUT/'references'
REF.mkdir(parents=True,exist_ok=True)
clips={}
for actor in ('pack-wolf','well-alpha'):
    source=ROOT/'blender-wolves'/f'{actor}-walking.blend'
    bpy.ops.wm.open_mainfile(filepath=str(source))
    scene=bpy.context.scene
    scene.render.threads_mode='FIXED';scene.render.threads=4
    rig=bpy.data.objects['WolfArmature']
    transforms=[(o,o.matrix_world.copy()) for o in scene.objects if o.type in ('CAMERA','LIGHT')]
    for action_name in ('walk','idle'):
        action=bpy.data.actions['Walking' if action_name=='walk' else 'Idle']
        rig.animation_data.action=action
        if action.slots: rig.animation_data.action_slot=action.slots[0]
        start,end=action.frame_range
        for d,direction in enumerate(('front','right','back','left')):
            for o,m in transforms: o.matrix_world=Matrix.Rotation(-d*math.pi/2,4,'Z')@m
            clip=f'{actor}-{action_name}-{direction}'
            frames=[]
            for phase in range(8):
                p=REF/f'{clip}-{phase:02d}.png'
                f=round(start+(end-start)*phase/8)
                if action_name=='walk': shutil.copy2(ROOT/'blender-wolves'/p.name,p)
                else:
                    scene.frame_set(f);scene.render.filepath=str(p);bpy.ops.render.render(write_still=True)
                frames.append({'src':'references/'+p.name,'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'blender_frame':f})
            clips[clip]={'actor':actor,'action':action_name,'direction':direction,'fps':10,'loop':True,'frame':[96,96],'anchor':[48,64 if actor=='well-alpha' else 80],'pixels_per_metre':48,'frames':frames,'blender_source':str(source)}
    for o,m in transforms:o.matrix_world=m
(OUT/'reference-manifest.json').write_text(json.dumps({'clips':clips},indent=2))
print('WOLF_REFERENCES_COMPLETE',len(clips),flush=True)
