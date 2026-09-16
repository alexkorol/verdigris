"""Measure directional surface-color continuity and source geometry stability."""
from pathlib import Path
from PIL import Image
import numpy as np,json,sys
args=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else sys.argv[1:]
sex=args[0] if args else 'male';cohort=args[1] if len(args)>1 else 'male-v7'
assert sex in ['male','female']
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-projection'/cohort;directions=['front','right','back','left'];stats={};comparisons=[]
for gait in ['walk','sprint']:
 stats[gait]={}
 for d in directions:
  samples={'cloth':[],'skin':[],'hair':[]}
  for k in [0,4]:
   pixels=np.array(Image.open(ROOT/f'frames/{sex}-{gait}-{d}-{k:02d}.png').convert('RGBA'));mask=np.array(Image.open(ROOT/f'masks/{sex}-{gait}-{d}-{k:02d}.png').convert('RGBA'))
   for material,channel in [('cloth',0),('skin',1),('hair',2)]:
    indices=[i for i in range(3) if i!=channel];good=(mask[:,:,channel]>230)&(mask[:,:,indices[0]]<30)&(mask[:,:,indices[1]]<30)&(pixels[:,:,3]>230);samples[material].extend(pixels[:,:,:3][good])
  stats[gait][d]={key:np.median(value,axis=0).tolist() for key,value in samples.items() if len(value)}
 for d in ['right','left']:
  for material in ['cloth','skin']:
   a=np.array(stats[gait]['front'][material]);b=np.array(stats[gait][d][material]);luma=np.array([.2126,.7152,.0722]);ca=a/max(a.sum(),1);cb=b/max(b.sum(),1)
   comparisons.append({'gait':gait,'views':['front',d],'material':material,'median_rgb_delta':(b-a).tolist(),'luminance_delta_8bit':round(float((b-a)@luma),2),'normalized_chromaticity_distance':round(float(np.linalg.norm(ca-cb)),4)})
report={'method':'Median source-painted RGB from interior semantic mesh pixels; measured phases0 and4 in each direction. Baked directional lighting naturally changes luminance. Chromaticity separates hue change from illumination.','surface_medians_rgb':stats,'comparisons':comparisons,'geometry':{}}
for gait in ['walk','sprint']:
 r=json.loads((ROOT/f'reports/{sex}-{gait}-projection.json').read_text());report['geometry'][gait]={'before':r['geometry_before'],'after':r['geometry_after'],'exact_match':r['geometry_before']==r['geometry_after']}
(ROOT/'reports/color-continuity.json').write_text(json.dumps(report,indent=2));print(json.dumps(comparisons,indent=2))
