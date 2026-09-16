"""Diagnostic: bind accepted four-view wolf paints to unchanged animated geometry.
Reuses the visibility/depth-weighted player projection method; no geometry editing.
"""
import bpy,math,json,sys,hashlib
import numpy as np
from pathlib import Path
from mathutils import Matrix,Vector
from mathutils.bvhtree import BVHTree
from bpy_extras.object_utils import world_to_camera_view
ROOT=next(p for p in Path(__file__).resolve().parents if (p/'native').is_dir())/'native/client/assets/first-slice/monsters-v2'
OUT=ROOT/'projection-proof/boss-attack'
nearest_surface=False
if nearest_surface:OUT=OUT/'v2'
for folder in ['frames','blender','reports']:(OUT/folder).mkdir(parents=True,exist_ok=True)
source=ROOT/'projection-proof/boss-appearance-atlas.png';idle=ROOT/'accepted/blender/well-alpha-idle-canvas128.blend'
dx=dy=0;records={};refine=True;sex='well-alpha';gait='attack'
report={'status':'diagnostic-not-accepted','actor':sex,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'views':[],'camera_matrices':[],'objects':{}}
for d,angle in enumerate([0,90,180,270]):
 sourceblend=idle;sourceframe=0
 bpy.ops.wm.open_mainfile(filepath=str(sourceblend));s=bpy.context.scene;rig=bpy.data.objects['WolfArmature'];action=bpy.data.actions['Idle'];rig.animation_data.action=action;rig.animation_data.action_slot=action.slots[0] if action.slots else None;s.frame_set(sourceframe);bpy.context.view_layer.update();c=s.camera;c.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@c.matrix_world;bpy.context.view_layer.update();eye=c.matrix_world.translation
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
  if o.name not in records:records[o.name]=(np.zeros((len(points),4,2),dtype=np.float32),np.zeros((len(o.data.loops),4),dtype=np.float32))
  uv,weight=records[o.name]
  for i,point in enumerate(points):
   q=world_to_camera_view(s,c,point);uv[i,d]=((d+q.x)/4,q.y)
  for poly in o.data.polygons:
   polygon=[points[i] for i in poly.vertices];point=sum(polygon,Vector())/len(polygon)
   norm=(polygon[1]-polygon[0]).cross(polygon[2]-polygon[0]).normalized();direction=(eye-point).normalized();facing=max(0,norm.dot(direction))
   if facing<.05:continue
   hit=bvh.ray_cast(eye,-direction,(eye-point).length+.02)
   if hit[0] is not None and abs(hit[3]-(eye-point).length)<.018:
    for loop in poly.loop_indices:weight[loop,d]=facing**6
  print('PROJECTED_VIEW',d,o.name,flush=True)
for name,(uv,weight) in records.items():
 uncovered=weight.sum(axis=1)<.001
 report['objects'][name]={'vertices':len(uv),'uncovered_vertices':int(uncovered.sum())}
 if nearest_surface and uncovered.any():
  coords=np.array([tuple(v.co) for v in bpy.data.objects[name].data.vertices])
  known=np.flatnonzero(~uncovered);unknown=np.flatnonzero(uncovered)
  donors=known[np.argmin(((coords[unknown,None,:]-coords[known][None,:,:])**2).sum(axis=2),axis=1)]
  uv[unknown]=uv[donors];weight[unknown]=weight[donors]
  report['objects'][name]['inferred_from_nearest_visible_surface']=int(len(unknown))
