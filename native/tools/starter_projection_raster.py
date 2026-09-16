"""Raster-only final cohort from packed projected scenes plus explicit overrides.

No geometry, UV, texture, shader, animation, framing or anchoring edits occur.
Usage (Blender): -- male source-cohort destination --filter BOX --override cohort
Accepted archive: -- male accepted/male new-cohort --archive [--verify-only]
"""
import bpy,json,sys,hashlib,math
from pathlib import Path
from mathutils import Matrix,Vector
from bpy_extras.object_utils import world_to_camera_view
args=sys.argv[sys.argv.index('--')+1:];sex,source,destination=args[:3]
assert sex in ['male','female']
def option(name,default):return args[args.index(name)+1] if name in args else default
filter_type=option('--filter','BOX')
assert filter_type in ['BOX','BLACKMAN_HARRIS']
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-projection'
cohorts=[source]+[args[i+1] for i,a in enumerate(args) if a=='--override']
out=ROOT/destination
assert out.resolve().is_relative_to(ROOT.resolve()) and out.resolve()!=ROOT.resolve()
archive='--archive' in args;verify_only='--verify-only' in args
assert not verify_only or archive, 'Verification-only mode requires an accepted archive'
assert not archive or len(cohorts)==1, 'An accepted archive is already a complete cohort'
reports={}
for cohort in cohorts:
 for path in (ROOT/cohort/'reports').glob(f'{sex}-*-projection.json'):
  report=json.loads(path.read_text());reports[report['gait']]=(cohort,path,report)
assert len(reports)==12, 'Expected both equipment variants of six actions'
if archive:
 archive_root=(ROOT/source).parent;manifest=json.loads((archive_root/'manifest.json').read_text())
 for action,(cohort,path,report) in reports.items():
  provenance=manifest['provenance'][f'{sex}-{action}-front'];packed=ROOT/cohort/'scenes'/f'{sex}-{action}-projection.blend'
  assert packed.resolve()==(archive_root/provenance['scene']).resolve()
  assert path.resolve()==(archive_root/provenance['projection_report']).resolve()
  assert hashlib.sha256(packed.read_bytes()).hexdigest()==provenance['scene_sha256']
  assert hashlib.sha256(path.read_bytes()).hexdigest()==provenance['projection_report_sha256']
  assert 'attack' not in action or report.get('attack_source_manifest')
 print('ARCHIVE_HASH_VERIFIED',sex,len(reports),flush=True)
 if verify_only:sys.exit(0)
assert not out.exists(), 'Final raster cohorts are immutable; choose a new version'
for d in ['frames','blender','reports']:(out/d).mkdir(parents=True)
for action,(cohort,path,report) in sorted(reports.items()):
 packed=ROOT/cohort/('scenes' if archive else 'blender')/f'{sex}-{action}-projection.blend'
 if report.get('target_file') and not archive:
  assert hashlib.sha256(Path(report['target_file']).read_bytes()).hexdigest()==report['target_sha256'], 'Source scene changed after projection preparation'
 if 'attack' in action and not report.get('attack_source_manifest'):
  original=Path(report['target_file']);assert hashlib.sha256(original.read_bytes()).hexdigest()==report['target_sha256']
  timing_path=original.parent/'manifest.json';timing=json.loads(timing_path.read_text());timing_clips=timing['clips']
  assert all(c['contact_index']==8 and len(c['frames'])==16 for c in timing_clips)
  report['attack_source_manifest']={'path':str(timing_path),'sha256':hashlib.sha256(timing_path.read_bytes()).hexdigest(),'motion':timing['source_motion'],'phase_mapping':[{'index':f['index'],'role':f['role'],'source_pose':f['source_pose']} for f in timing_clips[0]['frames']]}
 bpy.ops.wm.open_mainfile(filepath=str(packed));s=bpy.context.scene
 s.cycles.pixel_filter_type=filter_type;s.cycles.filter_width=1.0
 frames=report.get('source_frames',list(range(72,104,4)) if any(a in action for a in ['walk','sprint']) else list(range(1,17 if 'idle' in action else 33,4)))
 report['source_cohort']=cohort;report['copied_from_cohort']=cohort;report['cohort']=destination
 if sex=='female':report['method']='Four views from exact refined female walk frame72. Her accepted front paint and registered side/back paint; depth visibility, normal weights and UV transfer by unchanged mesh vertex index. Separately recorded intentional braid curve-width revision.'
 report['raster_pass']={'operation':'Only reconstruction filter changed; unchanged geometry/materials/animation/camera.','filter_type':filter_type,'filter_width':1.0,'source_packed_scene_sha256':hashlib.sha256(packed.read_bytes()).hexdigest(),'source_report_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'script_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}
 s.frame_set(frames[0]);report['render_complete']=False
 bpy.ops.wm.save_as_mainfile(filepath=str(out/'blender'/packed.name),compress=True)
 target_report=out/'reports'/path.name
 target_report.write_text(json.dumps(report,indent=2),encoding='utf-8')
 transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']]
 size=report['camera']['frame'][0];anchor=report['camera']['anchor']
 for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
  for o,m in transforms:o.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@m
  for phase,frame in enumerate(frames):
   s.frame_set(frame);p=world_to_camera_view(s,s.camera,Vector())
   assert abs(p.x*size-anchor[0])<.01 and abs((1-p.y)*size-anchor[1])<.01
   s.render.filepath=str(out/'frames'/f'{sex}-{action}-{direction}-{phase:02d}.png');bpy.ops.render.render(write_still=True)
 report['render_complete']=True;target_report.write_text(json.dumps(report,indent=2),encoding='utf-8')
 print('RASTER_CLIP_COMPLETE',sex,action,flush=True)
print('RASTER_COHORT_COMPLETE',sex,destination,flush=True)
