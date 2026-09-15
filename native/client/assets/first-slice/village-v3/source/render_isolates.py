"""Isolate existing editable village props at an invariant 48 screen pixels/metre."""
import bpy, math, json
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parent
REPO=ROOT.parents[5]
SOURCE=REPO/'docs/art-review/2026-09-15-village-composition/village-composition.blend'
groups=[('longhouse',(-5,.1,0)),('mature-tree',(-7,-3,0)),('palisade',(-4,8.1,0)),('square-well',(.6,-.1,0))]
def belongs(o,name):
 n=o.name
 c=sum((o.matrix_world@Vector(v) for v in o.bound_box),Vector())/8
 if name=='longhouse':return c.x<-2 and any(n.startswith(p) for p in ['earth plaster walls','structural post','gable','continuous reed roof','reed course','open doorway'])
 if name=='mature-tree':return c.y<0 and n.startswith(('mature irregular trunk','visible branching','irregular leaf spray'))
 if name=='palisade':return n.startswith('palisade') and -5.5<=c.x<-2.5
 return n.startswith(('well stone','well post','well lifting beam'))
for name,origin in groups:
 bpy.ops.wm.open_mainfile(filepath=str(SOURCE))
 s=bpy.context.scene
 for o in list(s.objects):
  if o.type=='MESH':
   keep=belongs(o,name);o.hide_render=not keep
   if keep:o.location-=Vector(origin)
 s.render.resolution_x=s.render.resolution_y=384
 s.render.resolution_percentage=100;s.render.film_transparent=True
 s.render.image_settings.color_mode='RGBA';s.render.image_settings.color_depth='8'
 s.eevee.taa_render_samples=1
 s.camera.data.ortho_scale=8
 target=Vector((0,162/48/math.sqrt(2),162/48/math.sqrt(2)))
 s.camera.location=target+Vector((0,-20,20))
 s.camera.rotation_euler=(target-s.camera.location).to_track_quat('-Z','Y').to_euler()
 s.render.filepath=str(ROOT/(name+'-guide.png'))
 bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/(name+'.blend')))
 bpy.ops.render.render(write_still=True)
 # Construction checks only. Production camera above remains saved in blend.
 if name=='longhouse':
  for label,offset in [('side',(20,0,3)),('opposite',(-15,15,15)),('top',(0,-.01,25))]:
   target=Vector((0,0,1.5));s.camera.location=target+Vector(offset)
   s.camera.rotation_euler=(target-s.camera.location).to_track_quat('-Z','Y').to_euler()
   s.render.filepath=str(ROOT/('construction-'+label+'.png'));bpy.ops.render.render(write_still=True)
(ROOT/'projection.json').write_text(json.dumps({'pixels_per_metre':48,'camera_elevation_degrees':45,'frame':[384,384],'anchor':[192,354],'atlas':[768,768],'integer_enlargement':2,'groups':[g[0] for g in groups]},indent=2))
