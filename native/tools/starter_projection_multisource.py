"""Bounded four-view, depth-tested paint projection on existing animated geometry.
Run Blender background --python this.py -- male [--prepare-only].
Outputs are candidates requiring independent native-scale review.
"""
import bpy,math,json,sys,hashlib
import numpy as np
from pathlib import Path
from mathutils import Matrix,Vector
from mathutils.bvhtree import BVHTree
from bpy_extras.object_utils import world_to_camera_view
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice'
args=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []
sex=args[0] if args and args[0] in ['male','female'] else 'male'
def option(name,default=None):return args[args.index(name)+1] if name in args else default
refine=True
idle=ROOT/f'starter-combat/blender/{sex}-idle.blend'
walk=ROOT/('starter-v2' if sex=='male' else 'starter-v2-female-refine')/f'blender/{sex}-walk-exterior.blend'
gait=option('--action','idle' if '--idle' in args else 'sprint' if '--sprint' in args else 'walk')
cohort=option('--cohort','v5' if sex=='male' else 'female-v2')
assert '/' not in cohort and '\\' not in cohort and cohort not in ['.','..']
OUT=ROOT/'starter-projection'/cohort
for d in ['frames','blender','reports']:(OUT/d).mkdir(parents=True,exist_ok=True)
source=ROOT/('starter-projection/v4/appearance-atlas.png' if sex=='male' else 'starter-projection/female-source-prep/appearance-atlas.png')
dx=dy=0;records={}
report={'status':'candidate-needs-review','sex':sex,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'views':[],'method':'Exact source-pose multi-view texture projection. Front/back accepted walk72 paint; sides registered idle1 paint. Original geometry, depth visibility, normal weights and UV transfer by unchanged vertex index.','camera_matrices':[],'objects':{}}
if sex=='female':report['method']='Four views from exact refined female walk frame72. Her accepted front paint and registered side/back paint; depth visibility, normal weights and UV transfer by unchanged mesh vertex index. Separately recorded intentional braid curve-width revision.'
for d,angle in enumerate([0,90,180,270]):
 sourceblend=walk if sex=='female' or d in [0,2] else idle;sourceframe=72 if sex=='female' or d in [0,2] else 1
 bpy.ops.wm.open_mainfile(filepath=str(sourceblend));s=bpy.context.scene;s.frame_set(sourceframe);bpy.context.view_layer.update();c=s.camera;c.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@c.matrix_world;bpy.context.view_layer.update();eye=c.matrix_world.translation
 report['camera_matrices'].append([list(v) for v in c.matrix_world]);report['views'].append({'direction':['front','right','back','left'][d],'geometry_file':str(sourceblend),'geometry_sha256':hashlib.sha256(sourceblend.read_bytes()).hexdigest(),'frame':sourceframe})
 deps=bpy.context.evaluated_depsgraph_get();vv=[];ff=[];active=[]
 for o in s.objects:
  if o.type not in ['MESH','CURVE'] or o.hide_render or not o.visible_get():continue
  active.append(o);ev=o.evaluated_get(deps);me=ev.to_mesh();off=len(vv);vv.extend(o.matrix_world@v.co for v in me.vertices);ff.extend(tuple(off+i for i in p.vertices) for p in me.polygons);ev.to_mesh_clear()
 bvh=BVHTree.FromPolygons(vv,ff,all_triangles=False)
 for o in active:
  if o.type!='MESH':continue
  toggled=[]
  for m in o.modifiers:
   if m.type in ['MASK','SUBSURF','SOLIDIFY']:toggled.append((m,m.show_viewport));m.show_viewport=False
  bpy.context.view_layer.update();ev=o.evaluated_get(bpy.context.evaluated_depsgraph_get());me=ev.to_mesh()
  if len(me.vertices)!=len(o.data.vertices):raise RuntimeError('Topology mapping mismatch: '+o.name)
  points=[o.matrix_world@v.co for v in me.vertices];normals=[(o.matrix_world.to_3x3().inverted().transposed()@v.normal).normalized() for v in me.vertices];ev.to_mesh_clear()
  for m,value in toggled:m.show_viewport=value
  bpy.context.view_layer.update()
  if o.name not in records:records[o.name]=(np.zeros((len(points),4,2),dtype=np.float32),np.zeros((len(points),4),dtype=np.float32))
  uv,weight=records[o.name]
  for i,(point,norm) in enumerate(zip(points,normals)):
   q=world_to_camera_view(s,c,point);uv[i,d]=((d+q.x)/4,(144+96*q.y)/256)
   direction=(eye-point).normalized();facing=max(0,norm.dot(direction))
   if facing<.05 or not (0<=q.x<=1 and 0<=q.y<=1):continue
   hit=bvh.ray_cast(eye,-direction,(eye-point).length+.02)
   if hit[0] is not None and abs(hit[3]-(eye-point).length)<.018:weight[i,d]=facing**6
  print('PROJECTED_VIEW',d,o.name,flush=True)