target=ROOT/'blender/well-alpha-attack-canvas128.blend'
bpy.ops.wm.open_mainfile(filepath=str(target));s=bpy.context.scene;s.frame_set(0)
report['target_sha256']=hashlib.sha256(target.read_bytes()).hexdigest()
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
geometry_before=geometry_digest()
paint=bpy.data.images.load(str(source));paint.alpha_mode='STRAIGHT'
raw=np.empty(paint.size[0]*paint.size[1]*4,dtype=np.float32);paint.pixels.foreach_get(raw);raw=raw.reshape(paint.size[1],paint.size[0],4)
palettes={};hair_tones={}
for name,(uv,weight) in records.items():
 colors=[]
 for d in range(4):
  indices=np.array([loop.vertex_index for loop in bpy.data.objects[name].data.loops]);xy=uv[indices,d];x=np.clip((xy[:,0]*paint.size[0]).astype(int),0,paint.size[0]-1);y=np.clip((xy[:,1]*paint.size[1]).astype(int),0,paint.size[1]-1);rgba=raw[y,x];valid=(weight[:,d]>.15)&(rgba[:,3]>.8)
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
 attr=o.data.color_attributes.new(name='ProjectionWeight',type='FLOAT_COLOR',domain='CORNER')
 attr.data.foreach_set('color',weight.reshape(-1))
 for idx,original in enumerate(list(o.data.materials)):
  mat=original.copy();mat.name=original.name+'_PROJECTED';o.data.materials[idx]=mat;mat.use_nodes=True;n=mat.node_tree.nodes;l=mat.node_tree.links
  out=next(v for v in n if v.type=='OUTPUT_MATERIAL');old=out.inputs['Surface'].links[0].from_socket
  if refine and name in palettes:
   fallback=n.new('ShaderNodeEmission');fallback.inputs['Color'].default_value=tuple(palettes[name])+(1,);old=fallback.outputs[0]
  a=n.new('ShaderNodeVertexColor');a.layer_name='ProjectionWeight';sep=n.new('ShaderNodeSeparateColor');l.new(a.outputs['Color'],sep.inputs['Color']);channels=[sep.outputs['Red'],sep.outputs['Green'],sep.outputs['Blue'],a.outputs['Alpha']]
  colors=[];weights=[]
  for d in range(4):
   tex=n.new('ShaderNodeTexImage');tex.image=paint;tex.interpolation='Closest';tex.extension='CLIP';coord=n.new('ShaderNodeUVMap');coord.uv_map=f'Projection{d}';l.new(coord.outputs['UV'],tex.inputs['Vector'])
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
  emission=n.new('ShaderNodeEmission');l.new(norm.outputs[0],emission.inputs['Color']);valid=n.new('ShaderNodeMath');valid.operation='GREATER_THAN';l.new(total,valid.inputs[0]);valid.inputs[1].default_value=.001
  mix=n.new('ShaderNodeMixShader');l.new(valid.outputs[0],mix.inputs[0]);l.new(old,mix.inputs[1]);l.new(emission.outputs[0],mix.inputs[2]);l.new(mix.outputs[0],out.inputs['Surface'])
# Original braid curves retain their exact geometry and head attachment. Their
# three strands use three colors sampled from this woman's own painted hair.
# This avoids projecting a hallucinated rear braid onto the garment.
report['braid_materials']={}
if sex=='female':
 tones=hair_tones.get('player-female_hair_scalp')
 if not tones:raise RuntimeError('No female painted hair samples available')
 for o in s.objects:
  if o.type!='CURVE' or 'braid' not in o.name or o.hide_render or not o.visible_get():continue
  k=int(o.name.rsplit('_',1)[-1])%3;mat=bpy.data.materials.new('Own_painted_braid_'+str(k));mat.use_nodes=True;n=mat.node_tree.nodes;n.clear();output=n.new('ShaderNodeOutputMaterial');em=n.new('ShaderNodeEmission');em.inputs['Color'].default_value=tuple(tones[k])+(1,);mat.node_tree.links.new(em.outputs[0],output.inputs[0]);o.data.materials.clear();o.data.materials.append(mat);report['braid_materials'][o.name]=tones[k]

s.render.engine='CYCLES';s.cycles.samples=24;s.cycles.use_denoising=False;s.render.threads_mode='FIXED';s.render.threads=3
s.render.film_transparent=True;s.render.image_settings.color_mode='RGBA';s.render.resolution_x=128;s.render.resolution_y=96;s.render.resolution_percentage=100
s.view_settings.view_transform='Standard';s.view_settings.exposure=0;s.view_settings.gamma=1
report['geometry_before']=geometry_before;report['geometry_after']=geometry_digest();assert report['geometry_before']==report['geometry_after']
report['frame']=[128,96];report['anchor']=[64,64];report['pixels_per_metre']=48
paint.pack();bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'blender/well-alpha-attack-projection.blend'),compress=True)
(OUT/'reports/well-alpha-attack-projection.json').write_text(json.dumps(report,indent=2))
transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']]
for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
 for o,m in transforms:o.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@m
 for phase in range(4):
  s.frame_set(phase);s.render.filepath=str(OUT/f'frames/well-alpha-attack-{direction}-{phase:02d}.png');bpy.ops.render.render(write_still=True)
print('WOLF_PROJECTION_COMPLETE',flush=True)


