"""Audit complete projected cohorts and make native attack timing review panels."""
from pathlib import Path
from PIL import Image,ImageDraw
import json,hashlib,numpy as np,sys
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-projection'
results={}
cohorts=sys.argv[1:3] if len(sys.argv)>2 else ['male-v6','female-v3']
for sex,cohort in zip(['male','female'],cohorts):
 root=ROOT/cohort;pack=root/'candidate-binary';manifest=json.loads((pack/'manifest.json').read_text());clips=manifest['clips']
 assert len(clips)==48 and sum(len(c['frames']) for c in clips.values())==416
 atlases={c['appearance_atlas_sha256'] for c in clips.values()};assert len(atlases)==1
 filters={(c.get('raster_pass') or {}).get('filter_type') for c in clips.values()};assert len(filters)==1
 braid={c['intentional_braid_geometry_change'].get('local_profile_sha256') for c in clips.values()}
 assert len(braid)==1
 checks=[];frames=0
 for key,c in clips.items():
  assert c['anchor']==([48,80] if c['frame_size']==[96,96] else [64,96])
  if 'attack' in c['action']:
   assert c['attack_timing']['contact_index']==8 and len(c['frames'])==16
  if c['equipment']=='branch-club':
   wood=c['weapon_appearance'];assert wood['patch_sha256']==hashlib.sha256((ROOT/'weapon-source'/wood['patch']).read_bytes()).hexdigest()
  else:assert c['weapon_appearance'] is None
  for f in c['frames']:
   p=pack/f['src'];assert hashlib.sha256(p.read_bytes()).hexdigest()==f['sha256']
   a=np.array(Image.open(p).convert('RGBA'));assert set(np.unique(a[:,:,3])).issubset({0,255});assert not a[a[:,:,3]==0,:3].any();frames+=1
 for equipment,prefix,idle in [('branch-club','club-attack','club-idle'),('unarmed','unarmed-attack','idle')]:
  contact=Image.new('RGBA',(96+4*128,4*(128+18)),(37,43,40,255));draw=ImageDraw.Draw(contact)
  for j,d in enumerate(['front','right','back','left']):
   resting=np.array(Image.open(pack/f'{sex}-{idle}-{d}-00.png'))
   draw.text((2,j*146),f'{sex} {prefix} {d}: idle | start | contact8 | recover9 | end15',fill='white')
   contact.alpha_composite(Image.fromarray(resting),(0,j*146+18+16))
   for x,k in enumerate([0,8,9,15]):
    p=pack/f'{sex}-{prefix}-{d}-{k:02d}.png';im=Image.open(p);contact.alpha_composite(im,(96+x*128,j*146+18))
   first=np.array(Image.open(pack/f'{sex}-{prefix}-{d}-00.png'))[16:112,16:112]
   last=np.array(Image.open(pack/f'{sex}-{prefix}-{d}-15.png'))[16:112,16:112]
   a=resting[:,:,3]>0;b=first[:,:,3]>0
   iou=float((a&b).sum()/(a|b).sum());both=a&b
   checks.append({'action':prefix,'direction':d,'start_end_exact_pixels':bool(np.array_equal(first,last)),'idle_alpha_iou':iou,'idle_rgb_mae_visible':float(np.abs(first[:,:,:3].astype(float)-resting[:,:,:3])[both].mean())})
   assert iou>.96
  contact.save(pack/f'{sex}-{prefix}-timing-native.png')
 results[sex]={'cohort':cohort,'frames':frames,'clips':len(clips),'filter_type':next(iter(filters)),'body_atlas_sha256':next(iter(atlases)),'braid_profile_sha256':next(iter(braid)),'endpoint_checks':checks,'manifest_sha256':hashlib.sha256((pack/'manifest.json').read_bytes()).hexdigest()}
(ROOT/('-'.join(cohorts)+'-audit.json')).write_text(json.dumps(results,indent=2),encoding='utf-8');print(json.dumps(results,indent=2))
