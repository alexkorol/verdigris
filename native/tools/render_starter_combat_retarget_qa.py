"""Compare Quaternius and target foot heights independently of floor shifts."""
import bpy,json,math
from pathlib import Path
from mathutils import Vector
from bpy_extras.object_utils import world_to_camera_view
OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat';rows=[]
for sex in ['male','female']:
 for action in ['attack','hit','death','unarmed-attack']:
  bpy.ops.wm.open_mainfile(filepath=str(OUT/'blender'/f'{sex}-{action}.blend'));s=bpy.context.scene;target=bpy.data.objects[f'MH_{sex}_Rig'];source=bpy.data.objects['Rig']
  s.frame_set(0);src={b.name:b.head.copy() for b in source.pose.bones}
  s.frame_set(1);tgt={b.name:b.head.copy() for b in target.pose.bones};rootz=target.pose.bones['Root'].location.z
  row={'sex':sex,'action':action,'source_frame':0,'target_frame':1,'target_root_z_adjustment_m':rootz,'source_object_matrix_world':[list(r) for r in source.matrix_world],'feet':{},'segment_direction_error_degrees':{}}
  for side,suffix in [('l','L'),('r','R')]:
   row['feet'][side]={'source_ankle':list(src[f'DEF-foot.{suffix}']),'source_toe':list(src[f'DEF-toe.{suffix}']),'target_ankle':list(tgt[f'foot_{side}']),'target_toe':list(tgt[f'ball_{side}']),'target_ankle_z_before_floor_shift':tgt[f'foot_{side}'].z-rootz,'target_toe_z_before_floor_shift':tgt[f'ball_{side}'].z-rootz}
   for name,sa,sb,ta,tb in [('thigh','thigh','shin','thigh','calf'),('calf','shin','foot','calf','foot'),('foot','foot','toe','foot','ball')]:
    v=(src[f'DEF-{sb}.{suffix}']-src[f'DEF-{sa}.{suffix}']).normalized();w=(tgt[f'{tb}_{side}']-tgt[f'{ta}_{side}']).normalized();row['segment_direction_error_degrees'][f'{name}_{side}']=math.degrees(v.angle(w))
  row['source_ankle_height_difference_m']=abs(row['feet']['l']['source_ankle'][2]-row['feet']['r']['source_ankle'][2]);row['target_ankle_height_difference_m']=abs(row['feet']['l']['target_ankle'][2]-row['feet']['r']['target_ankle'][2]);rows.append(row)
(OUT/'provenance/retarget-grounding-audit.json').write_text(json.dumps(rows,indent=2));print(json.dumps([{'sex':r['sex'],'action':r['action'],'source_ankle_dz':r['source_ankle_height_difference_m'],'target_ankle_dz':r['target_ankle_height_difference_m'],'max_direction_error_deg':max(r['segment_direction_error_degrees'].values())} for r in rows]))
endpoints=[]
for sex in ['male','female']:
 for action in ['hit','death']:
  sourcefile=OUT/'transition-v3-skin'/f'{sex}-{action}'/f'club-{action}.blend';bpy.ops.wm.open_mainfile(filepath=str(sourcefile));s=bpy.context.scene;rig=bpy.data.objects[f'MH_{sex}_Rig'];r={'sex':sex,'action':action,'source':str(sourcefile),'frames':[]}
  for i in [0,7]:
   s.frame_set(1+i*4);dep=bpy.context.evaluated_depsgraph_get();height=[]
   for o in s.objects:
    if o.type=='MESH' and not o.hide_render and len(o.data.vertices) and o.name.startswith((f'{sex}_',f'MH_{sex}_',f'player_{sex}_')) and any(not c.hide_render for c in o.users_collection):
     co=[(o.matrix_world@v.co).z for v in o.evaluated_get(dep).data.vertices]
     if co:height.append({'object':o.name,'min_z':min(co)})
   size=s.render.resolution_x;origin=world_to_camera_view(s,s.camera,Vector((0,0,0)));p1=world_to_camera_view(s,s.camera,Vector((-.5,0,0)));p2=world_to_camera_view(s,s.camera,Vector((.5,0,0)))
   r['frames'].append({'phase':i,'root_location':list(rig.pose.bones['Root'].location),'head_world':list(rig.matrix_world@rig.pose.bones['head'].head),'ankles_world':{side:list(rig.matrix_world@rig.pose.bones[f'foot_{side}'].head) for side in ['l','r']},'lowest_visible_meshes':sorted(height,key=lambda x:x['min_z'])[:8],'world_origin_pixel':[origin.x*size,(1-origin.y)*size],'pixels_per_metre_at_origin':(p2.x-p1.x)*size})
  endpoints.append(r)
(OUT/'provenance/reaction-endpoint-audit.json').write_text(json.dumps(endpoints,indent=2));print('REACTION_ENDPOINTS',json.dumps(endpoints))
