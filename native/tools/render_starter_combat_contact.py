"""Lossless reference packing and nearest-neighbor QA contacts (no art repaint)."""
from pathlib import Path
from PIL import Image,ImageDraw
import json
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat'
bad=[]
for sex in ['male','female']:
 for clip,n in [('idle',4),('attack',8),('hit',8),('death',8),('club-idle',4),('club-walk',8),('club-sprint',8),('unarmed-attack',8),('unarmed-hit',8),('unarmed-death',8)]:
  files=[ROOT/'references'/f'{sex}-{clip}-{a}-{i:02d}.png' for a in ['front','right','back','left'] for i in range(n)]
  if not all(p.exists() for p in files):continue
  size=Image.open(files[0]).width;contact=Image.new('RGBA',(n*size,4*size));checks=[]
  for j,a in enumerate(['front','right','back','left']):
   sheet=Image.new('RGBA',(4*size,2*size))
   for i in range(n):
    p=ROOT/'references'/f'{sex}-{clip}-{a}-{i:02d}.png';im=Image.open(p).convert('RGBA');box=im.getbbox();assert im.size==(size,size) and box
    if not (box[0]>0 and box[1]>0 and box[2]<size and box[3]<size):bad.append((p.name,box))
    contact.paste(im,(i*size,j*size));sheet.paste(im,((i%4)*size,(i//4)*size));checks.append({'frame':p.name,'bbox':box})
   (ROOT/'guides').mkdir(exist_ok=True)
   if size==128:
    for start in [0,4]:
     square=Image.new('RGBA',(256,256));cells=[]
     for k in range(4):
      p=ROOT/'references'/f'{sex}-{clip}-{a}-{start+k:02d}.png';square.paste(Image.open(p),((k%2)*128,(k//2)*128));cells.append({'row':k//2,'column':k%2,'phase':start+k,'reference':str(p.relative_to(ROOT)).replace('\\','/'),'nativeSize':[128,128],'nativeAnchor':[64,96]})
     guide=ROOT/'guides'/f'{sex}-{clip}-{a}-phases-{start}-{start+3}-4x.png';square.resize((1024,1024),Image.Resampling.NEAREST).save(guide);guide.with_suffix('.json').write_text(json.dumps({'sheetSize':[1024,1024],'scale':4,'cells':cells},indent=2))
   else:sheet.resize((size*16,size*8),Image.Resampling.NEAREST).save(ROOT/'guides'/f'{sex}-{clip}-{a}-4x.png')
  bg=Image.new('RGBA',contact.size,(36,40,44));bg.alpha_composite(contact);bg.resize((contact.width*2,contact.height*2),Image.Resampling.NEAREST).save(ROOT/f'{sex}-{clip}-contact.png')
  (ROOT/'provenance'/f'{sex}-{clip}-bounds.json').write_text(json.dumps(checks,indent=2))
assert not bad,('Frame edges crop sprites',bad)