for name,(uv,weight) in records.items():report['objects'][name]={'vertices':len(uv),'uncovered_vertices':int((weight.sum(axis=1)<.001).sum())}
target=Path(option('--target')) if '--target' in args else idle if gait=='idle' else walk if gait=='walk' else walk.with_name(f'{sex}-sprint-exterior.blend')
first=int(option('--start',72 if 'walk' in gait or 'sprint' in gait else 1));count=int(option('--count',4 if 'idle' in gait else 8));frame_numbers=list(range(first,first+count*4,4))
bpy.ops.wm.open_mainfile(filepath=str(target));s=bpy.context.scene;s.frame_set(first);size=s.render.resolution_x;anchor=[48,80] if size==96 else [64,96]
if size not in [96,128]:raise ValueError('Unexpected source camera frame size: '+str(size))
held_club=bpy.data.objects.get(f'{sex}_starter_branch_club')
if '--unarmed' in args and held_club:held_club.hide_render=True
report['equipment']='branch-club' if held_club and not held_club.hide_render else 'unarmed'
report['target_sha256']=hashlib.sha256(target.read_bytes()).hexdigest();report['target_file']=str(target);report['gait']=gait;report['source_frames']=frame_numbers;report['cohort']=cohort
report['projection_script_sha256']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
if 'attack' in gait:
 assert count==16, 'Grounded attacks require 16 phases'
 report['attack_timing']={'contact_index':8,'contact_normalized_phase':0.5,'preparation_indices':list(range(8)),'recovery_indices':list(range(9,16)),'source_contact_frame':frame_numbers[8]}
 timing_path=Path(option('--timing',str(target.parent/'manifest.json')));timing=json.loads(timing_path.read_text());timing_clips=timing['clips']
 assert all(c['contact_index']==8 and len(c['frames'])==16 for c in timing_clips)
 report['attack_source_manifest']={'path':str(timing_path),'sha256':hashlib.sha256(timing_path.read_bytes()).hexdigest(),'motion':timing['source_motion'],'phase_mapping':[{'index':f['index'],'role':f['role'],'source_pose':f['source_pose']} for f in timing_clips[0]['frames']]}
def geometry_digest():
 h=hashlib.sha256()
 for obj in sorted(s.objects,key=lambda x:x.name):
  if obj.type!='MESH':continue
  h.update(obj.name.encode());h.update(str(list(obj.matrix_world)).encode())
  coords=np.empty(len(obj.data.vertices)*3,dtype=np.float32);obj.data.vertices.foreach_get('co',coords);h.update(coords.tobytes())
  if obj.data.shape_keys:
   for key in obj.data.shape_keys.key_blocks:
    key.data.foreach_get('co',coords);h.update(coords.tobytes())
 return h.hexdigest()
def braid_digest():
 h=hashlib.sha256();records=[]
 for o in sorted(s.objects,key=lambda x:x.name):
  if o.type!='CURVE' or 'braid' not in o.name or not o.visible_get():continue
  record={'name':o.name,'bevel_depth':o.data.bevel_depth,'matrix_world':[list(v) for v in o.matrix_world],'splines':[{'points':[list(p.co) for p in sp.points],'radii':[p.radius for p in sp.points]} for sp in o.data.splines]};records.append(record)
 h.update(json.dumps(records,sort_keys=True).encode());return h.hexdigest(),records
geometry_before=geometry_digest();braid_before,braid_before_data=braid_digest()
paint=bpy.data.images.load(str(source));paint.alpha_mode='STRAIGHT'
raw=np.empty(paint.size[0]*paint.size[1]*4,dtype=np.float32);paint.pixels.foreach_get(raw);raw=raw.reshape(paint.size[1],paint.size[0],4)
palettes={};hair_tones={}
for name,(uv,weight) in records.items():
 colors=[]
 for d in range(4):
  xy=uv[:,d];x=np.clip((xy[:,0]*paint.size[0]).astype(int),0,paint.size[0]-1);y=np.clip((xy[:,1]*paint.size[1]).astype(int),0,paint.size[1]-1);rgba=raw[y,x];valid=(weight[:,d]>.15)&(rgba[:,3]>.8)
  colors.extend(rgba[valid,:3])
 if len(colors):
  rgb=np.median(colors,axis=0);linear=np.where(rgb<=.04045,rgb/12.92,((rgb+.055)/1.055)**2.4);palettes[name]=linear.tolist()
  if 'hair_scalp' in name:
   levels=np.quantile(colors,[.30,.50,.70],axis=0);levels=np.where(levels<=.04045,levels/12.92,((levels+.055)/1.055)**2.4);hair_tones[name]=levels.tolist()
