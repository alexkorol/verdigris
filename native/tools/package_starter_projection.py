"""Package a coherent projected appearance for review or the native art importer.

The --reviewed flag records completed visual review; it does not bypass the
runtime's required action/direction/equipment cohort completeness checks.
"""
import argparse,hashlib,json,shutil
from pathlib import Path
from PIL import Image
import numpy as np
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def package(source,output,reviewed=False):
 data=json.loads((source/'manifest.json').read_text());assert data['clips'],'Empty appearance cohort';output.mkdir(parents=True,exist_ok=True);(output/'frames').mkdir(exist_ok=True)
 shutil.copy2(source/'manifest.json',output/'source-manifest.json')
 result={'accepted':reviewed,'status':'Visually reviewed partial projection cohort' if reviewed else 'Candidate awaiting visual review','technique':data['technique'],'source_manifest':'source-manifest.json','source_manifest_sha256':sha(source/'manifest.json'),'clips':[],'provenance':{},'coverage':{}}
 atlas_by_sex={}
 for key,clip in data['clips'].items():
  pieces=key.split('-');sex=pieces[0];direction=pieces[-1];action=pieces[-2];equipment='club' if 'club' in pieces else 'unarmed';identity=f'player_{sex}_{equipment}'
  assert direction in ['front','right','back','left'];assert action in ['idle','walk','sprint','attack','hit','death']
  atlas=clip['appearance_atlas_sha256'];assert sex not in atlas_by_sex or atlas_by_sex[sex]==atlas,'Mixed appearance sources within a sex';atlas_by_sex[sex]=atlas
  size=clip['frame_size'];anchor=clip['anchor'];assert (size,anchor) in [([96,96],[48,80]),([128,128],[64,96])]
  frames=[]
  for entry in clip['frames']:
   src=source/entry['src'];assert sha(src)==entry['sha256'];im=Image.open(src);a=np.array(im);assert im.mode=='RGBA' and list(im.size)==size
   assert set(np.unique(a[:,:,3]))=={0,255} and not a[a[:,:,3]==0,:3].any();box=im.getbbox();assert box and min(box[:2])>0 and box[2]<size[0] and box[3]<size[1]
   dst=output/'frames'/src.name;shutil.copy2(src,dst);frames.append('frames/'+src.name)
  fps={'idle':6,'walk':10,'sprint':14,'attack':20,'hit':12,'death':10}[action]
  result['clips'].append({'identity':identity,'action':action,'direction':direction,'frames':frames,'frame':size,'anchor':anchor,'pixels_per_metre':48,'fps':fps,'loop':action in ['idle','walk','sprint']})
  result['provenance'][key]={'source_clip_metadata':{k:v for k,v in clip.items() if k!='frames'},'geometry_sha256':clip['geometry_sha256'],'appearance_atlas_sha256':atlas,'source_frame_entries':clip['frames'],'frame_sha256':[sha(output/f) for f in frames]}
  result['coverage'].setdefault(identity,[]).append(action+'/'+direction)
 result['missing']={identity:[a+'/'+d for a in ['idle','walk','sprint','attack','hit','death'] for d in ['front','right','back','left'] if a+'/'+d not in entries] for identity,entries in result['coverage'].items()}
 result['complete']=all(not m for m in result['missing'].values()) and all(f'player_{sex}_{e}' in result['coverage'] for sex in atlas_by_sex for e in ['unarmed','club'])
 (output/'manifest.json').write_text(json.dumps(result,indent=2));print(json.dumps({'frames':sum(len(c['frames']) for c in result['clips']),'clips':len(result['clips']),'complete':result['complete'],'reviewed':reviewed,'manifest':str(output/'manifest.json')}))
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('source',type=Path);p.add_argument('output',type=Path);p.add_argument('--reviewed',action='store_true');a=p.parse_args();package(a.source,a.output,a.reviewed)
