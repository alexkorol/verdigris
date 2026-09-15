"""Package visually reviewed starter exterior clips with exact source provenance."""
import json,hashlib,shutil
from pathlib import Path
from PIL import Image
import numpy as np
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice';WORK=ROOT/'starter-v2';OUT=WORK/'accepted'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def package():
 for d in ['frames','references','originals','reports','blender','previews','generation','guides']:(OUT/d).mkdir(parents=True,exist_ok=True)
 result={'accepted':True,'status':'Three visually reviewed unarmed walk clips; not a complete playable player family','appearance_reference_sha256':digest(ROOT/'concept/starter-player-exterior.png'),'clips':[],'provenance':{},'not_included':['complete cardinal locomotion','club loadout','combat','hit','death','native cohort activation']}
 for clip in ['male-walk-front','male-walk-back','female-walk-front']:
  sex,gait,direction=clip.split('-');refroot=ROOT/('starter-v2-female-refine' if sex=='female' else 'starter-v2')
  report=json.loads((WORK/'review'/f'{clip}.json').read_text());raw=WORK/'source'/f'{clip}.png'
  assert digest(raw)==report['source_sha256']
  assert report['registered_mean_iou']>=.70
  files=[];paint_centers=[];ref_centers=[]
  for entry in report['frames']:
   src=WORK/'candidates'/entry['file'];ref=refroot/entry['reference']['src']
   assert digest(src)==entry['sha256'];assert digest(ref)==entry['reference']['sha256'];assert entry['silhouette_iou']>=.65
   im=Image.open(src);alpha=np.array(im)[:,:,3];assert im.size==(96,96) and im.mode=='RGBA' and set(np.unique(alpha))=={0,255}
   for centers,data in [(paint_centers,alpha),(ref_centers,np.array(Image.open(ref))[:,:,3])]:
    y,x=np.nonzero(data>=128);centers.append([float(x.mean()),float(y.mean())])
   shutil.copy2(src,OUT/'frames'/src.name);shutil.copy2(ref,OUT/'references'/src.name);files.append('frames/'+src.name)
   entry['packaged_reference']='references/'+src.name
  paint_centers=np.array(paint_centers);ref_centers=np.array(ref_centers)
  transitions=[]
  for i in range(len(files)):
   j=(i+1)%len(files);err=(paint_centers[j]-paint_centers[i])-(ref_centers[j]-ref_centers[i]);transitions.append({'from':i,'to':j,'centroid_motion_error_px':err.tolist(),'magnitude_px':float(np.linalg.norm(err))})
  assert max(t['magnitude_px'] for t in transitions)<2.5,'New animation displacement not explained by source motion'
  report['loop_transition_checks']=transitions
  report['metric_definitions']={'registration_iou':'Paint alpha >=128 versus Blender reference alpha >=128; used to estimate shared translation.','silhouette_iou':'Binary recovered paint alpha >0 versus Blender reference alpha >0, including antialiased edge coverage. This broader reference mask explains higher final overlap scores.','reference_integrity':'Every compared source reference hash is checked before copying; packaged_reference points at byte-identical copy.'}
  report['reference_sha256']={entry['packaged_reference']:digest(OUT/entry['packaged_reference']) for entry in report['frames']}
  report['visual_acceptance']='Parent and player agents inspected native/2x contact including row3->4 and loop7->0; no obvious clipping or artificial displacement. Owner acceptance not claimed.'
  report['alpha_verification']='Composited on opaque background before review. Hidden RGB under zero alpha is not visible haze.'
  (OUT/'reports'/f'{clip}.json').write_text(json.dumps(report,indent=2))
  shutil.copy2(raw,OUT/'originals'/raw.name);shutil.copy2(WORK/'review'/f'{clip}-2x.png',OUT/'previews'/f'{clip}-2x.png')
  blend=refroot/'blender'/f'{sex}-{gait}-exterior.blend';shutil.copy2(blend,OUT/'blender'/blend.name)
  frames=[]
  for i in range(8):
   b=Image.new('RGBA',(192,96),(35,44,31,255));b.alpha_composite(Image.open(OUT/'references'/Path(files[i]).name),(0,0));b.alpha_composite(Image.open(OUT/files[i]),(96,0));frames.append(b.resize((576,288),Image.Resampling.NEAREST).convert('RGB'))
  frames[0].save(OUT/'previews'/f'{clip}-reference-vs-paint.gif',save_all=True,append_images=frames[1:],duration=100,loop=0,disposal=2)
  result['clips'].append({'identity':f'player_{sex}_unarmed','action':gait,'direction':direction,'frames':files,'frame':[96,96],'anchor':[48,80],'pixels_per_metre':48,'fps':10,'loop':True})
  callname={'male-walk-front':'call-126.js','female-walk-front':'call-456.js','male-walk-back':'male-web-exact-call.js'}[clip]
  callsource=WORK/'generation-calls'/callname;calltarget=OUT/'generation'/f'{clip}-exact-call.js'
  if callsource.exists():shutil.copy2(callsource,calltarget)
  assert calltarget.exists(),'Exact archived generation call required'
  guide=refroot/'guides'/f'{clip}.png';guidetarget=OUT/'guides'/guide.name;shutil.copy2(guide,guidetarget)
  result['provenance'][clip]={'guide':'guides/'+guidetarget.name,'guide_sha256':digest(guidetarget),'exact_generation_call':'generation/'+calltarget.name,'exact_generation_call_sha256':digest(calltarget),'original':'originals/'+raw.name,'original_sha256':digest(raw),'blender':'blender/'+blend.name,'blender_sha256':digest(blend),'report':'reports/'+clip+'.json','registration':report.get('registrations'),'source_reference_folder':str(refroot),'frame_sha256':[digest(OUT/f) for f in files]}
 (OUT/'manifest.json').write_text(json.dumps(result,indent=2))
 (OUT/'README.md').write_text('These 24 painted frames are an accepted partial art milestone: male walk front/back and female walk front. They match the newer coarse-flax starter exterior and the refined long female braid. They are not a complete player family. Native integration must keep the cohort disabled until its action/direction/equipment contract is complete.\n\nAll frames are 96x96 RGBA, anchor [48,80], 48px/metre. Source pixels are reconstructed on the inferred 4x lattice without per-frame resizing or recentering. Female source row2 was uniformly 8px too high: a single recorded integer row translation restores it, and transitions across the row boundary and loop are measured against the exact Blender poses. Hidden RGB in zero-alpha regions is sanitized by reconstruction; transparent originals must be inspected by alpha-compositing them.\n')
 print(json.dumps({'accepted_frames':sum(len(c['frames']) for c in result['clips']),'manifest':str(OUT/'manifest.json'),'max_motion_errors':{k:max(x['magnitude_px'] for x in json.loads((OUT/'reports'/f'{k}.json').read_text())['loop_transition_checks']) for k in result['provenance']}}))
if __name__=='__main__':package()