report['paint_sampled_fallback_linear_rgb']=palettes
# Material multiplication is done in linear space. UV image data is sRGB;
# Blender converts it once. Keep Standard view and unit exposure.
for name,(uv,weight) in records.items():
 targetname=name.replace('_walk_','_sprint_') if 'sprint' in gait else name
 o=bpy.data.objects.get(targetname)
 if not o or o.type!='MESH' or len(o.data.vertices)!=len(uv):raise RuntimeError('Target correspondence failed: '+name)
 for d in range(4):
  lay=o.data.uv_layers.new(name=f'Projection{d}')
  for loop in o.data.loops:lay.data[loop.index].uv=uv[loop.vertex_index,d]
 attr=o.data.color_attributes.new(name='ProjectionWeight',type='FLOAT_COLOR',domain='POINT')
 attr.data.foreach_set('color',weight.reshape(-1))
 for idx,original in enumerate(list(o.data.materials)):
  mat=original.copy();mat.name=original.name+'_PROJECTED';o.data.materials[idx]=mat;mat.use_nodes=True;n=mat.node_tree.nodes;l=mat.node_tree.links
  out=next(v for v in n if v.type=='OUTPUT_MATERIAL');old=out.inputs['Surface'].links[0].from_socket
  if refine and name in palettes:
   fallback=n.new('ShaderNodeEmission');fallback.inputs['Color'].default_value=tuple(palettes[name])+(1,);old=fallback.outputs[0]
  a=n.new('ShaderNodeVertexColor');a.layer_name='ProjectionWeight';sep=n.new('ShaderNodeSeparateColor');l.new(a.outputs['Color'],sep.inputs['Color']);channels=[sep.outputs['Red'],sep.outputs['Green'],sep.outputs['Blue'],a.outputs['Alpha']]
  colors=[];weights=[];textures=[]
  for d in range(4):
   tex=n.new('ShaderNodeTexImage');tex.image=paint;textures.append(tex.outputs['Color']);tex.interpolation='Closest';tex.extension='CLIP';coord=n.new('ShaderNodeUVMap');coord.uv_map=f'Projection{d}';l.new(coord.outputs['UV'],tex.inputs['Vector'])
   w=n.new('ShaderNodeMath');w.operation='MULTIPLY';l.new(channels[d],w.inputs[0]);l.new(tex.outputs['Alpha'],w.inputs[1])
   # Keep rope/skin-colored generated pixels off the fabric surface. Confidence
   # is derived from the source-painted cloth/rope chromaticity, not invented RGB.
   if 'baked_cloth_samples' in name:
    cloth=palettes[name];cord=palettes.get(f'player-{sex}_waist_cord_cloth_bound',cloth);edge=(cloth[2]/max(cloth[0],1e-6)+cord[2]/max(cord[0],1e-6))*.5
    rgb=n.new('ShaderNodeSeparateColor');l.new(tex.outputs['Color'],rgb.inputs[0]);ratio=n.new('ShaderNodeMath');ratio.operation='DIVIDE';l.new(rgb.outputs['Blue'],ratio.inputs[0]);l.new(rgb.outputs['Red'],ratio.inputs[1]);confidence=n.new('ShaderNodeMapRange');confidence.clamp=True;l.new(ratio.outputs[0],confidence.inputs['Value']);confidence.inputs['From Min'].default_value=edge-.04;confidence.inputs['From Max'].default_value=edge+.04
    safe=n.new('ShaderNodeMath');safe.operation='MULTIPLY';l.new(w.outputs[0],safe.inputs[0]);l.new(confidence.outputs[0],safe.inputs[1]);w=safe
   weights.append(w.outputs[0])
   mul=n.new('ShaderNodeVectorMath');mul.operation='SCALE';l.new(tex.outputs['Color'],mul.inputs[0]);l.new(w.outputs[0],mul.inputs['Scale']);colors.append(mul.outputs[0])
  def summation(sockets,vector):
   result=sockets[0]
   for v in sockets[1:]:
    add=n.new('ShaderNodeVectorMath' if vector else 'ShaderNodeMath');add.operation='ADD';l.new(result,add.inputs[0]);l.new(v,add.inputs[1]);result=add.outputs[0]
   return result
  total=summation(weights,False);color=summation(colors,True);inv=n.new('ShaderNodeMath');inv.operation='DIVIDE';inv.inputs[0].default_value=1;l.new(total,inv.inputs[1]);norm=n.new('ShaderNodeVectorMath');norm.operation='SCALE';l.new(color,norm.inputs[0]);l.new(inv.outputs[0],norm.inputs['Scale'])
  # Avoid blending different generated eyes/nose or fabric marks into a washed
  # average. Select the strongest visible source at face/cloth, while retaining
  # smooth multi-view mixing elsewhere on the body.
  final_color=norm.outputs[0]
  if (sex=='female' or '--strongest' in args) and (name==f'MH_{sex}_Body' or 'baked_cloth_samples' in name):
   best=weights[0];chosen=textures[0]
   for d in range(1,4):
    larger=n.new('ShaderNodeMath');larger.operation='GREATER_THAN';l.new(weights[d],larger.inputs[0]);l.new(best,larger.inputs[1]);choose=n.new('ShaderNodeMixRGB');l.new(larger.outputs[0],choose.inputs[0]);l.new(chosen,choose.inputs[1]);l.new(textures[d],choose.inputs[2]);chosen=choose.outputs[0];maximum=n.new('ShaderNodeMath');maximum.operation='MAXIMUM';l.new(best,maximum.inputs[0]);l.new(weights[d],maximum.inputs[1]);best=maximum.outputs[0]
   if 'baked_cloth_samples' in name:final_color=chosen
   else:
    region=o.data.attributes.get('ProjectionFace') or o.data.attributes.new(name='ProjectionFace',type='FLOAT',domain='POINT')
    for v in o.data.vertices:region.data[v.index].value=1 if v.co.z>1.47 else 0
    attribute=n.new('ShaderNodeAttribute');attribute.attribute_name='ProjectionFace';blend=n.new('ShaderNodeMixRGB');l.new(attribute.outputs['Fac'],blend.inputs[0]);l.new(norm.outputs[0],blend.inputs[1]);l.new(chosen,blend.inputs[2]);final_color=blend.outputs[0]
  emission=n.new('ShaderNodeEmission');l.new(final_color,emission.inputs['Color']);valid=n.new('ShaderNodeMath');valid.operation='GREATER_THAN';l.new(total,valid.inputs[0]);valid.inputs[1].default_value=.001
  mix=n.new('ShaderNodeMixShader');l.new(valid.outputs[0],mix.inputs[0]);l.new(old,mix.inputs[1]);l.new(emission.outputs[0],mix.inputs[2]);l.new(mix.outputs[0],out.inputs['Surface'])
