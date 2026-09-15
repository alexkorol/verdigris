"""Extract only used CC0 motion clips from the public Quaternius library."""
import bpy,json,hashlib,sys
from pathlib import Path
OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat'
if '--' not in sys.argv:raise SystemExit('Usage: blender -b --python render_starter_combat_library.py -- /path/to/AnimationLibrary_Godot_Standard.gltf')
source=Path(sys.argv[sys.argv.index('--')+1])
bpy.ops.wm.read_factory_settings(use_empty=True);bpy.ops.import_scene.gltf(filepath=str(source))
r=next(o for o in bpy.data.objects if o.type=='ARMATURE');r.animation_data_clear()
keep=['Sword_Attack','Hit_Chest','Death01','Punch_Cross']
for a in list(bpy.data.actions):
 if a.name not in keep:bpy.data.actions.remove(a)
 else:a.use_fake_user=True
for o in list(bpy.data.objects):
 if o!=r:bpy.data.objects.remove(o,do_unlink=True)
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'mocap/selected-animation-library.blend'),compress=True)
(OUT/'mocap/source.json').write_text(json.dumps({'creator':'Quaternius / Gonzalo Furnier','license':'CC0-1.0','primary':'https://quaternius.itch.io/universal-animation-library','mirror':'https://github.com/J-Ponzo/gltf-universal-animation-library','mirror_distribution_date':'2025-06-10','selected_actions':keep,'input_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'note':'Source subset only; palm-frame retarget avoids copying source rig elbow twist.'},indent=2))
