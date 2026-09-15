"""Compose native monster references on exact four-pixel imagegen grids.

No sprite fitting, resampling, or per-frame centering occurs. Authored canvas
dimensions and anchor positions remain unchanged inside the padded slots.
"""
import argparse,hashlib,json
from pathlib import Path
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/monsters-v2'

def build(root,selected=None):
    records=json.loads((root/'reference-manifest.json').read_text())['clips']
    output=root/'guides-v4';output.mkdir(exist_ok=True)
    result={}
    for key,record in records.items():
        if selected and key not in selected:continue
        width,height=record['frame'];count=len(record['frames'])
        if count==8 and width==128:
            columns,slot,inset,size=2,[128,96],[0,0],[1024,1536]
        elif count==8 and width==96:
            columns,slot,inset,size=4,[96,128],[0,16],[1536,1024]
        elif count==4:
            columns,slot,inset,size=2,[192,128],[(192-width)//2,(128-height)//2],[1536,1024]
        else:raise ValueError(f'Unsupported authored frame layout: {key}')
        canvas=Image.new('RGBA',(size[0]//4,size[1]//4))
        for index,frame in enumerate(record['frames']):
            source=root/frame['src'];assert hashlib.sha256(source.read_bytes()).hexdigest()==frame['sha256']
            image=Image.open(source);assert image.size==(width,height) and image.mode=='RGBA'
            canvas.alpha_composite(image,((index%columns)*slot[0]+inset[0],(index//columns)*slot[1]+inset[1]))
        target=output/f'{key}.png'
        canvas.resize(tuple(size),Image.Resampling.NEAREST).save(target)
        result[key]={**record,'layout':{'columns':columns,'slot':slot,'inset':inset,'scale':4,'output':size},'guide':str(target.relative_to(root)).replace('\\','/'),'guide_sha256':hashlib.sha256(target.read_bytes()).hexdigest()}
    manifest=root/'transfer-manifest-v4.json'
    previous=json.loads(manifest.read_text())['clips'] if selected and manifest.exists() else {}
    previous.update(result);manifest.write_text(json.dumps({'clips':previous},indent=2))
    print(f'Built {len(result)} exact-grid guides')

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--asset-root',type=Path,default=ROOT);parser.add_argument('--clip',action='append');args=parser.parse_args();build(args.asset_root,args.clip)
