"""Four-pose 2x2 guides: 192x128 slots at4x preserve a128x96 boss canvas."""
from pathlib import Path
import json
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/monsters-v2'
m=json.loads((ROOT/'reference-manifest.json').read_text());groups={};out=ROOT/'guides-v3';out.mkdir(exist_ok=True)
for name,record in m['clips'].items():
    if record['actor']!='well-alpha':continue
    for part in range((len(record['frames'])+3)//4):
        frames=record['frames'][part*4:part*4+4];key=f'{name}-part{part}';sheet=Image.new('RGBA',(384,256))
        for i,f in enumerate(frames):sheet.paste(Image.open(ROOT/f['src']),((i%2)*192+32,(i//2)*128+16))
        sheet.resize((1536,1024),Image.Resampling.NEAREST).save(out/f'{key}.png')
        groups[key]={**record,'frames':frames,'parent_clip':name,'part':part,'layout':{'columns':2,'slot':[192,128],'inset':[32,16],'scale':4}}
(ROOT/'transfer-manifest-v3.json').write_text(json.dumps({'clips':groups},indent=2))
