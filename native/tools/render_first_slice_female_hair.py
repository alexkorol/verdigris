"""Centered back braid on the existing reviewed female rigs; original files preserved."""
import bpy, math, json, hashlib
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice'
ART=Path('Z:/Code/.worktrees/wizard-art-player-guides/art_studies/starter-derivatives-v01')
OUT=ROOT/'blender-player';OUT.mkdir(exist_ok=True)
for gait in ('walk','sprint'):
 source=ART/'paint-v2/blender'/f'female-{gait}-linen-sport.blend'
 bpy.ops.wm.open_mainfile(filepath=str(source))
 rig=bpy.data.objects['MH_female_Rig'];hair=bpy.data.objects['player-female_hair_scalp']
 attachment=min((v.co.copy() for v in hair.data.vertices),key=lambda p:abs(p.x-.025)*3+abs(p.z-1.60)+abs(p.y-.06))
 print('BRAID_ATTACHMENT',list(attachment),flush=True)
 # Curves use the same authored mesh coordinate space as the fitted scalp.
 # The three interwoven strands are centered at the nape, not on a shoulder.
 for strand in range(3):
  curve=bpy.data.curves.new('centered_back_braid','CURVE');curve.dimensions='3D'
  curve.bevel_depth=.009;curve.bevel_resolution=2
  spline=curve.splines.new('POLY');spline.points.add(80)
  for i,p in enumerate(spline.points):
   t=i/80;angle=t*math.pi*15+strand*2*math.pi/3;radius=.012*(1-.65*t)
   p.co=(attachment.x+radius*math.cos(angle),attachment.y+.07*math.sin(t*math.pi/2)+radius*math.sin(angle),attachment.z-.36*t,1)
  obj=bpy.data.objects.new('female_centered_back_braid_'+str(strand),curve)
  bpy.context.scene.collection.objects.link(obj);curve.materials.append(hair.data.materials[0])
  obj.parent=hair.parent;obj.parent_type=hair.parent_type;obj.parent_bone=hair.parent_bone
  obj.matrix_parent_inverse=hair.matrix_parent_inverse.copy();obj.matrix_basis=hair.matrix_basis.copy()
  obj['design']='Single braid centered behind neck to upper back; no side tail'
 hair['style']='Long hair drawn into one centered back braid'
 dst=OUT/f'female-{gait}-center-braid.blend'
 renderer=ART/'blender/render_baked_frames.py'
 exec(compile(renderer.read_text(),str(renderer),'exec'),{'__file__':str(renderer),'SEX':'female','GAIT':gait,'OUTPUT_DIR':str(OUT),'SAVE_PATH':str(dst),'FRAMING_PATH':str(OUT/f'female-{gait}-framing.json')})
