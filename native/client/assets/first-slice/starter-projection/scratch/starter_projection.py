"""Bounded four-view, depth-tested paint projection on existing animated geometry.
Run Blender background --python this.py -- male [--prepare-only].
This is a diagnostic proof, not a production-approved asset pack.
"""
import bpy,math,json,sys,hashlib
import numpy as np
from pathlib import Path
from mathutils import Matrix,Vector
from mathutils.bvhtree import BVHTree
from bpy_extras.object_utils import world_to_camera_view
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice';OUT=ROOT/'starter-projection'
for d in ['frames','blender','reports']:(OUT/d).mkdir(parents=True,exist_ok=True)
sex='male';args=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []
if args and args[0] in ['male','female']:sex=args[0]
refine='--refine' in args
if refine:
 OUT=OUT/('v3' if '--semantic' in args else 'v2')
 for d in ['frames','blender','reports']:(OUT/d).mkdir(parents=True,exist_ok=True)
source=ROOT/'starter-v2/originals/player-unarmed-idle.png'
registration=json.loads((ROOT/'starter-projection/registration.json').read_text()) if refine else {'translation':[0,0]}
dx,dy=registration['translation']
idle=ROOT/f'starter-combat/blender/{sex}-idle.blend'
walk=ROOT/('starter-v2' if sex=='male' else 'starter-v2-female-refine')/f'blender/{sex}-walk-exterior.blend'
bpy.ops.wm.open_mainfile(filepath=str(idle));s=bpy.context.scene;s.frame_set(1);bpy.context.view_layer.update()
c=s.camera;basecam=c.matrix_world.copy()
# Dependency graph visibility respects excluded collections. BVH includes all
# rendered surfaces, including other body parts, clothes, hair and shoe curves.
deps=bpy.context.evaluated_depsgraph_get();vv=[];ff=[];active=[]
for o in s.objects:
 if o.type not in ['MESH','CURVE'] or o.hide_render or not o.visible_get():continue
 active.append(o)
 ev=o.evaluated_get(deps);me=ev.to_mesh();off=len(vv)
 vv.extend(o.matrix_world@v.co for v in me.vertices);ff.extend(tuple(off+i for i in p.vertices) for p in me.polygons);ev.to_mesh_clear()
bvh=BVHTree.FromPolygons(vv,ff,all_triangles=False)
records={};report={'status':'diagnostic-only','sex':sex,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'idle_sha256':hashlib.sha256(idle.read_bytes()).hexdigest(),'motion_sha256':hashlib.sha256(walk.read_bytes()).hexdigest(),'method':'Four exact idle camera projections. Per-surface depth test and angle weighting. Static UV coordinates deform with original mesh. Geometry determines alpha. Emission avoids double lighting. No silhouette warp or cutout.','camera_matrices':[],'objects':{}}
cameras=[]
for angle in [0,90,180,270]:
 c.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@basecam;cameras.append(c.matrix_world.copy());report['camera_matrices'].append([list(v) for v in c.matrix_world])
for o in active:
 if o.type!='MESH':continue
 # Expose original-index vertices while retaining all deformation. Restoring
 # topology modifiers after the evaluation preserves the unchanged render mesh.
 toggled=[]
 for m in o.modifiers:
  if m.type in ['MASK','SUBSURF','SOLIDIFY']:
   toggled.append((m,m.show_viewport));m.show_viewport=False
 bpy.context.view_layer.update();ev=o.evaluated_get(bpy.context.evaluated_depsgraph_get());me=ev.to_mesh()
 if len(me.vertices)!=len(o.data.vertices):raise RuntimeError('Topology mapping mismatch: '+o.name)
 points=[o.matrix_world@v.co for v in me.vertices];normals=[(o.matrix_world.to_3x3().inverted().transposed()@v.normal).normalized() for v in me.vertices]
 ev.to_mesh_clear()
 for m,value in toggled:m.show_viewport=value
 bpy.context.view_layer.update()
 uv=np.zeros((len(points),4,2),dtype=np.float32);weight=np.zeros((len(points),4),dtype=np.float32)
 for d,mat in enumerate(cameras):
  c.matrix_world=mat;bpy.context.view_layer.update();eye=mat.translation
  for i,(point,norm) in enumerate(zip(points,normals)):
   q=world_to_camera_view(s,c,point);uv[i,d]=((d*96+q.x*96-dx)/384,(144+96*q.y+dy)/256-(0 if sex=='male' else .5))
   direction=(eye-point).normalized();facing=max(0,norm.dot(direction))
   if facing<.05 or not (0<=q.x<=1 and 0<=q.y<=1):continue
   hit=bvh.ray_cast(eye,-direction,(eye-point).length+.02)
   if hit[0] is not None and abs(hit[3]-(eye-point).length)<.018:weight[i,d]=facing**6
 records[o.name]=(uv,weight)
 report['objects'][o.name]={'vertices':len(points),'uncovered_vertices':int((weight.sum(axis=1)<.001).sum())}
 print('PROJECTED',o.name,report['objects'][o.name],flush=True)
