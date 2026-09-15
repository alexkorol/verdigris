"""Strict Pixel Respecter transfer with per-actor fixed canvas geometry."""
import argparse,hashlib,json,sys
from pathlib import Path
import numpy as np
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/monsters-v2'
sys.path.insert(0,'Z:/Code/Python/pixel-perfecter')
from pixel_perfecter.reconstructor import PixelArtReconstructor

def guides():
    m=json.loads((ROOT/'reference-manifest.json').read_text())
    for actor in ('pack-wolf','well-alpha'):
        for direction in ('front','right','back','left'):
            if f'{actor}-attack-{direction}' not in m['clips']:continue
            a=m['clips'][f'{actor}-attack-{direction}'];b=m['clips'][f'{actor}-hit-{direction}']
            m['clips'][f'{actor}-combat-{direction}']={**a,'action':'combat','frames':a['frames']+b['frames']}
    (ROOT/'transfer-manifest.json').write_text(json.dumps(m,indent=2))
    (ROOT/'guides').mkdir(exist_ok=True)
    for clip,data in m['clips'].items():
        w,h=data['frame']; scale=3 if w==128 else 4; slot_h=160 if w==128 else 128; inset=32 if w==128 else 16; sheet=Image.new('RGBA',(w*4,341 if w==128 else 256))
        for i,f in enumerate(data['frames']):sheet.paste(Image.open(ROOT/f['src']),((i%4)*w,(i//4)*slot_h+inset))
        canvas=Image.new('RGBA',(1536,1024));canvas.paste(sheet.resize((w*4*scale,sheet.height*scale),Image.Resampling.NEAREST),(0,0));canvas.save(ROOT/'guides'/f'{clip}.png')

def recover(raw,clip):
    m=json.loads((ROOT/'transfer-manifest.json').read_text())['clips'][clip];refs=m['frames'];w,h=m['frame']; scale=3 if w==128 else 4; slot_h=160 if w==128 else 128; inset=32 if w==128 else 16
    for folder in ('source','candidates','review'):(ROOT/folder).mkdir(exist_ok=True)
    raw=Path(raw);im=Image.open(raw)
    if im.mode!='RGBA' or im.size!=(1536,1024):raise ValueError(f'Expected RGBA1536x1024, got{im.mode}{im.size}')
    a=np.array(im);alpha=a[:,:,3]
    if alpha.min()!=0 or alpha.max()<250:raise ValueError('Missing genuine transparent/opaque alpha')
    (ROOT/'source'/f'{clip}.png').write_bytes(raw.read_bytes())
    a[:,:,3]=np.where(alpha>=128,255,0);a[a[:,:,3]==0,:3]=0
    rec=PixelArtReconstructor(image=a);rec.use_hough=False;rec.run()
    fit=next(c for c in rec.last_fit_debug['candidates'] if c['size']==scale)
    if fit['gated_out']:raise ValueError('Authored pixel grid fails variance gate')
    rec.cell_size=scale;rec.offset=tuple(fit['offset']);recovered=Image.fromarray(rec._empirical_pixel_reconstruction())
    recovered.save(ROOT/'review'/f'{clip}-recovered.png');ox,oy=fit['offset'];slots=[];masks=[]
    for i,ref in enumerate(refs):
        p=ROOT/ref['src'];assert hashlib.sha256(p.read_bytes()).hexdigest()==ref['sha256']
        x=round(-ox/scale)+(i%4)*w;y=round(-oy/scale)+(i//4)*slot_h
        slots.append(recovered.crop((x,y,x+w,y+slot_h)))
        masks.append(np.array(Image.open(p).convert('RGBA'))[:,:,3]>=128)
    best=(-1,0,0)
    for dy in range(-12,13):
        for dx in range(-12,13):
            score=0
            for cell,mask in zip(slots,masks):
                b=np.array(cell.crop((-dx,inset-dy,w-dx,inset+h-dy)))[:,:,3]>=128
                score+=np.count_nonzero(b&mask)/max(1,np.count_nonzero(b|mask))
            score/=len(slots)
            if score>best[0]:best=(score,dx,dy)
    report={'clip':clip,'source_sha256':hashlib.sha256(raw.read_bytes()).hexdigest(),'selected_grid':fit,'frame':m['frame'],'anchor':m['anchor'],'whole_cycle_translation':list(best[1:]),'registered_mean_iou':best[0],'alpha_policy':'Native alpha threshold128, noRGBkey','visual_acceptance':False,'frames':[]}
    if best[0]<.70:
        (ROOT/'review'/f'{clip}-rejected.json').write_text(json.dumps(report,indent=2));raise ValueError(f'Whole-sheet IoU{best[0]:.3f} fails.70')
    _,dx,dy=best;pending=[];preview=Image.new('RGBA',(w*8,384),(36,42,35,255))
    for i,(ref,slot,mask) in enumerate(zip(refs,slots,masks)):
        frame=slot.crop((-dx,inset-dy,w-dx,inset+h-dy));a=np.array(frame);a[:,:,3]=np.where(a[:,:,3]>=128,255,0);a[a[:,:,3]==0,:3]=0;frame=Image.fromarray(a);box=frame.getbbox();b=a[:,:,3]>0
        iou=np.count_nonzero(b&mask)/max(1,np.count_nonzero(b|mask))
        if iou<.65:raise ValueError(f'{clip} frame{i} IoU{iou:.3f} fails.65')
        if not box or box[0]<=0 or box[1]<=0 or box[2]>=w or box[3]>=h:raise ValueError(f'Empty/clipped{clip} frame{i}{box}')
        if np.count_nonzero(np.array(slot)[:,:,3]>=128)!=np.count_nonzero(b):raise ValueError('Crop would discardforeground')
        name=Path(ref['src']).name;pending.append((frame,ROOT/'candidates'/name))
        report['frames'].append({'file':name,'reference':ref,'bbox':box,'silhouette_iou':float(iou)})
        preview.alpha_composite(frame.resize((w*2,h*2),Image.Resampling.NEAREST),((i%4)*w*2,(i//4)*192))
    for (frame,p),entry in zip(pending,report['frames']):frame.save(p);entry['sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    preview.save(ROOT/'review'/f'{clip}-2x.png');(ROOT/'review'/f'{clip}.json').write_text(json.dumps(report,indent=2))
    print(clip,'OK',round(best[0],3),[round(f['silhouette_iou'],3) for f in report['frames']])
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--asset-root',type=Path);p.add_argument('--guides',action='store_true');p.add_argument('--raw');p.add_argument('--clip');a=p.parse_args()
    if a.asset_root:ROOT=a.asset_root
    if a.guides:guides()
    else:recover(a.raw,a.clip)
