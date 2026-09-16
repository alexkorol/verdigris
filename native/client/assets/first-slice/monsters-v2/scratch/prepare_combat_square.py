import json
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
refs=json.loads((root/'reference-manifest.json').read_text())
trans=json.loads((root/'transfer-manifest-v4.json').read_text())
for key,r in refs['clips'].items():
    if r['action'] not in ('attack','hit'):continue
    w,h=r['frame'];x=(192-w)//2;y=(128-h)//2
    guide=Image.new('RGBA',(384,256))
    for i,f in enumerate(r['frames']):guide.alpha_composite(Image.open(root/f['src']),((i%2)*192+x,(i//2)*128+y))
    guide.resize((1536,1024),Image.Resampling.NEAREST).save(root/'guides-v4'/f'{key}.png')
    trans['clips'][key]={**r,'layout':{'columns':2,'slot':[192,128],'inset':[x,y],'scale':4,'output':[1536,1024]}}
death=json.loads((root/'death-v2/transfer-manifest-v4.json').read_text())
trans['clips'].update(death['clips'])
for key,r in death['clips'].items():refs['clips'][key]={**r,'blender_source':r.get('blender_source',r.get('blender'))}
for key,r in refs['clips'].items():
    if r['actor']=='well-alpha' and r['action']!='death':r['blender_source']=f'blender/well-alpha-{r["action"]}-canvas128.blend'
(root/'transfer-manifest-v4.json').write_text(json.dumps(trans,indent=2))
(root/'reference-manifest.json').write_text(json.dumps(refs,indent=2))
