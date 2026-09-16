"""Blender 2.91 geometry-only guides for six isolated art proposals."""
import bpy, math, os
from mathutils import Vector
ROOT = os.path.dirname(os.path.abspath(__file__))

def setup(name, target, ortho, size):
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    s=bpy.context.scene
    s.render.engine='BLENDER_EEVEE'
    s.render.resolution_x,s.render.resolution_y=size
    s.render.resolution_percentage=100
    s.render.film_transparent=True
    s.render.image_settings.file_format='PNG'
    s.render.image_settings.color_mode='RGBA'
    s.world.color=(0.15,0.15,0.15)
    bpy.ops.object.camera_add(location=(4,-7,5) if name.startswith('bowl') else (7,-12,8))
    cam=bpy.context.object
    cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler()
    cam.data.type='ORTHO';cam.data.ortho_scale=ortho;s.camera=cam
    for pos,power,scale in [((-4,-5,8),1000,5),((4,2,6),700,4)]:
        bpy.ops.object.light_add(type='AREA',location=pos)
        l=bpy.context.object;l.data.energy=power;l.data.size=scale
        l.rotation_euler=(Vector(target)-l.location).to_track_quat('-Z','Y').to_euler()
    return s

def lathe(profile):
    n=96;vs=[];fs=[]
    for r,z in profile:
        vs.extend((r*math.cos(i*math.tau/n),r*math.sin(i*math.tau/n),z) for i in range(n))
    for j in range(len(profile)-1):
        for i in range(n):
            a=j*n+i;b=j*n+(i+1)%n;fs.append((a,b,b+n,a+n))
    mesh=bpy.data.meshes.new('continuous vessel wall');mesh.from_pydata(vs,[],fs);mesh.update()
    ob=bpy.data.objects.new('plain ritual bowl',mesh);bpy.context.collection.objects.link(ob)
    for p in mesh.polygons:p.use_smooth=True

def rod(a,b,r):
    d=Vector(b)-Vector(a)
    bpy.ops.mesh.primitive_cone_add(vertices=12,radius1=r,radius2=r*.55,depth=d.length,location=(Vector(a)+Vector(b))/2)
    bpy.context.object.rotation_euler=d.to_track_quat('Z','Y').to_euler()

def save(s,name):
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(ROOT,name+'.blend'))
    s.render.filepath=os.path.join(ROOT,name+'.png')
    bpy.ops.render.render(write_still=True)

s=setup('bowl-01',(0,0,.5),3.5,(768,768))
lathe([(0,0),(.48,0),(.5,.1),(.35,.2),(.55,.28),(.8,.5),(1,.85),(1,.92),(.93,.92),(.89,.78),(.73,.49),(.49,.33),(0,.27)])
save(s,'bowl-01-guide')
s=setup('bowl-02',(0,0,.4),3.5,(768,768))
lathe([(0,0),(.5,0),(.65,.15),(.87,.4),(1,.7),(1.02,.78),(.96,.8),(.92,.69),(.8,.43),(.58,.22),(0,.15)])
save(s,'bowl-02-guide')
for variant in [1,2]:
    s=setup('tree',(0,0,3),7.3,(512,1024))
    rod((0,0,0),(.08,0,5.7),.16)
    for i in range(10):
        z=1.5+i*.39;a=i*2.4
        reach=(.66 if variant==1 else .82)*(1-abs(z-3.6)/4)
        end=(math.cos(a)*reach,math.sin(a)*reach,z+.65)
        rod((0,0,z),end,.06)
        bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2,radius=1,location=end)
        bpy.context.object.scale=(.44,.4,.8 if variant==1 else .52)
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2,radius=1,location=(.03,0,5.55))
    bpy.context.object.scale=(.38,.34,.64)
    save(s,'tree-0%d-guide'%variant)
