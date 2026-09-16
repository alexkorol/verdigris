from PIL import Image,ImageDraw
from pathlib import Path
import numpy as np,json
r=Path('native/client/assets/first-slice');out=r/'starter-projection';im=Image.open(r/'starter-v2/originals/player-unarmed-idle.png').convert('RGBA');dirs=['front','right','back','left'];slots=[im.crop((d*384,0,(d+1)*384,512)).resize((96,128),Image.Resampling.NEAREST) for d in range(4)];refs=[Image.open(r/f'starter-combat/references/male-idle-{d}-00.png').convert('RGBA') for d in dirs];best=(-1,0,0)
for dy in range(-12,13):
 for dx in range(-12,13):
  scores=[]
  for slot,ref in zip(slots,refs):
   a=np.array(slot.crop((-dx,16-dy,96-dx,112-dy)))[:,:,3]>=128;b=np.array(ref)[:,:,3]>=128;scores.append((a&b).sum()/(a|b).sum())
  score=float(np.mean(scores))
  if score>best[0]:best=(score,dx,dy)
print(best)
canvas=Image.new('RGBA',(384,288),(40,45,42,255));draw=ImageDraw.Draw(canvas)
for d,(slot,ref) in enumerate(zip(slots,refs)):
 canvas.alpha_composite(ref,(d*96,0));canvas.alpha_composite(slot.crop((-best[1],16-best[2],96-best[1],112-best[2])),(d*96,96))
 a=np.array(ref);b=np.array(slot.crop((-best[1],16-best[2],96-best[1],112-best[2])));over=np.zeros_like(a);over[:,:,0]=a[:,:,3];over[:,:,1]=b[:,:,3];over[:,:,3]=255;canvas.alpha_composite(Image.fromarray(over),(d*96,192))
canvas.resize((1536,1152),Image.Resampling.NEAREST).save(out/'registration-contact.png');(out/'registration.json').write_text(json.dumps({'mean_iou':best[0],'translation':[best[1],best[2]]}))
