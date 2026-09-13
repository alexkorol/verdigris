"""Revise the recovered Tree 01 scene; no appearance generation or runtime edits.
Run with Blender 2.91 --background --python <this file>.
"""
import bpy
import json
import math
import random
from pathlib import Path
from mathutils import Vector
from bpy_extras.object_utils import world_to_camera_view

ROOT = Path(__file__).resolve().parent
random.seed(301)
bpy.ops.wm.open_mainfile(filepath=str(ROOT / 'baseline/tree-01-guide.blend'))
s = bpy.context.scene
bpy.context.preferences.filepaths.save_version = 0
camera = s.camera
camera_before = {'matrix_world': [list(v) for v in camera.matrix_world],
                 'ortho_scale': camera.data.ortho_scale}
# Measured runtime hero: 63 visible rows. Adult model height is a review
# assumption, not a recovered canonical metres-per-pixel standard.
adult_height = 1.8
adult_fraction = abs(world_to_camera_view(s, camera, Vector((0, 0, adult_height))).y
                     - world_to_camera_view(s, camera, Vector((0, 0, 0))).y)
logical_h = 2 * round(63 / adult_fraction / 2)
logical_w = logical_h // 2

def render(name, logical=False):
    s.render.resolution_x = logical_w if logical else 512
    s.render.resolution_y = logical_h if logical else 1024
    s.render.resolution_percentage = 100
    s.render.filepath = str(ROOT / (name + '.png'))
    bpy.ops.render.render(write_still=True)

render('original-model')
render('original-logical-raw', True)
foliage = sorted([o for o in s.objects if o.type == 'MESH' and o.name.startswith('Icosphere')], key=lambda o:o.name)
original_trunk_and_branches = [o.name for o in s.objects if o.type == 'MESH' and o not in foliage]
clusters = [(o.name, o.location.copy(), o.scale.copy()) for o in foliage]
for o in foliage:
    bpy.data.objects.remove(o, do_unlink=True)

wood = bpy.data.materials.new('neutral branch structure')
wood.diffuse_color = (.23, .23, .23, 1)
leafmats = []
for val in [.36, .43, .5, .58]:
    mat = bpy.data.materials.new('neutral leaf %.2f' % val)
    mat.diffuse_color = (val,val,val,1)
    leafmats.append(mat)
for name in original_trunk_and_branches:
    s.objects[name].data.materials.clear()
    s.objects[name].data.materials.append(wood)

def branch(name, points, radius):
    curve = bpy.data.curves.new(name, 'CURVE')
    curve.dimensions = '3D'
    curve.resolution_u = 2
    curve.bevel_depth = radius
    curve.bevel_resolution = 1
    spline = curve.splines.new('POLY')
    spline.points.add(len(points)-1)
    for i, (p, q) in enumerate(zip(spline.points,points)):
        p.co = (*q,1)
        p.radius = max(.12,1-i/(len(points)-.5))
    ob = bpy.data.objects.new(name, curve)
    s.collection.objects.link(ob)
    curve.materials.append(wood)

verts, faces, mids = [], [], []
def leaf(pos, length, angle, tilt):
    direction = Vector((math.cos(angle),math.sin(angle),tilt)).normalized()
    width = direction.cross(Vector((0,0,1))).normalized() * length * .32
    n = len(verts)
    verts.extend([tuple(pos),tuple(pos + direction*length*.45 + width),
                  tuple(pos + direction*length),tuple(pos + direction*length*.45 - width)])
    faces.append((n,n+1,n+2,n+3))
    mids.append(random.randrange(len(leafmats)))

# Keep original main-branch endpoints and crown envelope. Change subordinate
# construction only: varying mass sizes and exposed branch windows.
weights = [1.0,.6,1.12,.78,.40,1.10,.70,.50,1.05,.83,.86]
for group, (name, center, oldscale) in enumerate(clusters):
    weight = weights[group]
    crown = group == 10
    base = center - Vector((0,0,.30 if crown else .23))
    count = max(7, round(25*weight))
    for j in range(count):
        a = j*2.39996 + group*.7 + random.uniform(-.28,.28)
        radial = random.uniform(.22,.44)*math.sqrt(weight)
        tip = center + Vector((math.cos(a)*radial, math.sin(a)*radial,
                               random.uniform(-.38,.56)*weight))
        mid = base.lerp(tip,.55) + Vector((0,0,-.09))
        branch('group%02d-secondary%02d' % (group,j), [base,mid,tip], .018)
        for side in [-1,1]:
            attach = mid.lerp(tip,.38)
            end = tip + Vector((math.cos(a+side*.85)*.12,math.sin(a+side*.85)*.12,.12))
            branch('group%02d-twig%02d-%d' % (group,j,side),[attach,end],.007)
            for k in range(14):
                pos = attach.lerp(end,(k+1)/15)
                leaf(pos, random.uniform(.14,.23), a+side*1.05+(k%2)*2.1,
                     random.uniform(-.1,.65))
mesh = bpy.data.meshes.new('individual leaves on secondary twigs')
mesh.from_pydata(verts,[],faces)
mesh.update()
ob = bpy.data.objects.new('foliage - individual leaf planes',mesh)
s.collection.objects.link(ob)
for mat in leafmats:
    mesh.materials.append(mat)
for p,m in zip(mesh.polygons,mids):
    p.material_index = m

assert camera_before == {'matrix_world':[list(v) for v in camera.matrix_world],
                         'ortho_scale':camera.data.ortho_scale}
render('revised-model')
render('revised-logical-raw', True)
ob.hide_render = True
render('revised-branches')
ob.hide_render = False
s.render.resolution_x, s.render.resolution_y = logical_w, logical_h
bpy.ops.wm.save_as_mainfile(filepath=str(ROOT / 'tree-01-revised.blend'))
(ROOT/'model-provenance.json').write_text(json.dumps({
    'baseline':'baseline/tree-01-guide.blend',
    'camera_unchanged':camera_before,'lighting_unchanged':True,
    'retained_meshes':original_trunk_and_branches,
    'logical_canvas':[logical_w,logical_h], 'reference_hero_visible_rows':63,
    'adult_model_height_assumption':adult_height,
    'projected_adult_fraction':adult_fraction,
    'scale_status':'Measured hero pixels; model-world adult height provisional. No canonical player-plane calibration found.',
    'seed':301,'appearance_generation_performed':False,'runtime_integrated':False,
    'review_status':'unapproved model-stage revision', 'foliage_group_weights':weights
},indent=2))
