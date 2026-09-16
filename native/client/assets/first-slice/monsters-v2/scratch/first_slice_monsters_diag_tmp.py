from PIL import Image
from pathlib import Path
p=Path('native/client/assets/first-slice/monsters-v2')
r=Image.open(p/'review/well-alpha-walk-right-recovered.png')
for i in range(8):
 a=r.crop(((i%4)*128,(i//4)*160,(i%4)*128+128,(i//4)*160+160))
 b=Image.open(p/'references'/f'well-alpha-walk-right-{i:02d}.png')
 print(i,a.size,a.getbbox(),b.getbbox())
