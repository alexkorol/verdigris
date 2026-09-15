"""Editable scenery guides, fixed 48 screen pixels/metre orthographic camera."""
import bpy, math, random, json
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/world-scenery'
for p in ('blender','references','guides','review','painted'): (ROOT/p).mkdir(parents=True,exist_ok=True)
random.seed(73)
bpy.ops.wm.read_factory_settings(use_empty=True)
s=bpy.context.scene
s.render.engine='CYCLES'; s.cycles.samples=32; s.cycles.use_denoising=False
s.render.resolution_x=s.render.resolution_y=256; s.render.resolution_percentage=100
s.render.image_settings.color_mode='RGBA'; s.render.film_transparent=True
s.world=bpy.data.worlds.new('sky'); s.world.use_nodes=True
s.world.node_tree.nodes['Background'].inputs[0].default_value=(.65,.7,.78,1)
s.world.node_tree.nodes['Background'].inputs[1].default_value=.4
def mat(n,c):
 m=bpy.data.materials.new(n); m.diffuse_color=(*c,1); m.use_nodes=True
 m.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value=(*c,1)
 m.node_tree.nodes['Principled BSDF'].inputs['Roughness'].default_value=.95
 return m
wood=mat('aged oak',(.21,.12,.055)); tip=mat('split heartwood',(.39,.26,.13)); stone=mat('weathered fieldstone',(.27,.3,.28)); moss=mat('moss',(.11,.16,.05)); dark=mat('well darkness',(.015,.022,.021))
leaf=mat('olive foliage',(.12,.19,.065))
def mesh(n,vs,fs,m):
 me=bpy.data.meshes.new(n); me.from_pydata(vs,[],fs); me.update(); o=bpy.data.objects.new(n,me); s.collection.objects.link(o); o.data.materials.append(m); return o
def cube(n,loc,dim,m):
 bpy.ops.mesh.primitive_cube_add(size=1,location=loc); o=bpy.context.object; o.name=n; o.dimensions=dim; bpy.ops.object.transform_apply(location=False,rotation=False,scale=True); o.data.materials.append(m)
 b=o.modifiers.new('worn edges','BEVEL'); b.width=.025; b.segments=1
 return o
def beam(n,a,b,r,m,tiprad=None):
 a,b=Vector(a),Vector(b); v=b-a
 bpy.ops.mesh.primitive_cone_add(vertices=8,radius1=r,radius2=r if tiprad is None else tiprad,depth=v.length,location=(a+b)/2)
 o=bpy.context.object;o.name=n;o.rotation_euler=v.to_track_quat('Z','Y').to_euler();o.data.materials.append(m);return o
cam=bpy.data.cameras.new('fixed48');c=bpy.data.objects.new('fixed48',cam);s.collection.objects.link(c);s.camera=c
c.location=(0,-10,7.8); target=Vector((0,0,1)); c.rotation_euler=(target-c.location).to_track_quat('-Z','Y').to_euler();cam.type='ORTHO';cam.ortho_scale=256/48
# A fixed ground-origin anchor: move camera along local up so origin appears at (128,200).
c.location+=c.rotation_euler.to_matrix()@Vector((0,(200-128)/48-1*math.cos(math.atan2(6.8,10)),0))
for n,loc,e in [('key',(-3,-4,7),700),('fill',(4,-2,5),180)]:
 d=bpy.data.lights.new(n,'AREA');d.energy=e;d.shape='DISK';d.size=4;o=bpy.data.objects.new(n,d);s.collection.objects.link(o);o.location=loc;o.rotation_euler=(-o.location).to_track_quat('-Z','Y').to_euler()
