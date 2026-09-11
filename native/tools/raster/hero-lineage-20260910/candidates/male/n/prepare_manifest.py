from pathlib import Path
from PIL import Image
import numpy as np, json, hashlib, shutil

BASE=Path(r"C:/Users/Alex/Documents/ChatGPT/verdigris-fable-renderer/native/tools/raster/hero-lineage-20260910/candidates/male")
ORIGINAL=Path(r"C:/Users/Alex/Downloads/ChatGPT Image Sep 10, 2026, 05_22_21 PM.png")
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
selected={
'n':{'idle':'idle','walk0':'walk0','walk1':'walk1','strike0':'strike0-repair','strike1':'strike1'},
'ne':{'idle':'idle','walk0':'walk1-geometry-repair','walk1':'walk1-female-geometry-repair','strike0':'strike0-repair','strike1':'strike1-left-repair'},
'e':{'idle':'idle','walk0':'walk0','walk1':'walk1-repair','strike0':'strike0','strike1':'strike1'}}
archive={'n/strike0':'strike0-rejected-scale','ne/walk0':'walk0-rejected-initial','ne/walk1':'walk1-rejected-extra-arm','ne/strike0':'strike0-rejected-scale','ne/strike1':'strike1-rejected-repeated-anticipation','e/walk1':'walk1-rejected-repeated-phase'}
for direction in selected:
 for p in (BASE/direction).glob('*.tool-output.json'):
  data=json.loads(p.read_text(encoding='utf-8-sig'))
  refs=data.get('references',[])
  data['reference_hashes_before_alias_selection']=[{'path':r,'sha256':sha(r)} for r in refs if isinstance(r,str) and Path(r).is_file()]
  data['metadata_note']='Raw built-in image bytes are unchanged. Reference hashes captured before selecting repaired aliases.'
  p.write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8')
guide=BASE.parent/'female/ne/walk1.png'
shutil.copy2(guide,BASE/'ne/reference-female-ne-walk1.png')
for direction, poses in selected.items():
 for pose, source in poses.items():
  if pose==source: continue
  folder=BASE/direction
  for suffix in ['.png','.prompt.txt','.tool-output.json']:
   current=folder/(pose+suffix)
   backup=folder/(archive[direction+'/'+pose]+suffix)
   if current.exists() and not backup.exists(): shutil.copy2(current,backup)
   shutil.copy2(folder/(source+suffix),current)
