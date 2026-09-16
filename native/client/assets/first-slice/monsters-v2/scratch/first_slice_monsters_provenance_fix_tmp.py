from pathlib import Path
p=Path('native/tools/first_slice_monsters_package.py');s=p.read_text()
s=s.replace("record['frames'].sort(key=lambda f:f['phase']);assert len(record['frames'])==len(refs[clip]['frames'])", """record['frames'].sort(key=lambda f:f['phase']);assert len(record['frames'])==len(refs[clip]['frames'])
        source=Path(refs[clip]['blender_source'])
        if not source.is_absolute():source=ROOT/source
        target=OUT/'blender'/source.name
        if not target.exists():shutil.copy2(source,target)
        record['blender']='blender/'+target.name
        record['blender_sha256']=sha(target)
        record['blender_action']='Idle' if record['action']=='idle' else 'Walking' if record['action']=='walk' else refs[clip].get('authored_action')""")
s=s.replace("(OUT/'approved-sheets.json').write_text", """native_refs={key:{**refs[key],'frames':[{'src':f['reference'],'sha256':f['reference_sha256'],'blender_frame':refs[key]['frames'][f['phase']]['blender_frame']} for f in record['frames']]} for key,record in manifest['clips'].items()}
    (OUT/'reference-manifest.json').write_text(json.dumps({'clips':native_refs},indent=2))
    (OUT/'transfer-manifest.json').write_text(json.dumps({'clips':native_refs},indent=2))
    (OUT/'approved-sheets.json').write_text""")
p.write_text(s)
p=Path('native/tools/first_slice_monsters_transfer.py');s=p.read_text().replace("a=m['clips'][f'{actor}-attack-{direction}'];b=", "if f'{actor}-attack-{direction}' not in m['clips']:continue\n            a=m['clips'][f'{actor}-attack-{direction}'];b=")
s=s.replace("p.add_argument('--guides',action='store_true')", "p.add_argument('--asset-root',type=Path);p.add_argument('--guides',action='store_true')")
s=s.replace("    if a.guides:guides()", "    if a.asset_root:ROOT=a.asset_root\n    if a.guides:guides()")
s=s.replace("Expected RGBA{w*16}x1024","Expected RGBA1536x1024").replace("Authored4px grid fails variance gate","Authored pixel grid fails variance gate")
p.write_text(s)