records=[]
def render(n,footprint):
 from bpy_extras.object_utils import world_to_camera_view
 s.render.filepath=str(ROOT/'references'/f'{n}.png');bpy.ops.render.render(write_still=True)
 bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/'blender'/f'{n}.blend'),compress=True)
 p=world_to_camera_view(s,c,Vector((0,0,0)))
 records.append({'id':n,'frame':[256,256],'anchor':[round(p.x*256),round((1-p.y)*256)],'pixels_per_metre':48,'footprint_metres':footprint,'projection':'orthographic','camera_elevation_degrees':34.2157})
 # Four diagnostic angles saved at native scale, with identical geometry.
 saved=c.matrix_world.copy()
 for i,(az,el) in enumerate([(315,55),(0,12),(225,35),(0,89)]):
  az,el=map(math.radians,(az,el));c.location=Vector((10*math.cos(el)*math.cos(az),10*math.cos(el)*math.sin(az),10*math.sin(el)))+Vector((0,0,.8));c.rotation_euler=(Vector((0,0,.8))-c.location).to_track_quat('-Z','Y').to_euler();s.render.filepath=str(ROOT/'review'/f'{n}-angle{i}.png');bpy.ops.render.render(write_still=True)
 c.matrix_world=saved
 for o in list(s.objects):
  if o.type=='MESH':bpy.data.objects.remove(o,do_unlink=True)
# Square well: four walls built from interlocking stone courses, hollow interior.
for z in range(3):
 for x in (-.63,.63):
  for y in (-.63,.63):cube('solid cornerstone',(x,y,.14+z*.27),(.29,.29,.26),stone)
 for side in range(4):
  for j in range(3):
   t=(j-1)*.32; pos=(t,-.63) if side==0 else ((.63,t) if side==1 else ((-t,.63) if side==2 else (-.63,-t)))
   o=cube('well masonry',(*pos,.14+z*.27),(.32,.28,.26) if side%2==0 else (.28,.32,.26),stone)
cube('dark water',(0,0,.09),(1.1,1.1,.02),dark)
for x in (-.93,.93):beam('well upright',(x,0,0),(x,0,2),.085,wood)
beam('well crossbar',(-1.04,0,1.92),(1.04,0,1.92),.08,wood)
beam('rope',(0,0,1.91),(0,0,.44),.015,tip)
render('square-well',[1.9,1.5])
random.seed(73)
for _ in range(48):random.uniform(-.025,.025)
for broken in (False,True):
 for i in range(13):
  x=(i-6)*.205;h=2+random.uniform(-.12,.12)
  if broken and 4<=i<=8:h=random.uniform(.2,.65)
  beam('palisade timber',(x,0,0),(x,0,h-.18),.12,wood)
  beam('split pointed top',(x,0,h-.18),(x,0,h),.12,tip,0)
 for z in (.5,1.25):
  if broken:
   beam('broken rail',(-1.36,.1,z),(-.56,.1,z),.06,wood);beam('broken rail',(.56,.1,z),(1.36,.1,z),.06,wood)
  else:beam('rear rail',(-1.36,.1,z),(1.36,.1,z),.06,wood)
 render('palisade-breached' if broken else 'palisade-intact',[2.8,.35])
for i in range(5):
 x,y=random.uniform(-.55,.55),random.uniform(-.35,.35);r=random.uniform(.25,.5)
 bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1,radius=r,location=(x,y,r*.37));o=bpy.context.object;o.scale=(1.2,.9,.72);o.data.materials.append(stone)
render('fieldstone-cluster',[1.6,1.0])
for i in range(28):
 a=random.uniform(0,math.tau);r=random.uniform(.1,.6);top=(math.cos(a)*r,math.sin(a)*r,random.uniform(.25,.75));beam('shrub twig',(0,0,0),top,.012,wood)
 for j in range(3):
  center=Vector(top)*(.55+j*.2); aa=a+j*1.4; d=Vector((math.cos(aa)*.16,math.sin(aa)*.16,.07));w=Vector((-math.sin(aa)*.07,math.cos(aa)*.07,0));mesh('leaf',[center-d,center+w,center+d,center-w],[(0,1,2,3)],leaf)
render('woodland-shrub',[1.5,1.5])
earth=mat('dry worn earth',(.22,.16,.095))
ring=[]
for i in range(32):
 a=i*math.tau/32;r=1.15+random.uniform(-.08,.08);ring.append((math.cos(a)*r,math.sin(a)*r,.003))
mesh('flat ground decal',ring,[tuple(range(32))],earth)
render('worn-earth-patch',[2.4,2.4])
(ROOT/'manifest-references.json').write_text(json.dumps({'assets':records,'ground_unit_pixels':48},indent=2))
