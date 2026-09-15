"""Blender: distinct village attackers from the reviewed MakeHuman/CMU rig.

Run with -- source-scenes output-folder identity guides|render [action].
Guide and animation geometry share the same deterministic variant operation.
ImageGen paints the four native guides; vertex-index UVs carry that one painting
through the existing motions. No per-frame image generation or alpha-bound fit.
"""
import bpy, json, math, sys, hashlib
import numpy as np
from pathlib import Path
from mathutils import Matrix, Vector
from mathutils.bvhtree import BVHTree
from bpy_extras.object_utils import world_to_camera_view

args=sys.argv[sys.argv.index('--')+1:]
SOURCE,OUT=map(Path,args[:2]); ID,MODE=args[2:4]
ACTION=args[4] if len(args)>4 else 'idle'
assert ID in ('village-invader','village-leader','defender','scribe')
SEX='female' if ID=='scribe' else 'male'
OUT=OUT/ID
for folder in ('guides','scenes','frames','reports'):(OUT/folder).mkdir(parents=True,exist_ok=True)
DIRS=('front','right','back','left')
PHASES={'idle':[1],'walk':[72,80,88,96],'attack':[1,13,25,37,49,61],'hit':[1,29],'death':[1,9,13,29]}
boss=ID=='village-leader'
COLORS={'skin':(.40,.25,.16,1),'cloth':(.22,.115,.05,1) if boss else (.08,.095,.075,1),
        'hair':(.055,.037,.026,1) if boss else (.016,.012,.009,1),'rope':(.22,.16,.095,1),'wood':(.12,.075,.035,1)}
if ID=='defender':COLORS.update(cloth=(.19,.16,.10,1),hair=(.22,.21,.18,1))
if ID=='scribe':COLORS.update(cloth=(.18,.16,.12,1),hair=(.095,.047,.02,1))

def category(o):
    name=o.name
    if 'hair' in name or 'beard' in name or 'braid' in name:return 'hair'
    if 'Body' in name:return 'skin'
    if 'cloth_samples' in name:return 'cloth'
    if 'club' in name:return 'wood'
    return 'rope'

def material(name,color):
    mat=bpy.data.materials.new(name);mat.diffuse_color=color;mat.use_nodes=True
    p=mat.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=color;p.inputs['Roughness'].default_value=.9
    return mat

def load(action):
    path=SOURCE/f'{SEX}-club-{action}-projection.blend'
    bpy.ops.wm.open_mainfile(filepath=str(path));s=bpy.context.scene;s.frame_set(PHASES[action][0])
    # Short close nape on the light attacker; the leader retains longer hair
    # and a full jaw beard. The face/nose mesh is never replaced by hair.
    if SEX=='male':
        hair=bpy.data.objects['player-male_hair_scalp']
        cutoff=1.55 if boss else 1.59 if ID=='defender' else 1.615
        for v in hair.data.vertices:
            if v.co.z<cutoff:v.co.z=cutoff+(v.co.z-cutoff)*(.48 if boss else .12)
            v.co.x*=1.0 if boss else .94
        if ID=='village-invader':bpy.data.objects['player-male_jaw_beard'].hide_render=True
    # One actor transform carries skin, baked cloth, belt, rig and hand-held
    # weapon together, preserving all reviewed collision/grip relationships.
    root=bpy.data.objects.new(ID+'_anatomy',None);s.collection.objects.link(root)
    for o in list(s.objects):
        if o==root or o.parent or o.type in ('CAMERA','LIGHT'):continue
        m=o.matrix_world.copy();o.parent=root;o.matrix_world=m
    root.scale=(1.14,1.09,1.035) if boss else (1.04,1.04,.98) if ID=='defender' else (1,1,1) if SEX=='female' else (.95,.97,.99)
    # The leader's heavy branch has a thicker striking end, not a larger grip.
    club=bpy.data.objects[f'{SEX}_starter_branch_club']
    if ID in ('defender','scribe'):club.hide_render=True
    if boss:
        for ring in range(7):
            verts=list(club.data.vertices)[ring*9:(ring+1)*9]
            center=sum((v.co for v in verts),Vector())/len(verts)
            for v in verts:v.co=center+(v.co-center)*(1+.35*ring/6)
    for o in s.objects:
        if o.type not in ('MESH','CURVE') or o.hide_render or not o.visible_get():continue
        # Retain independently UV-painted branch. New guide clothing is one
        # continuous baked shell, avoiding an intersecting extra armor layer.
        if 'starter_branch_club' in o.name:continue
        o.data.materials.clear();o.data.materials.append(material(ID+'_'+category(o),COLORS[category(o)]))
    s.render.engine='CYCLES';s.cycles.samples=12;s.cycles.use_denoising=False
    s.cycles.pixel_filter_type='BOX';s.cycles.filter_width=1
    s.render.film_transparent=True;s.render.image_settings.color_mode='RGBA';s.render.resolution_percentage=100
    s.view_settings.view_transform='Standard';s.view_settings.exposure=0;s.view_settings.gamma=1
    bpy.context.view_layer.update()
    return s,path

