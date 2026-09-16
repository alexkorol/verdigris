from PIL import Image,ImageDraw
from pathlib import Path
r=Path(__file__).resolve().parents[1]
out=Image.new('RGBA',(1024,8*116),(35,41,34,255));d=ImageDraw.Draw(out)
for row,(action,direction) in enumerate((a,b) for a in ('idle','walk') for b in ('front','right','back','left')):
    d.text((4,row*116),f'{action} {direction}',fill='white')
    for i in range(8):out.alpha_composite(Image.open(r/'candidates'/f'well-alpha-{action}-{direction}-{i:02}.png'),(i*128,row*116+20))
out.save(r/'well-alpha-native-review.png')
