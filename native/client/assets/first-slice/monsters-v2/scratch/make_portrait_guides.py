from pathlib import Path
from PIL import Image
import json
root=Path('native/client/assets/first-slice/monsters-v2');m=json.loads((root/'reference-manifest.json').read_text());groups={};out=root/'guides-v4';out.mkdir(exist_ok=True)
for key,c in m['clips'].items():
 if c['actor']!='well-alpha' or c['action'] not in ('idle','walk'):continue
 sheet=Image.new('RGBA',(256,384))
 for i,f in enumerate(c['frames']):sheet.paste(Image.open(root/f['src']),((i%2)*128,(i//2)*96))
 sheet.resize((1024,1536),Image.Resampling.NEAREST).save(out/f'{key}.png')
 groups[key]={**c,'layout':{'columns':2,'slot':[128,96],'inset':[0,0],'scale':4,'output':[1024,1536]}}
(root/'transfer-manifest-v4.json').write_text(json.dumps({'clips':groups},indent=2))
p=Path('native/tools/first_slice_monsters_transfer_v3.py');s=p.read_text().replace('transfer-manifest-v3.json','transfer-manifest-v4.json').replace("im.size!=(1536,1024)","im.size!=tuple(layout['output'])")
s=s.replace("preview=Image.new('RGBA',(w*8,384)","preview=Image.new('RGBA',(w*8,384)")
Path('native/tools/first_slice_monsters_transfer_v4.py').write_text(s)