# The unarmed body paintings contain no weapon pixels. Bind the existing
# right-hand club independently to a hand-excluded ImageGen wooden-branch crop.
club=bpy.data.objects.get(f'{sex}_starter_branch_club')
if club and not club.hide_render:
 woodroot=ROOT/'starter-projection/weapon-source';woodmeta=json.loads((woodroot/'provenance.json').read_text());wood=bpy.data.images.load(str(woodroot/'painted-branch-patch.png'));wood.alpha_mode='STRAIGHT';wood.pack()
 if len(club.data.vertices)!=63:raise RuntimeError('Unexpected branch topology for reviewed seven-ring UV mapping')
 layer=club.data.uv_layers.new(name='PaintedWood')
 for loop in club.data.loops:
  ring=loop.vertex_index//9;around=loop.vertex_index%9;layer.data[loop.index].uv=(ring/6,around/8)
 mat=bpy.data.materials.new('ImageGen_painted_branch_wood');mat.use_nodes=True;n=mat.node_tree.nodes;l=mat.node_tree.links;n.clear();output=n.new('ShaderNodeOutputMaterial');em=n.new('ShaderNodeEmission');tex=n.new('ShaderNodeTexImage');tex.image=wood;tex.interpolation='Closest';coord=n.new('ShaderNodeUVMap');coord.uv_map='PaintedWood';l.new(coord.outputs[0],tex.inputs[0]);mix=n.new('ShaderNodeMixRGB');mix.blend_type='MIX';mix.inputs[1].default_value=tuple(woodmeta['fallback_linear_rgb'])+(1,);l.new(tex.outputs['Alpha'],mix.inputs[0]);l.new(tex.outputs['Color'],mix.inputs[2]);l.new(mix.outputs[0],em.inputs[0]);l.new(em.outputs[0],output.inputs[0]);club.data.materials.clear();club.data.materials.append(mat);report['weapon_appearance']=woodmeta;assert woodmeta['patch_sha256']==hashlib.sha256((woodroot/woodmeta['patch']).read_bytes()).hexdigest()