def geometry_digest(s):
    h=hashlib.sha256()
    for o in sorted(s.objects,key=lambda x:x.name):
        if o.type!='MESH' or o.hide_render or not o.visible_get():continue
        h.update(o.name.encode());h.update(np.array([v.co[:] for v in o.data.vertices],dtype=np.float32).tobytes())
    return h.hexdigest()

def reference_records(s):
    records={};base=s.camera.matrix_world.copy()
    for d in range(4):
        s.camera.matrix_world=Matrix.Rotation(-math.pi*d/2,4,'Z')@base;bpy.context.view_layer.update();eye=s.camera.matrix_world.translation
        verts=[];faces=[];active=[];deps=bpy.context.evaluated_depsgraph_get()
        for o in s.objects:
            if o.type not in ('MESH','CURVE') or o.hide_render or not o.visible_get():continue
            active.append(o);ev=o.evaluated_get(deps);me=ev.to_mesh();off=len(verts)
            verts.extend(o.matrix_world@v.co for v in me.vertices);faces.extend(tuple(off+i for i in f.vertices) for f in me.polygons);ev.to_mesh_clear()
        bvh=BVHTree.FromPolygons(verts,faces)
        for o in active:
            if o.type!='MESH' or 'starter_branch_club' in o.name:continue
            disabled=[]
            for m in o.modifiers:
                if m.type in ('MASK','SUBSURF','SOLIDIFY'):disabled.append((m,m.show_viewport));m.show_viewport=False
            bpy.context.view_layer.update();ev=o.evaluated_get(bpy.context.evaluated_depsgraph_get());me=ev.to_mesh()
            assert len(me.vertices)==len(o.data.vertices),o.name
            points=[o.matrix_world@v.co for v in me.vertices]
            normals=[(o.matrix_world.to_3x3().inverted().transposed()@v.normal).normalized() for v in me.vertices];ev.to_mesh_clear()
            for m,val in disabled:m.show_viewport=val
            bpy.context.view_layer.update()
            if o.name not in records:records[o.name]=[np.zeros((len(points),4,2),dtype=np.float32),np.zeros((len(points),4),dtype=np.float32)]
            uv,weights=records[o.name]
            for i,(point,normal) in enumerate(zip(points,normals)):
                q=world_to_camera_view(s,s.camera,point)
                uv[i,d]=((d%2*128+16+q.x*96)/256,(128-(d//2)*128+16+q.y*96)/256)
                toward=(eye-point).normalized();facing=max(0,normal.dot(toward))
                if facing<.05 or not 0<=q.x<=1 or not 0<=q.y<=1:continue
                hit=bvh.ray_cast(eye,-toward,(eye-point).length+.03)
                if hit[0] is not None and abs(hit[3]-(eye-point).length)<.025:weights[i,d]=facing**6
        s.render.filepath=str(OUT/'guides'/f'{DIRS[d]}.png');bpy.ops.render.render(write_still=True)
    s.camera.matrix_world=base
    return records

def project(s,records):
    paint=bpy.data.images.load(str(OUT/'appearance-native.png'));paint.alpha_mode='STRAIGHT';paint.pack()
    for name,(uv,weights) in records.items():
        o=bpy.data.objects.get(name)
        if not o or o.hide_render or not o.visible_get():continue
        assert len(o.data.vertices)==len(uv),name
        for layer in list(o.data.uv_layers):o.data.uv_layers.remove(layer)
        # Select the strongest source at each vertex; normalized interpolation
        # stays on the same surface. Four full views cover moving limbs.
        total=weights.sum(axis=1);weights=weights/np.maximum(total[:,None],1e-12)
        for d in range(4):
            layer=o.data.uv_layers.get('EnemyView'+str(d)) or o.data.uv_layers.new(name='EnemyView'+str(d))
            for lp in o.data.loops:layer.data[lp.index].uv=uv[lp.vertex_index,d]
        attr=o.data.color_attributes.get('EnemyWeight') or o.data.color_attributes.new(name='EnemyWeight',type='FLOAT_COLOR',domain='POINT')
        attr.data.foreach_set('color',weights.reshape(-1))
        mat=bpy.data.materials.new(ID+'_paint_'+name);mat.use_nodes=True;n=mat.node_tree.nodes;l=mat.node_tree.links;n.clear()
        out=n.new('ShaderNodeOutputMaterial');em=n.new('ShaderNodeEmission');l.new(em.outputs[0],out.inputs['Surface'])
        wc=n.new('ShaderNodeVertexColor');wc.layer_name='EnemyWeight';sep=n.new('ShaderNodeSeparateColor');l.new(wc.outputs['Color'],sep.inputs[0])
        summed=None;wsum=None
        for d in range(4):
            tex=n.new('ShaderNodeTexImage');tex.image=paint;tex.interpolation='Closest';tex.extension='CLIP'
            coord=n.new('ShaderNodeUVMap');coord.uv_map='EnemyView'+str(d);l.new(coord.outputs[0],tex.inputs[0])
            w=sep.outputs[d] if d<3 else wc.outputs['Alpha']
            aw=n.new('ShaderNodeMath');aw.operation='MULTIPLY';l.new(w,aw.inputs[0]);l.new(tex.outputs['Alpha'],aw.inputs[1])
            mul=n.new('ShaderNodeVectorMath');mul.operation='SCALE';l.new(tex.outputs['Color'],mul.inputs[0]);l.new(aw.outputs[0],mul.inputs['Scale'])
            if summed is None:summed=mul.outputs[0];wsum=aw.outputs[0]
            else:
                add=n.new('ShaderNodeVectorMath');add.operation='ADD';l.new(summed,add.inputs[0]);l.new(mul.outputs[0],add.inputs[1]);summed=add.outputs[0]
                addw=n.new('ShaderNodeMath');addw.operation='ADD';l.new(wsum,addw.inputs[0]);l.new(aw.outputs[0],addw.inputs[1]);wsum=addw.outputs[0]
        inv=n.new('ShaderNodeMath');inv.operation='DIVIDE';inv.inputs[0].default_value=1;l.new(wsum,inv.inputs[1])
        norm=n.new('ShaderNodeVectorMath');norm.operation='SCALE';l.new(summed,norm.inputs[0]);l.new(inv.outputs[0],norm.inputs['Scale'])
        valid=n.new('ShaderNodeMath');valid.operation='GREATER_THAN';l.new(wsum,valid.inputs[0]);valid.inputs[1].default_value=.01
        mix=n.new('ShaderNodeMixRGB');l.new(valid.outputs[0],mix.inputs[0]);mix.inputs[1].default_value=COLORS[category(o)];l.new(norm.outputs[0],mix.inputs[2]);l.new(mix.outputs[0],em.inputs[0])
        o.data.materials.clear();o.data.materials.append(mat)

s,source=load('idle')
if MODE=='guides':
    records=reference_records(s)
    np.savez_compressed(OUT/'projection.npz',**{name+'__'+str(k):v for name,parts in records.items() for k,v in enumerate(parts)})
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'scenes'/f'{ID}-reference.blend'),compress=True)
else:
    data=np.load(OUT/'projection.npz');records={key[:-3]:[data[key],data[key[:-1]+'1']] for key in data.files if key.endswith('__0')}
    s,source=load(ACTION);project(s,records)
    base=[(o,o.matrix_world.copy()) for o in s.objects if o.type in ('CAMERA','LIGHT')]
    size=s.render.resolution_x;anchor=[48,80] if size==96 else [64,96]
    before=geometry_digest(s)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'scenes'/f'{ID}-{ACTION}.blend'),compress=True)
    for d in range(4):
        for o,m in base:o.matrix_world=Matrix.Rotation(-math.pi*d/2,4,'Z')@m
        for i,frame in enumerate(PHASES[ACTION]):
            s.frame_set(frame);p=world_to_camera_view(s,s.camera,Vector())
            assert abs(p.x*size-anchor[0])<.01 and abs((1-p.y)*size-anchor[1])<.01
            s.render.filepath=str(OUT/'frames'/f'{ID}-{ACTION}-{DIRS[d]}-{i:02d}.png');bpy.ops.render.render(write_still=True)
    (OUT/'reports'/f'{ACTION}.json').write_text(json.dumps({'source':source.name,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'geometry_sha256':before,'appearance_sha256':hashlib.sha256((OUT/'appearance-native.png').read_bytes()).hexdigest(),'source_frames':PHASES[ACTION],'frame':[size,size],'anchor':anchor,'pixels_per_metre':48,'filter':'BOX 1.0','identity':ID,'action':ACTION},indent=2))
