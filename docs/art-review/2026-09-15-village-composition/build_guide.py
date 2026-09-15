"""One editable composition proposal; no production assets are overwritten."""
import bpy, math, random
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parent
REPO=ROOT.parents[2]
random.seed(15)
bpy.ops.wm.read_factory_settings(use_empty=True)
s=bpy.context.scene
s.render.engine='BLENDER_EEVEE'
s.eevee.use_gtao=True
s.eevee.gtao_distance=3
s.render.resolution_x=768;s.render.resolution_y=512;s.render.resolution_percentage=100
s.view_settings.view_transform='Standard';s.view_settings.look='Medium High Contrast'
s.world=bpy.data.worlds.new('overcast sky');s.world.color=(.24,.27,.32)
def mat(n,c):
 m=bpy.data.materials.new(n);m.diffuse_color=(*c,1);return m
earth=mat('trampled ochre earth',(.25,.20,.13));clay=mat('warm clay daub',(.49,.40,.27))
wood=mat('weathered timber',(.18,.13,.08));reed=mat('bundled reed',(.36,.30,.17))
stone=mat('local fieldstone',(.34,.35,.29));green=mat('woodland green',(.11,.19,.10))
leaves=[mat('leaf '+str(i),c) for i,c in enumerate([(.11,.20,.09),(.17,.25,.12),(.22,.30,.15),(.13,.22,.14)])]
def box(n,p,d,m):
 bpy.ops.mesh.primitive_cube_add(size=1,location=p);o=bpy.context.object;o.name=n;o.dimensions=d;o.data.materials.append(m);return o
def beam(n,a,b,r,m):
 a,b=Vector(a),Vector(b);v=b-a
 bpy.ops.mesh.primitive_cone_add(vertices=7,radius1=r,radius2=r*.7,depth=v.length,location=(a+b)/2)
 o=bpy.context.object;o.name=n;o.rotation_euler=v.to_track_quat('Z','Y').to_euler();o.data.materials.append(m);return o
def mesh(n,vs,faces,m):
 me=bpy.data.meshes.new(n);me.from_pydata(vs,[],faces);o=bpy.data.objects.new(n,me);s.collection.objects.link(o);o.data.materials.append(m);return o
box('village clearing',(0,1,-.08),(24,28,.15),earth)
def house(x,y,w,d):
 box('earth plaster walls',(x,y,.95),(w,d,1.9),clay)
 for xx in [-w/2,w/2]:
  for yy in [-d/2,0,d/2]:beam('structural post',(x+xx,y+yy,0),(x+xx,y+yy,2.1),.09,wood)
 for yy in [-d/2,d/2]:
  mesh('gable',[(x-w/2,y+yy,1.9),(x+w/2,y+yy,1.9),(x,y+yy,3.5)],[(0,1,2)],clay)
  beam('gable tie',(x-w/2,y+yy,1.9),(x+w/2,y+yy,1.9),.08,wood)
 for side in [-1,1]:
  mesh('continuous reed roof',[(x,y-d/2-.35,3.6),(x,y+d/2+.35,3.6),(x+side*(w/2+.4),y+d/2+.35,1.85),(x+side*(w/2+.4),y-d/2-.35,1.85)],[(0,1,2,3)],reed)
  for k in range(20):
   yy=y-d/2+k*d/19
   beam('reed course',(x,yy,3.61),(x+side*(w/2+.4),yy,1.87),.025,reed)
 box('open doorway',(x,y-d/2-.015,.8),(.85,.03,1.6),wood)
house(-5,3,3.8,5.8);house(5,4.1,3.6,6.0)
for x in [i*.25-9 for i in range(73)]:
 if .7<x<3.0:continue
 h=2.1+random.uniform(-.15,.15)
 beam('palisade',(x,8.1,0),(x,8.1,h),.13,wood)
for i in range(4):
 for j in range(4):
  if i not in [0,3] and j not in [0,3]:continue
  box('well stone',(.6+(i-1.5)*.29,-.1+(j-1.5)*.29,.36),(.29,.29,.72),stone)
for x in [-.12,1.32]:beam('well post',(x,-.1,0),(x,-.1,1.8),.07,wood)
beam('well lifting beam',(-.2,-.1,1.8),(1.4,-.1,1.8),.07,wood)
def tree(x,y):
 beam('mature irregular trunk',(x,y,0),(x+.18,y,4.8),.25,wood)
 for i in range(9):
  a=i*2.4;end=Vector((x+math.cos(a)*random.uniform(1,2),y+math.sin(a)*random.uniform(1,2),random.uniform(4.1,6)))
  beam('visible branching',(x+.1,y,2.8+i*.15),end,.10,wood)
  for k in range(13):
   p=end+Vector((random.uniform(-.7,.7),random.uniform(-.7,.7),random.uniform(-.3,.5)))
   r=random.uniform(.15,.45)
   mesh('irregular leaf spray',[p+Vector((-r,0,0)),p+Vector((0,-r,.1)),p+Vector((r,0,0)),p+Vector((0,r,.08))],[(0,1,2,3)],random.choice(leaves))
tree(-8,6);tree(8,7);tree(-7,-3)
for sex,x,y in [('male',-1.2,-2.1),('female',-3,-.7)]:
 p=REPO/'native/client/assets/first-slice/starter-projection/accepted'/sex/'frames'/f'{sex}-club-idle-front-00.png'
 im=bpy.data.images.load(str(p));m=bpy.data.materials.new(sex+' scale marker');m.use_nodes=True;m.blend_method='CLIP';m.shadow_method='NONE';m.alpha_threshold=.5;m.use_screen_refraction=False
 nt=m.node_tree;nt.nodes.clear();tex=nt.nodes.new('ShaderNodeTexImage');tex.image=im;tex.interpolation='Closest'
 em=nt.nodes.new('ShaderNodeEmission');nt.links.new(tex.outputs['Color'],em.inputs[0]);trans=nt.nodes.new('ShaderNodeBsdfTransparent');mix=nt.nodes.new('ShaderNodeMixShader');nt.links.new(tex.outputs['Alpha'],mix.inputs[0]);nt.links.new(trans.outputs[0],mix.inputs[1]);nt.links.new(em.outputs[0],mix.inputs[2]);out=nt.nodes.new('ShaderNodeOutputMaterial');nt.links.new(mix.outputs[0],out.inputs[0])
 bpy.ops.mesh.primitive_plane_add(size=2,location=(x,y,.9));o=bpy.context.object;o.name=sex+' native scale guide';o.rotation_euler=(math.radians(48),0,0);o.data.materials.append(m)
cam=bpy.data.cameras.new('straight grid camera');o=bpy.data.objects.new('straight grid camera',cam);s.collection.objects.link(o);o.location=(0,-18,18);target=Vector((0,3,0));o.rotation_euler=(target-o.location).to_track_quat('-Z','Y').to_euler();cam.type='ORTHO';cam.ortho_scale=16;s.camera=o
bpy.ops.object.light_add(type='AREA',location=(-4,-6,12));bpy.context.object.data.energy=1600;bpy.context.object.data.size=8
s.render.image_settings.file_format='PNG';s.render.filepath=str(ROOT/'guide-native.png')
bpy.ops.file.pack_all()
bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/'village-composition.blend'))
bpy.ops.render.render(write_still=True)