if refine:
 s.render.resolution_x=s.render.resolution_y=96;s.render.resolution_percentage=100
 report['registration']=registration
 report['idle_pose_verification']={'file':str(idle),'frame':s.frame_current,'checks':[]}
 for d,mat in enumerate(cameras):
  c.matrix_world=mat;s.render.filepath=str(OUT/f'frames/source-idle-{d}.png');bpy.ops.render.render(write_still=True)
  report['idle_pose_verification']['checks'].append({'render':f'frames/source-idle-{d}.png','source_reference':f'starter-combat/references/{sex}-idle-'+['front','right','back','left'][d]+'-00.png'})
bpy.ops.wm.open_mainfile(filepath=str(walk));s=bpy.context.scene;s.frame_set(72)
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
palettes={}
for name,(uv,weight) in records.items():
 colors=[]
 for d in range(4):
  xy=uv[:,d];x=np.clip((xy[:,0]*paint.size[0]).astype(int),0,paint.size[0]-1);y=np.clip((xy[:,1]*paint.size[1]).astype(int),0,paint.size[1]-1);rgba=raw[y,x];valid=(weight[:,d]>.15)&(rgba[:,3]>.8)
  colors.extend(rgba[valid,:3])
 if len(colors):
  rgb=np.median(colors,axis=0);linear=np.where(rgb<=.04045,rgb/12.92,((rgb+.055)/1.055)**2.4);palettes[name]=linear.tolist()
report['paint_sampled_fallback_linear_rgb']=palettes
# Material multiplication is done in linear space. UV image data is sRGB;
# Blender converts it once. Keep Standard view and unit exposure.
for name,(uv,weight) in records.items():
 o=bpy.data.objects.get(name)
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
  colors=[];weights=[]
  for d in range(4):
   tex=n.new('ShaderNodeTexImage');tex.image=paint;tex.interpolation='Closest';tex.extension='CLIP';coord=n.new('ShaderNodeUVMap');coord.uv_map=f'Projection{d}';l.new(coord.outputs['UV'],tex.inputs['Vector'])
   w=n.new('ShaderNodeMath');w.operation='MULTIPLY';l.new(channels[d],w.inputs[0]);l.new(tex.outputs['Alpha'],w.inputs[1])
   # Keep rope/skin-colored generated pixels off the fabric surface. Confidence
   # is derived from the source-painted cloth/rope chromaticity, not invented RGB.
   if '--semantic' in args and 'baked_cloth_samples' in name:
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
s.render.engine='CYCLES';s.cycles.samples=24;s.cycles.use_denoising=False;s.render.film_transparent=True;s.render.image_settings.color_mode='RGBA';s.render.resolution_x=s.render.resolution_y=96;s.render.resolution_percentage=100
s.view_settings.view_transform='Standard';s.view_settings.exposure=0;s.view_settings.gamma=1
report['geometry_before']=geometry_before;report['geometry_after']=geometry_digest();assert report['geometry_before']==report['geometry_after']
report['camera']={'type':s.camera.data.type,'lens':s.camera.data.lens,'shift_x':s.camera.data.shift_x,'shift_y':s.camera.data.shift_y,'sensor_width':s.camera.data.sensor_width,'frame':[96,96],'anchor':[48,80],'pixels_per_metre':48}
paint.pack();bpy.ops.wm.save_as_mainfile(filepath=str(OUT/f'blender/{sex}-walk-projection.blend'),compress=True)
(OUT/f'reports/{sex}-projection.json').write_text(json.dumps(report,indent=2))
if '--prepare-only' in args:sys.exit(0)
transforms=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ['CAMERA','LIGHT']]
for direction,angle in [('front',0),('right',90),('back',180),('left',270)]:
 for o,m in transforms:o.matrix_world=Matrix.Rotation(math.radians(-angle),4,'Z')@m
 for phase,frame in enumerate(range(72,101,4)):
  s.frame_set(frame);p=world_to_camera_view(s,s.camera,Vector());assert abs(p.x*96-48)<.01 and abs((1-p.y)*96-80)<.01
  s.render.filepath=str(OUT/f'frames/{sex}-walk-{direction}-{phase:02d}.png');bpy.ops.render.render(write_still=True)
print('PROJECTION_COMPLETE',sex,flush=True)