wrist={
'n':{'idle':([464,596],True,96),'walk0':([462,595],True,97),'walk1':([520,420],False,None),'strike0':([437,265],True,-59),'strike1':([521,151],True,-89)},
'ne':{'idle':([505,565],True,80),'walk0':([550,398],False,None),'walk1':([469,571],True,101),'strike0':([479,267],True,-56),'strike1':([579,89],True,-62)},
'e':{'idle':([671,632],False,None),'walk0':([501,580],True,98),'walk1':([778,486],True,6),'strike0':([730,362],True,-78),'strike1':([954,322],True,0)}}
notes={
'n':{
'idle':'Full north back; face hidden. Anatomical left wrist visible at screen-left. Standing body936px vs original970 (-3.5%).',
'walk0':'Left knee shortened/forward at screen-left; right calf trails down at screen-right with visible sole. Left arm back, right arm forward.',
'walk1':'Opposite: right knee shortened/forward at screen-right, left calf trails down at screen-left with sole visible. Left forearm is occluded beyond torso.',
'strike0':'Selected focused repair preserves left bent arm and reduces initial scale drift. Head-to-feet966px vs idle936 (+3.2%); feet26px lower. Initial1008px attempt rejected.',
'strike1':'Correct anatomical left arm extends up/away; right arm down. Hair-to-feet approximately935px, matching idle. High northward reach.'},
'ne':{
'idle':'Back three-quarter upper-right; right arm near screen-right and left arm far screen-left.',
'walk0':'Selected geometry repair initially named walk1. Short forward right knee at screen-right and long trailing left calf lower-left; left arm forward/occluded, right arm down.',
'walk1':'Cross-character geometry edit now produces short forward far foot upper-left and long trailing right calf/sole lower-right, with right arm forward and left arm down. Clear opposite silhouette to selected walk0. Male short hair/build restored; somewhat longer tunic/belt ends and cloth texture remain from guide. Head size similar; body extent differs due stride/foreshortening. Parent should review native cycle.',
'strike0':'Focused scale repair: correct far left bent arm; near right arm down; body939px vs idle934 (+0.5%).',
'strike1':'Selected second correction retains left shoulder attachment with arm extended high above head. Right arm down. Contact is a high reach, more overhead than a low forward swing; equipment review caveat. Head/feet match idle, raised fist extends above head.'},
'e':{
'idle':'Right-facing profile. Anatomical left arm is far and occluded; visible foreground hanging arm is anatomical right. Left wrist location is inferred, not measured.',
'walk0':'Open right-facing stride with near right arm forward and far left arm back. Tunic hides exact hip connection; paired support should be reviewed at native scale.',
'walk1':'Selected repair reverses near/right arm backward and far/left arm forward. Front thigh now intended near/right; hip ownership remains partly occluded. First repeated pose rejected.',
'strike0':'Far anatomical-left fist cocked in front of upper chest; near/right arm unchanged down. Body size matches idle.',
'strike1':'Far anatomical-left arm extended right, partly occluded at shoulder; near/right arm stays down. Planted feet and body scale match idle.'}}
for direction, poses in selected.items():
 folder=BASE/direction
 frames=[]
 for pose, original_stem in poses.items():
  p=folder/(pose+'.png'); im=Image.open(p); rgb=np.asarray(im.convert('RGB')).astype(np.int16)
  mask=(rgb.max(2)-rgb.min(2)>15)&(rgb.mean(2)<225)
  yy,xx=np.where(mask); bbox=[int(xx.min()),int(yy.min()),int(xx.max()+1),int(yy.max()+1)]
  alpha=np.asarray(im.getchannel('A')) if 'A' in im.getbands() else np.full((im.height,im.width),255,np.uint8)
  xy,visible,angle=wrist[direction][pose]
  record={'pose':pose,'file':str(p),'sha256':sha(p),'selected_raw_stem':original_stem,
    'prompt':str(folder/(pose+'.prompt.txt')),'tool_output':str(folder/(pose+'.tool-output.json')),
    'size':[im.width,im.height],'mode':im.mode,
    'alpha':{'has_channel':'A' in im.getbands(),'min':int(alpha.min()),'max':int(alpha.max()),'zero_pixels':int((alpha==0).sum()),'partial_pixels':int(((alpha>0)&(alpha<255)).sum()),'verdict':'FAIL: opaque painted checkerboard; raw unchanged' if 'A' not in im.getbands() else 'Inspect alpha visually'},
    'estimated_foreground_bbox':bbox,'bbox_method':'Warm/saturated foreground mask: RGB max-min>15 and mean<225; diagnostic estimate, NOT alpha and not used to edit pixels',
    'estimated_body_extent_px':bbox[3]-bbox[1],
    'left_wrist':{'source_xy':xy,'normalized_xy':[round(xy[0]/im.width,5),round(xy[1]/im.height,5)],'visible':visible,'confidence':'medium approximate visual measurement' if visible else 'low; occluded and inferred','forearm_direction_deg':angle},
    'visual_review':notes[direction][pose]}
  frames.append(record)
 manifest={'schema_version':1,'owner':'bestiary_assets','character':'male','direction':direction,'status':'source candidates selected; pending parent native reconstruction and game review',
  'original_identity':{'path':str(ORIGINAL),'sha256':sha(ORIGINAL),'size':[1254,1254],'alpha128_bbox':[432,128,822,1098],'standing_height_px':970},
  'canvas_authority':'Original1254square source; raw generation never fitted/rescaled/repainted.',
  'requested_common_source_pivot':[627,1098],
  'source_pivot_caveat':'Requested anchor is not proof of registration. Body and foot offsets in review require common-scale parent reconstruction; do not independently fit each silhouette.',
  'left_wrist_convention':'Anatomical LEFT. Source x right and y down. Normalized by1254. Forearm direction is elbow-to-wrist screen angle (0right,+90down), NOT a verified weapon angle; hands are empty. Occluded wrists are estimates.',
  'workflow':{'type':'Reference-preserving isolated single-pose built-in edits; original identity always retained. Idle direction normally added; successful opposite-pose geometry used for targeted repairs.',
   'sources':['https://x.com/elle_elle_e/status/2097439673308389840','https://www.flixly.ai/blog/stop-motion-chatgpt-images-2-5','https://x.com/higgsfield/status/2097514684811554911'],
   'adaptation':'Our controlled one-character direction/motion adaptation of separate-frame and explicit-reference-role workflows; not a claim these creators tested this character or produced these outputs. Tool exposes no selectable model variant.'},
  'frames':frames,'retained_attempts':[p.name for p in folder.glob('*.png') if p.stem not in poses],
  'production_changes':'None: no runtime/catalog/code/build/Git edits.'}
 (folder/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
 deps=[]
 for p in sorted(folder.iterdir()):
  if p.is_file() and p.name not in ['dependencies.json','prepare_manifest.py']:
   deps.append({'file':str(p),'sha256':sha(p),'bytes':p.stat().st_size})
 (folder/'dependencies.json').write_text(json.dumps({'direction':direction,'selected_files_frozen':True,'files':deps},indent=2)+'\n',encoding='utf-8')
 print(direction,[(f['pose'],f['estimated_foreground_bbox'],f['mode']) for f in frames])
