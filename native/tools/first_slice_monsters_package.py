"""Publish only explicitly visually reviewed monster sheets and their provenance."""
import argparse,json,hashlib,shutil
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/monsters-v2'
OUT=ROOT/'accepted'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def package(approved):
    refs=json.loads((ROOT/'reference-manifest.json').read_text())['clips']
    variable_layouts=json.loads((ROOT/'transfer-manifest-v4.json').read_text())['clips'] if (ROOT/'transfer-manifest-v4.json').exists() else {}
    sheet_transfers={}
    manifest={'schema':1,'pixels_per_metre':48,'alpha':'binary native RGBA; original native alpha threshold128, neverRGBkey','clips':{},'source':'Blender authored poses, built-in imagegen paint, Pixel Respecter4xgrid reconstruction','visual_acceptance':'Agent native-scale andintegerzoom inspection; owner acceptance notclaimed'}
    for folder in ('frames','references','originals','reports','blender','guides'):(OUT/folder).mkdir(parents=True,exist_ok=True)
    if (ROOT/'source-credit.json').exists():
        credit=json.loads((ROOT/'source-credit.json').read_text());source=ROOT/credit['original_file']
        assert sha(source)==credit['original_sha256']
        shutil.copy2(source,OUT/credit['original_file']);shutil.copy2(ROOT/'source-credit.json',OUT/'source-credit.json')
        manifest['model_source']='source-credit.json'
    for key in approved:
        report=json.loads((ROOT/'review'/f'{key}.json').read_text());source=ROOT/'source'/f'{key}.png'
        assert sha(source)==report['source_sha256']
        shutil.copy2(source,OUT/'originals'/source.name)
        report['visual_acceptance']=True
        (OUT/'reports'/f'{key}.json').write_text(json.dumps(report,indent=2))
        upload_guide=report.get('upload_guide')
        if upload_guide:
            guide_source=ROOT/upload_guide;assert sha(guide_source)==report['upload_guide_sha256']
            shutil.copy2(guide_source,OUT/'guides'/guide_source.name)
        for extra in report.get('additional_references',[]):
            p=ROOT/extra['file'];assert sha(p)==extra['sha256']
            shutil.copy2(p,OUT/'guides'/p.name)
        for f in report['frames']:
            clip=Path(f['file']).stem.rsplit('-',1)[0];record=refs[clip]
            p=ROOT/'candidates'/f['file'];r=ROOT/f['reference']['src']
            assert sha(p)==f['sha256'] and sha(r)==f['reference']['sha256']
            im=Image.open(p);assert im.size==tuple(record['frame']) and im.mode=='RGBA'
            assert set(im.getchannel('A').getdata())=={0,255}
            assert f['silhouette_iou'] >= (.70 if record['action'] in ('attack','hit','death') else .65)
            shutil.copy2(p,OUT/'frames'/p.name);shutil.copy2(r,OUT/'references'/r.name)
            if clip not in manifest['clips']:
                manifest['clips'][clip]={k:record[k] for k in ('actor','action','direction','fps','loop','frame','anchor','pixels_per_metre')}
                manifest['clips'][clip]['frames']=[]
            manifest['clips'][clip]['frames'].append({'file':'frames/'+p.name,'sha256':sha(p),'reference':'references/'+r.name,'reference_sha256':sha(r),'phase':int(p.stem.rsplit('-',1)[1]),'source':'originals/'+source.name,'source_sha256':sha(source),'silhouette_iou':f['silhouette_iou'],'report':'reports/'+key+'.json'})
            if upload_guide:
                manifest['clips'][clip]['frames'][-1].update({'upload_guide':'guides/'+Path(upload_guide).name,'upload_guide_sha256':report['upload_guide_sha256']})
            for extra in report.get('additional_references',[]):
                manifest['clips'][clip]['frames'][-1].setdefault('additional_references',[]).append({'file':'guides/'+Path(extra['file']).name,'sha256':extra['sha256']})
        phases=[int(Path(f['file']).stem.rsplit('-',1)[1]) for f in report['frames']]
        layout=dict(report.get('layout') or variable_layouts.get(clip,{}).get('layout',{}))
        if layout:
            if not upload_guide and len(phases)!=len(refs[clip]['frames']):
                layout['cell_indices']=phases;layout['fixed_translation']=report['whole_cycle_translation']
            sheet_transfers[key]={**refs[clip],'layout':layout,'frames':[{'src':'references/'+Path(f['reference']['src']).name,'sha256':f['reference']['sha256'],'blender_frame':refs[clip]['frames'][phase]['blender_frame']} for phase,f in zip(phases,report['frames'])]}
    for clip,record in manifest['clips'].items():
        record['frames'].sort(key=lambda f:f['phase']);assert len(record['frames'])==len(refs[clip]['frames'])
        source=Path(refs[clip]['blender_source'])
        if not source.is_absolute():source=ROOT/source
        target=OUT/'blender'/source.name
        if not target.exists() or sha(target)!=sha(source):shutil.copy2(source,target)
        record['blender']='blender/'+target.name
        record['blender_sha256']=sha(target)
        record['blender_action']='Idle' if record['action']=='idle' else 'Walking' if record['action']=='walk' else refs[clip].get('authored_action')
        guide=variable_layouts.get(clip,{}).get('guide')
        if guide:
            guide=ROOT/guide;target=OUT/'guides'/guide.name;shutil.copy2(guide,target)
            record['reference_guide']='guides/'+target.name
            record['reference_guide_sha256']=sha(target)
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2))
    native_refs={key:{**refs[key],'frames':[{'src':f['reference'],'sha256':f['reference_sha256'],'blender_frame':refs[key]['frames'][f['phase']]['blender_frame']} for f in record['frames']]} for key,record in manifest['clips'].items()}
    (OUT/'reference-manifest.json').write_text(json.dumps({'clips':native_refs},indent=2))
    (OUT/'transfer-manifest.json').write_text(json.dumps({'clips':native_refs},indent=2))
    variable_refs=sheet_transfers
    if variable_refs:(OUT/'transfer-manifest-v4.json').write_text(json.dumps({'clips':variable_refs},indent=2))
    (OUT/'approved-sheets.json').write_text(json.dumps(approved,indent=2))
    if (ROOT/'imagegen-prompts.json').exists():shutil.copy2(ROOT/'imagegen-prompts.json',OUT/'imagegen-prompts.json')
    print('Published',len(manifest['clips']),'clips',sum(len(c['frames']) for c in manifest['clips'].values()),'frames')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('approved_file');a=p.parse_args();package(json.loads(Path(a.approved_file).read_text()))
