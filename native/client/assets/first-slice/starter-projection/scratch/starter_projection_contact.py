"""Inspect the proof at native scale and nearest-neighbor zoom; do not alter assets."""
from PIL import Image,ImageDraw
from pathlib import Path
import numpy as np,json
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice';OUT=ROOT/'starter-projection'
import sys
if '--refine' in sys.argv:OUT=OUT/('v3' if '--semantic' in sys.argv else 'v2')
if '--v4' in sys.argv:OUT=ROOT/'starter-projection/v4'
if '--v5' in sys.argv:OUT=ROOT/'starter-projection/v5'
gait='sprint' if '--sprint' in sys.argv else 'walk'
dirs=['front','right','back','left'];sex='male';cell=96
contact=Image.new('RGBA',(8*cell,8*(cell+16)),(37,43,40,255));draw=ImageDraw.Draw(contact);metrics=[]
for row,d in enumerate(dirs):
 for k in range(8):
  f=f'{sex}-{gait}-{d}-{k:02d}.png';original=Image.open(ROOT/'starter-v2/references'/f).convert('RGBA');proof=Image.open(OUT/'frames'/f).convert('RGBA')
  contact.alpha_composite(original,(k*cell,row*2*(cell+16)+16));contact.alpha_composite(proof,(k*cell,(row*2+1)*(cell+16)+16))
  a=np.array(original)[:,:,3]>128;b=np.array(proof)[:,:,3]>128
  metrics.append({'frame':f,'geometry_alpha_iou':float((a&b).sum()/(a|b).sum()),'dimensions':list(proof.size),'bbox':proof.getbbox()})
 draw.text((2,row*2*(cell+16)),d+' original Blender',fill='white');draw.text((2,(row*2+1)*(cell+16)),d+' projected paint',fill='white')
contact.save(OUT/f'male-{gait}-contact-native.png');contact.resize((contact.width*2,contact.height*2),Image.Resampling.NEAREST).save(OUT/f'male-{gait}-contact-2x.png')
(OUT/f'reports/male-{gait}-frame-comparison.json').write_text(json.dumps(metrics,indent=2))
print('minimum geometry IoU',min(v['geometry_alpha_iou'] for v in metrics))

cycles=[]
for k in range(8):
 frame=Image.new('RGBA',(192,192),(37,43,40,255))
 for j,d in enumerate(dirs):
  im=Image.open(OUT/'frames'/f'{sex}-{gait}-{d}-{k:02d}.png').convert('RGBA');frame.alpha_composite(im,((j%2)*96,(j//2)*96))
 cycles.append(frame.convert('RGB').resize((576,576),Image.Resampling.NEAREST))
cycles[0].save(OUT/f'male-{gait}-four-directions.gif',save_all=True,append_images=cycles[1:],duration=100,loop=0,disposal=2)
