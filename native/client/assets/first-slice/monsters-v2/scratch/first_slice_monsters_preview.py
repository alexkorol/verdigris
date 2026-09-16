from pathlib import Path
from PIL import Image,ImageDraw
p=Path('native/client/assets/first-slice/monsters-v2');im=Image.new('RGBA',(768,768),(36,42,35,255));d=ImageDraw.Draw(im)
for y,a in enumerate(('attack','hit','death')):
 for x,direction in enumerate(('front','right','back','left')):
  for k in range(4):
   f=Image.open(p/'references'/f'pack-wolf-{a}-{direction}-{k:02d}.png')
   im.alpha_composite(f, (x*192+(k%2)*96,y*256+(k//2)*96+32))
  d.text((x*192,y*256),a+' '+direction,fill='white')
im.save(p/'combat-refs-review.png')
