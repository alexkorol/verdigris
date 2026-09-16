"""Read actual fist/club forward reach from Blender source samples."""
import bpy,json,sys
from pathlib import Path
OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat'
fist='fist' in sys.argv;aligned='aligned' in sys.argv or fist;results=[]
for sex in ['male','female']:
 for action in ['attack','unarmed-attack']:
  source=OUT/('contact-v5-fist' if fist and action=='unarmed-attack' else 'contact-v4-repaired')/f'{sex}-{action}'/('club-attack-transition.blend' if action=='attack' else 'unarmed-attack-transition.blend') if aligned else OUT/'blender'/f'{sex}-{action}.blend'
  bpy.ops.wm.open_mainfile(filepath=str(source));s=bpy.context.scene;rig=bpy.data.objects[f'MH_{sex}_Rig'];rows=[];endpoint_poses=[]
  for i in range(16 if aligned else 8):
   s.frame_set(1+4*i);dep=bpy.context.evaluated_depsgraph_get()
   if action=='attack':
    club=bpy.data.objects[f'{sex}_starter_branch_club'].evaluated_get(dep);pts=[club.matrix_world@v.co for v in club.data.vertices]
   else:pts=[rig.matrix_world@rig.pose.bones[f'{finger}_{j:02d}_r'].head for finger in ['index','middle','ring','pinky'] for j in [1,2,3]]
   point=min(pts,key=lambda p:p.y);rows.append({'phase':i,'frame':1+4*i,'normalized_effect_phase':i/16 if aligned else None,'farthest_forward_world':list(point),'forward_reach_m':-point.y})
   if aligned and i in [0,15]:endpoint_poses.append({b.name:[v for row in b.matrix for v in row] for b in rig.pose.bones})
  peak=max(rows,key=lambda r:r['forward_reach_m'])['phase'];club=bpy.data.objects.get(f'{sex}_starter_branch_club');endpoint_error=max(abs(a-b) for n in endpoint_poses[0] for a,b in zip(endpoint_poses[0][n],endpoint_poses[1][n])) if aligned else None
  if aligned:
   assert peak==8,(sex,action,peak)
   assert endpoint_error<1e-6,endpoint_error
   assert action!='unarmed-attack' or club is None or club.hide_render
  results.append({'sex':sex,'action':action,'source':str(source),'forward_axis':'negative world Y','contact_landmark_definition':'Farthest forward club surface vertex, or right finger joints for punch; forward extent is a visible contact proxy, not a collision solver.','samples':rows,'peak_phase':peak,'normalized_contact_phase':.5 if aligned else None,'endpoint_bone_matrix_max_error':endpoint_error,'equipment_hidden':bool(club and club.hide_render)})
(OUT/'provenance'/('fist-contact-reach.json' if fist else 'aligned-contact-reach.json' if aligned else 'source-contact-reach.json')).write_text(json.dumps(results,indent=2));print(json.dumps(results))
