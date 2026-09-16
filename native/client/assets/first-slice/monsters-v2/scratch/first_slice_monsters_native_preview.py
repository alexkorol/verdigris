from pathlib import Path
from PIL import Image,ImageDraw
p=Path('native/client/assets/first-slice/monsters-v2');im=Image.new('RGBA',(800,864),(36,42,35,255));d=ImageDraw.Draw(im)
for row,(a,dr) in enumerate((a,dr) for a in ('idle','walk') for dr in ('front','right','back','left')):
 d.text((0,row*108),a+' '+dr,fill='white')
 for k in range(8):im.alpha_composite(Image.open(p/'candidates'/f'pack-wolf-{a}-{dr}-{k:02d}.png'),(k*100,row*108+12))
im.save(p/'pack-wolf-native-review.png')
