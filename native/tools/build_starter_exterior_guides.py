import json,sys
from pathlib import Path
from PIL import Image
r=Path('native/client/assets/first-slice/starter-v2')
if len(sys.argv)>1:r=Path(sys.argv[1])
for manifest in r.glob('*-references.json'):
 for clip,rec in json.loads(manifest.read_text())['clips'].items():
  sheet=Image.new('RGBA',(384,256))
  for i,f in enumerate(rec['frames']):sheet.paste(Image.open(r/f['src']),((i%4)*96,(i//4)*128+16))
  sheet.resize((1536,1024),Image.Resampling.NEAREST).save(r/'guides'/f'{clip}.png')
