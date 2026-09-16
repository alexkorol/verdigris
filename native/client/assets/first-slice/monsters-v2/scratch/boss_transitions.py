from PIL import Image,ImageDraw
from pathlib import Path
r=Path(__file__).resolve().parents[1];frames=[]
for action in ('idle','walk'):
  for i in range(8):
    canvas=Image.new('RGBA',(640,160),(35,41,34,255));d=ImageDraw.Draw(canvas)
    for col,direction in enumerate(('front','right','back','left')):
      d.text((col*160+4,4),f'{direction} {action}',fill='white')
      canvas.alpha_composite(Image.open(r/'candidates'/f'well-alpha-{action}-{direction}-{i:02}.png'),(col*160+16,48))
    frames.append(canvas.resize((1280,320),Image.Resampling.NEAREST))
frames[0].save(r/'well-alpha-idle-walk-transition.gif',save_all=True,append_images=frames[1:],duration=120,loop=0,disposal=2)
pair=Image.new('RGBA',(1024,480),(35,41,34,255));d=ImageDraw.Draw(pair)
for row,direction in enumerate(('front','right','back','left')):
  for col,action in enumerate(('idle','walk')):
    d.text((col*512+4,row*120),f'{direction} {action}',fill='white')
    for i,phase in enumerate((0,4)):
      im=Image.open(r/'candidates'/f'well-alpha-{action}-{direction}-{phase:02}.png')
      pair.alpha_composite(im.resize((192,144),Image.Resampling.NEAREST),(col*512+i*256,row*120+12))
pair.save(r/'well-alpha-transition-pairs.png')
