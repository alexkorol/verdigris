import json,hashlib,shutil
from pathlib import Path
R=Path(__file__).resolve().parents[1]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
prompts=json.loads((R/'imagegen-prompts.json').read_text())
original=json.loads((R/'combat-imagegen-prompts.json').read_text())
refs=json.loads((R/'reference-manifest.json').read_text())['clips']
keys=['well-alpha-attack-front','well-alpha-attack-back']
for key in keys:prompts[key]=original[key]
# Preserve the three already reviewed cells of the original left sheet.
key='well-alpha-attack-left-original-good'
old='well-alpha-attack-left'
report=json.loads((R/'review'/f'{old}.json').read_text())
report['clip']=key;report['frames']=[f for f in report['frames'] if not f['file'].endswith('-02.png')]
report['visual_acceptance']=True
report['review_note']='Original phases 0,1,3 retained after independent complete repaired-cohort review.'
(R/'review'/f'{key}.json').write_text(json.dumps(report,indent=2))
shutil.copy2(R/'source'/f'{old}.png',R/'source'/f'{key}.png')
prompts[key]=original[old];keys.append(key)
right=json.loads((R/'projection-proof/boss-attack/right-single-results.json').read_text())
for direction,phases in [('left',[2]),('right',list(range(4)))]:
    for phase in phases:
        clip=f'well-alpha-attack-{direction}';key=f'{clip}-single-{phase:02d}'
        folder=R/'projection-proof/boss-attack'/('paint-left-02-style' if direction=='left' else 'paint-right')
        report=json.loads((folder/'review'/f'{key}.json').read_text())
        report['frames'][0]['reference']=refs[clip]['frames'][phase]
        guide=R/'projection-proof/boss-attack/single-guides'/f'{clip}-{phase:02d}.png'
        style=R/'projection-proof/boss-attack'/f'idle-{direction}-style-4x.png'
        report['upload_guide']=str(guide.relative_to(R));report['upload_guide_sha256']=sha(guide)
        report['additional_references']=[{'file':str(style.relative_to(R)),'sha256':sha(style)}]
        report['layout']=json.loads((folder/'transfer-manifest-v4.json').read_text())['clips'][key]['layout']
        report['visual_acceptance']=True
        report['review_note']='Independent complete cohort reviewed; exact authored placement, matching coarse coat and contour.'
        (R/'review'/f'{key}.json').write_text(json.dumps(report,indent=2))
        name=report['frames'][0]['file']
        shutil.copy2(folder/'candidates'/name,R/'candidates'/name)
        shutil.copy2(folder/'source'/f'{key}.png',R/'source'/f'{key}.png')
        prompt=json.loads((folder/'prompt.json').read_text()) if direction=='left' else right[phase]
        prompts[key]={'prompt':prompt['prompt'],'guide':'guides/'+guide.name,'guide_sha256':sha(guide),'style_reference':'guides/'+style.name,'style_sha256':sha(style),'source_sha256':report['source_sha256']}
        keys.append(key)
(R/'imagegen-prompts.json').write_text(json.dumps(prompts,indent=2))
(R/'scratch/boss-attack-approved-report-keys.json').write_text(json.dumps(keys,indent=2))
print(keys)
