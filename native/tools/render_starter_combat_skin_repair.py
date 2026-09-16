"""Restore existing underarm topology omitted by the clothing visibility mask."""
import bpy,sys,math,json,hashlib
from pathlib import Path
from mathutils import Matrix
from mathutils.bvhtree import BVHTree
OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat'
args=sys.argv[sys.argv.index('--')+1:];sex=args[0];clip=args[1] if len(args)>1 else 'attack';aligned='aligned' in args;count=16 if clip in ['attack','unarmed-attack'] else 8
scene_name='unarmed-attack-transition.blend' if clip=='unarmed-attack' else 'club-attack-transition.blend' if clip=='attack' else f'club-{clip}.blend'
source=OUT/'contact-v4-source'/f'{sex}-{clip}'/scene_name if aligned else OUT/'transition-v2'/sex/'club-attack-transition.blend' if clip=='attack' else OUT/'blender'/f'{sex}-{clip}.blend'
dest=OUT/'contact-v4-repaired'/f'{sex}-{clip}' if aligned else OUT/'transition-v3-skin'/(sex if clip=='attack' else f'{sex}-{clip}')
for d in ['references','inspection']:(dest/d).mkdir(parents=True,exist_ok=True)
bpy.ops.wm.open_mainfile(filepath=str(source));s=bpy.context.scene;rig=bpy.data.objects[f'MH_{sex}_Rig'];body=bpy.data.objects[f'MH_{sex}_Body']
mask=next(m for m in body.modifiers if m.type=='MASK' and 'under kit' in m.name);vg=body.vertex_groups[mask.vertex_group];bg=body.vertex_groups['body'].index
shoulder=rig.data.bones['upperarm_r'].head_local
added=[]
for v in body.data.vertices:
 groups={g.group:g.weight for g in v.groups}
 # Anatomical side torso, bounded relative to the shoulder. No mesh vertex,
 # cloth coordinate, skeleton, or camera is changed.
 if groups.get(bg,0)>0 and groups.get(vg.index,0)==0 and abs(v.co.x)>abs(shoulder.x)*.53 and shoulder.z-.20<v.co.z<shoulder.z+.02:
  added.append(v.index)
vg.add(added,1,'REPLACE')
# Preserve covered front-chest masking. Derive exclusions from every real pose,
# not a hardcoded regional guess; never change the body or tunic coordinates.
attr=body.data.attributes.new('qa_original_index','INT','POINT')
for i,v in enumerate(attr.data):v.value=i
cloth=bpy.data.objects[f'{sex}_walk_baked_cloth_samples'];excluded=set();candidate=set(added)
for i in range(count):
 s.frame_set(1+i*4);dep=bpy.context.evaluated_depsgraph_get();b=body.evaluated_get(dep);c=cloth.evaluated_get(dep);ids=[v.value for v in b.data.attributes['qa_original_index'].data]
 polys=[list(p.vertices) for p in b.data.polygons if any(ids[k] in candidate for k in p.vertices)]
 bt=BVHTree.FromPolygons([b.matrix_world@v.co for v in b.data.vertices],polys);ct=BVHTree.FromPolygons([c.matrix_world@v.co for v in c.data.vertices],[list(p.vertices) for p in c.data.polygons])
 for face,_ in bt.overlap(ct):excluded.update(ids[k] for k in polys[face] if ids[k] in candidate)
if excluded:vg.remove(list(excluded))
body.data.attributes.remove(attr);added=sorted(candidate-excluded);print('RESTORED_UNDERARM_VERTICES',sex,clip,len(added),'EXCLUDED',len(excluded),flush=True)
size=s.render.resolution_x
s.render.fps=80;s.cycles.samples=24;s.render.film_transparent=True
transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']]
for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
 for o,m in transforms:o.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@m
 for i in range(count):
  s.frame_set(1+i*4);s.render.resolution_x=s.render.resolution_y=size;s.render.filepath=str(dest/'references'/f'{direction}-{i:02d}.png');bpy.ops.render.render(write_still=True)
  s.render.resolution_x=s.render.resolution_y=384;s.render.filepath=str(dest/'inspection'/f'{direction}-{i:02d}.png');bpy.ops.render.render(write_still=True)
for o,m in transforms:o.matrix_world=m
s.frame_set(1);s.render.resolution_x=s.render.resolution_y=size
bpy.ops.wm.save_as_mainfile(filepath=str(dest/scene_name),compress=True)
(dest/'repair.json').write_text(json.dumps({'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'sex':sex,'action':clip,'equipment':'unarmed' if clip=='unarmed-attack' else 'branch-club','frame_count':count,'contact_index':8 if aligned else None,'normalized_contact_phase':.5 if aligned else None,'restored_existing_vertices':added,'excluded_covered_vertices':sorted(excluded),'mask_vertex_group':vg.name,'selection':'existing body vertices hidden by clothing mask, abs(x)>0.53*shoulder lateral offset, shoulder.z-0.20 < z < shoulder.z+0.02; exclude restored vertices in faces intersecting tunic at any actual sample','geometry_and_clothing_modified':False,'frame_size':[size,size],'anchor':[64,96] if size==128 else [48,80],'pixels_per_metre':48,'status':'pending four-direction visual review'},indent=2))
