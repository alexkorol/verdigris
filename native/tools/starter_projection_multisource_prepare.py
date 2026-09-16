from pathlib import Path
from PIL import Image
import json
r=Path('native/client/assets/first-slice');o=r/'starter-projection/v4';o.mkdir(exist_ok=True)
atlas=Image.new('RGBA',(1536,1024));raw=Image.open(r/'starter-v2/originals/player-unarmed-idle.png').convert('RGBA');dx,dy=json.loads((r/'starter-projection/registration.json').read_text())['translation']
for d,direction in enumerate(['front','right','back','left']):
 if d in [0,2]:frame=Image.open(r/f'starter-v2/candidates/male-walk-{direction}-00.png').convert('RGBA').resize((384,384),Image.Resampling.NEAREST)
 else:frame=raw.crop((d*384-dx*4,64-dy*4,(d+1)*384-dx*4,448-dy*4))
 atlas.paste(frame,(d*384,64))
atlas.save(o/'appearance-atlas.png')
