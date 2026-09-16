"""Reconstruct a reviewed single-frame cohort at authored placement, then stage it.

This never publishes accepted assets. All original inputs stay in place; every
candidate must reproduce byte-for-byte before any shared candidate is copied.
"""
import argparse, hashlib, json, shutil, sys
from pathlib import Path
import first_slice_monsters_transfer_v4 as transfer

ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/monsters-v2'
REPO=Path(__file__).resolve().parents[2]
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()

def run(cohort_path):
    cohort_path=Path(cohort_path).resolve();base=cohort_path.parent
    cohort=json.loads(cohort_path.read_text());assert cohort['accepted'] is True
    clip=cohort['clip'];reference=json.loads((ROOT/'reference-manifest.json').read_text())['clips'][clip]
    assert cohort['frame']==reference['frame'] and cohort['anchor']==reference['anchor']
    assert sorted(f['phase'] for f in cohort['frames'])==list(range(len(reference['frames'])))
    stage=ROOT/'single-transfer'/clip;stage.mkdir(parents=True,exist_ok=True)
    records={};inputs={}
    for f in cohort['frames']:
        phase=f['phase'];key=f'{clip}-single-{phase:02d}'
        guide=Path(f['guide']);guide=guide if guide.is_absolute() else REPO/guide
        source=base/f['source'];native=base/f['frame']
        assert sha(guide)==f['guide_sha256'] and sha(source)==f['source_sha256'] and sha(native)==f['sha256']
        assert f['diagnostic_translation']==[0,0]
        ref=reference['frames'][phase].copy();ref['src']=str(ROOT/ref['src'])
        records[key]={**reference,'frames':[ref],'layout':{'columns':1,'slot':[192,128],'inset':[(192-reference['frame'][0])//2,16],'scale':8,'output':[1536,1024],'fixed_translation':[0,0]}}
        inputs[key]=(f,guide,source,native)
    (stage/'transfer-manifest-v4.json').write_text(json.dumps({'clips':records},indent=2))
    transfer.ROOT=stage
    for key,(f,guide,source,native) in inputs.items():
        transfer.recover(source,key)
        recovered=stage/'candidates'/Path(records[key]['frames'][0]['src']).name
        assert sha(recovered)==sha(native),f'Pixel transfer differs from reviewed frame: {key}'
    # No shared writes before the complete cohort reproduces.
    prompts=json.loads((ROOT/'imagegen-prompts.json').read_text())
    for key,(f,guide,source,native) in inputs.items():
        record=records[key];frame_name=Path(record['frames'][0]['src']).name
        report=json.loads((stage/'review'/f'{key}.json').read_text())
        report['frames'][0]['reference']=reference['frames'][f['phase']]
        guide_target=ROOT/'guides-single'/f'{key}.png';guide_target.parent.mkdir(exist_ok=True)
        shutil.copy2(guide,guide_target)
        report['upload_guide']=str(guide_target.relative_to(ROOT));report['upload_guide_sha256']=sha(guide_target)
        report['layout']=record['layout'];report['visual_acceptance']=True
        report['review_note']=cohort.get('visual_acceptance','Reviewed complete cohort')
        shutil.copy2(stage/'candidates'/frame_name,ROOT/'candidates'/frame_name)
        shutil.copy2(source,ROOT/'source'/f'{key}.png')
        (ROOT/'review'/f'{key}.json').write_text(json.dumps(report,indent=2))
        prompts[key]={'prompt':(base/f['prompt']).read_text(),'guide':'guides/'+guide_target.name,'guide_sha256':sha(guide_target),'source_sha256':sha(source),'method':'single colored Blender pose; fixed 8px grid and fixed authored placement'}
    (ROOT/'imagegen-prompts.json').write_text(json.dumps(prompts,indent=2))
    (stage/'approved-report-keys.json').write_text(json.dumps(list(records),indent=2))
    print('Staged reviewed cohort',clip,'with',len(records),'exactly reproduced frames; not published')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('cohort');run(p.parse_args().cohort)
