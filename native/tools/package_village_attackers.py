"""Freeze minimal enemy clips after review; never resize rendered pixels."""
import argparse,hashlib,json
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw

def main():
    p=argparse.ArgumentParser();p.add_argument('folder',type=Path);p.add_argument('--reviewed',action='store_true');p.add_argument('--npcs',action='store_true');args=p.parse_args();root=args.folder
    manifest={'accepted':args.reviewed,'scope':'Village-defense prologue: human invader and breaker','technique':'Distinct editable MakeHuman anatomy/hair variants; reviewed CMU/Quaternius motion; Blender guides -> ImageGen appearance -> Pixel Respecter -> depth-tested vertex projection -> native Blender rendering. No generated animation poses.','clips':[],'provenance':{}}
    identities=('defender','scribe') if args.npcs else ('village-invader','village-leader')
    actions=('idle',) if args.npcs else ('idle','walk','attack','hit','death')
    if args.npcs:manifest['scope']='Village-defense prologue: defender and well keeper static cardinal views'
    for identity in identities:
        folder=root/identity;rows=[];animation=[]
        for action in actions:
            report=json.loads((folder/'reports'/f'{action}.json').read_text());size=report['frame'][0]
            count=len(report['source_frames']);contact=Image.new('RGBA',(128*count,128*4),(38,42,38,255))
            for di,direction in enumerate(('front','right','back','left')):
                frames=[]
                for phase in range(count):
                    src=folder/'frames'/f'{identity}-{action}-{direction}-{phase:02d}.png';a=np.array(Image.open(src).convert('RGBA'))
                    a[:,:,3]=np.where(a[:,:,3]>=128,255,0);a[a[:,:,3]==0,:3]=0
                    im=Image.fromarray(a);box=im.getbbox()
                    assert im.size==tuple(report['frame']) and box and box[0]>0 and box[1]>0 and box[2]<size and box[3]<size,(src,box)
                    target=folder/'accepted'/src.name;target.parent.mkdir(exist_ok=True);im.save(target)
                    contact.alpha_composite(im,(phase*128+(128-size)//2,di*128+(128-size)//2))
                    frames.append(str(target.relative_to(root)).replace('\\','/'))
                manifest['clips'].append({'identity':identity,'action':action,'direction':direction,'fps':{'idle':1,'walk':6,'attack':10,'hit':8,'death':8}[action],'loop':action in ('idle','walk'),'frame':report['frame'],'anchor':report['anchor'],'pixels_per_metre':48,'frames':frames})
            contact.save(folder/f'{action}-contact-native.png');contact.resize((contact.width*2,contact.height*2),Image.Resampling.NEAREST).save(folder/f'{action}-contact-2x.png')
            rows.append(contact)
            for phase in range(count):
                sheet=Image.new('RGBA',(128*4,128),(38,42,38,255))
                for di in range(4):sheet.paste(contact.crop((phase*128,di*128,(phase+1)*128,(di+1)*128)),(di*128,0))
                animation.append(sheet.resize((1024,256),Image.Resampling.NEAREST).convert('RGB'))
        animation[0].save(folder/'motions.gif',save_all=True,append_images=animation[1:],duration=160,loop=0)
        overview=Image.new('RGBA',(768,640),(38,42,38,255))
        for row,contact in enumerate(rows):
            # Front view strips at native size; full four-view contacts alongside.
            overview.paste(contact.crop((0,0,contact.width,128)),(0,row*128))
        overview.resize((1536,1280),Image.Resampling.NEAREST).save(folder/'overview-2x.png')
        manifest['provenance'][identity]={'transfer':json.loads((folder/'transfer.json').read_text()),'actions':{a:json.loads((folder/'reports'/f'{a}.json').read_text()) for a in actions}}
    assert len(manifest['clips'])==(8 if args.npcs else 40)
    manifest['frame_sha256']={f:hashlib.sha256((root/f).read_bytes()).hexdigest() for c in manifest['clips'] for f in c['frames']}
    (root/'manifest.json').write_text(json.dumps(manifest,indent=2));print(json.dumps({'clips':len(manifest['clips']),'frames':len(manifest['frame_sha256']),'accepted':args.reviewed}))
if __name__=='__main__':main()
