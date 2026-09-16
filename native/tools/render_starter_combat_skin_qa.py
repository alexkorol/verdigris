"""Measure new underarm face/cloth intersections over the repaired clip."""
import bpy,json,sys
from pathlib import Path
from mathutils.bvhtree import BVHTree
OUT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat/transition-v3-skin'
args=sys.argv[sys.argv.index('--')+1:];sex=args[0];clip=next((x for x in args[1:] if x in ['hit','death','unarmed-attack']),'attack');derive='derive-exclusions' in args;aligned='aligned' in args;dest=OUT.parent/'contact-v4-repaired'/f'{sex}-{clip}' if aligned else OUT/(sex if clip=='attack' else f'{sex}-{clip}');repair=json.loads((dest/'repair.json').read_text());added=set(repair['restored_existing_vertices']);count=16 if clip in ['attack','unarmed-attack'] else 8
bpy.ops.wm.open_mainfile(filepath=str(dest/('unarmed-attack-transition.blend' if clip=='unarmed-attack' else 'club-attack-transition.blend' if clip=='attack' else f'club-{clip}.blend')));s=bpy.context.scene;body=bpy.data.objects[f'MH_{sex}_Body'];cloth=bpy.data.objects[f'{sex}_walk_baked_cloth_samples']
if derive:
 rig=bpy.data.objects[f'MH_{sex}_Rig'];shoulder=rig.data.bones['upperarm_r'].head_local;vg=body.vertex_groups[repair['mask_vertex_group']];bg=body.vertex_groups['body'].index
 for v in body.data.vertices:
  groups={g.group:g.weight for g in v.groups}
  if groups.get(bg,0)>0 and groups.get(vg.index,0)==0 and abs(v.co.x)>abs(shoulder.x)*.53 and shoulder.z-.20<v.co.z<shoulder.z+.02:added.add(v.index)
 vg.add(list(added),1,'REPLACE')
attr=body.data.attributes.new('qa_original_index','INT','POINT')
for i,v in enumerate(attr.data):v.value=i
rows=[];crossing_vertices=set()
for i in range(count):
 s.frame_set(1+i*4);dep=bpy.context.evaluated_depsgraph_get();b=body.evaluated_get(dep);c=cloth.evaluated_get(dep)
 ids=[v.value for v in b.data.attributes['qa_original_index'].data]
 polys=[list(p.vertices) for p in b.data.polygons if any(ids[k] in added for k in p.vertices)]
 bt=BVHTree.FromPolygons([b.matrix_world@v.co for v in b.data.vertices],polys)
 ct=BVHTree.FromPolygons([c.matrix_world@v.co for v in c.data.vertices],[list(p.vertices) for p in c.data.polygons])
 pairs=bt.overlap(ct)
 for face,_ in pairs:crossing_vertices.update(ids[k] for k in polys[face] if ids[k] in added)
 rows.append({'phase':i,'restored_faces':len(polys),'intersecting_face_pairs':len(pairs),'restored_faces_intersecting':len(set(a for a,_ in pairs))})
(dest/('broad-restoration-intersections.json' if derive else 'skin-cloth-intersections.json')).write_text(json.dumps({'method':'BVH overlap of newly visible body faces against baked tunic at each actual animation sample. Boundary overlaps require visual inspection; not an automatic poke-through verdict.','frames':rows},indent=2));print('INTERSECTIONS',rows)
(dest/('excluded-covered-vertices.json' if derive else 'intersecting-restored-vertices.json')).write_text(json.dumps([{'index':i,'rest':list(body.data.vertices[i].co)} for i in sorted(crossing_vertices)],indent=2))