# Original braid curves retain their exact geometry and head attachment. Their
# three strands use three colors sampled from this woman's own painted hair.
# This avoids projecting a hallucinated rear braid onto the garment.
report['braid_materials']={}
if sex=='female':
 braidmeta=json.loads((ROOT/'starter-projection/female-source-prep/braid-tones.json').read_text());tones=braidmeta['linear_rgb'];report['braid_tone_source']=braidmeta
 if not tones:raise RuntimeError('No female painted hair samples available')
 for o in s.objects:
  if o.type!='CURVE' or 'braid' not in o.name or o.hide_render or not o.visible_get():continue
  k=int(o.name.rsplit('_',1)[-1])%3;mat=bpy.data.materials.new('Own_painted_braid_'+str(k));mat.use_nodes=True;n=mat.node_tree.nodes;n.clear();output=n.new('ShaderNodeOutputMaterial');em=n.new('ShaderNodeEmission');em.inputs['Color'].default_value=tuple(tones[k])+(1,);mat.node_tree.links.new(em.outputs[0],output.inputs[0]);o.data.materials.clear();o.data.materials.append(mat);report['braid_materials'][o.name]=tones[k]
if sex=='female':
 strands=sorted([o for o in s.objects if o.type=='CURVE' and 'natural_long_braid_' in o.name and o.visible_get()],key=lambda o:o.name)
 assert len(strands)==3
 point_count=len(strands[0].data.splines[0].points)
 for i in range(point_count):
  points=[o.data.splines[0].points[i] for o in strands];center=sum((p.co.xyz for p in points),Vector())/3;t=i/(point_count-1)
  # Keep the exact centerline, enlarge the upper/middle woven cross-section,
  # and retain a fine tip. The same rest-curve profile is used in every action.
  profile=1.55 if t<.7 else 1.55-(t-.7)/.3*.55
  for point in points:
   delta=point.co.xyz-center;co=center+delta*profile;point.co=(co.x,co.y,co.z,point.co.w);point.radius*=profile
 braid_after,braid_after_data=braid_digest();report['intentional_braid_geometry_change']={'before_sha256':braid_before,'after_sha256':braid_after,'before':braid_before_data,'after':braid_after_data,'profile':'1.55x radial cross-section upper70%, linearly tapering to1.0x at tip. Centerline unchanged. No 2D dilation.','authorization':'Owner requested2-3nativepixel upper/middle braid and taperedtip.'}
if sex=='female':
 local_profile=[{key:value for key,value in record.items() if key!='matrix_world'} for record in braid_after_data];report['intentional_braid_geometry_change']['local_profile_sha256']=hashlib.sha256(json.dumps(local_profile,sort_keys=True).encode()).hexdigest()
s.render.engine='CYCLES';s.cycles.samples=24;s.cycles.use_denoising=False;s.render.film_transparent=True;s.render.image_settings.color_mode='RGBA';s.render.resolution_x=s.render.resolution_y=size;s.render.resolution_percentage=100
s.view_settings.view_transform='Standard';s.view_settings.exposure=0;s.view_settings.gamma=1
report['geometry_before']=geometry_before;report['geometry_after']=geometry_digest();assert report['geometry_before']==report['geometry_after']
report['camera']={'type':s.camera.data.type,'lens':s.camera.data.lens,'shift_x':s.camera.data.shift_x,'shift_y':s.camera.data.shift_y,'sensor_width':s.camera.data.sensor_width,'frame':[size,size],'anchor':anchor,'pixels_per_metre':48}
paint.pack();bpy.ops.wm.save_as_mainfile(filepath=str(OUT/f'blender/{sex}-{gait}-projection.blend'),compress=True)
(OUT/f'reports/{sex}-{gait}-projection.json').write_text(json.dumps(report,indent=2))
if '--prepare-only' in args:sys.exit(0)
transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']]
for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
 for o,m in transforms:o.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@m
 for phase,frame in enumerate(frame_numbers):
  s.frame_set(frame);p=world_to_camera_view(s,s.camera,Vector());assert abs(p.x*size-anchor[0])<.01 and abs((1-p.y)*size-anchor[1])<.01
  s.render.filepath=str(OUT/f'frames/{sex}-{gait}-{direction}-{phase:02d}.png');bpy.ops.render.render(write_still=True)
print('PROJECTION_COMPLETE',sex,flush=True)
report['render_complete']=True
(OUT/f'reports/{sex}-{gait}-projection.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
