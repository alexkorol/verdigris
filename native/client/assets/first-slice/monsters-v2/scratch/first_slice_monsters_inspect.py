import bpy,json
r=bpy.data.objects['WolfArmature']
print('WOLF_BONES',json.dumps([{'name':b.name,'head':list(b.head_local),'tail':list(b.tail_local),'parent':b.parent.name if b.parent else None} for b in r.data.bones]))
print('PARENTS',[(o.name,o.parent.name if o.parent else None,list(o.location),list(o.rotation_euler),list(o.scale)) for o in bpy.context.scene.objects if o.type=='ARMATURE' or 'normalization' in o.name or 'facing' in o.name])
