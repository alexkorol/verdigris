"""Strict Pixel Respecter transfer with per-actor fixed canvas geometry."""
import argparse,hashlib,json,sys
from pathlib import Path
import numpy as np
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/monsters-v2'
sys.path.insert(0,'Z:/Code/Python/pixel-perfecter')
from pixel_perfecter.reconstructor import PixelArtReconstructor

def recover(raw,clip):
    m=json.loads((ROOT/'transfer-manifest-v4.json').read_text())['clips'][clip];refs=m['frames'];w,h=m['frame']; layout=m['layout']; scale=layout['scale']; columns=layout['columns'];slot_w,slot_h=layout['slot'];inset_x,inset=layout['inset'];row_offset=layout.get('row_offset',0)
    for folder in ('source','candidates','review'):(ROOT/folder).mkdir(exist_ok=True)
    raw=Path(raw);im=Image.open(raw)
    if im.mode!='RGBA' or im.size!=tuple(layout['output']):raise ValueError(f'Expected RGBA {layout["output"]}, got {im.mode} {im.size}')
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
        cell_index=layout.get('cell_indices',list(range(len(refs))))[i]
        x=round(-ox/scale)+(cell_index%columns)*slot_w;y=round(-oy/scale)+(cell_index//columns)*slot_h+row_offset
        slots.append(recovered.crop((x,y,x+slot_w,y+slot_h)))
        masks.append(np.array(Image.open(p).convert('RGBA'))[:,:,3]>=128)
    best=(-1,0,0)
    fixed=layout.get('fixed_translation')
    shifts_y=[fixed[1]] if fixed is not None else range(-12,13)
    shifts_x=[fixed[0]] if fixed is not None else range(-12,13)
    for dy in shifts_y:
        for dx in shifts_x:
            score=0
            for cell,mask in zip(slots,masks):
                b=np.array(cell.crop((inset_x-dx,inset-dy,inset_x+w-dx,inset+h-dy)))[:,:,3]>=128
                score+=np.count_nonzero(b&mask)/max(1,np.count_nonzero(b|mask))
            score/=len(slots)
            if score>best[0]:best=(score,dx,dy)
    report={'clip':clip,'source_sha256':hashlib.sha256(raw.read_bytes()).hexdigest(),'selected_grid':fit,'frame':m['frame'],'anchor':m['anchor'],'whole_cycle_translation':list(best[1:]),'registered_mean_iou':best[0],'alpha_policy':'Native alpha threshold128, noRGBkey','visual_acceptance':False,'frames':[]}
    if best[0]<.70:
        (ROOT/'review'/f'{clip}-rejected.json').write_text(json.dumps(report,indent=2));raise ValueError(f'Whole-sheet IoU{best[0]:.3f} fails.70')
    _,dx,dy=best;pending=[];preview=Image.new('RGBA',(w*8,384),(36,42,35,255))
    for i,(ref,slot,mask) in enumerate(zip(refs,slots,masks)):
        frame=slot.crop((inset_x-dx,inset-dy,inset_x+w-dx,inset+h-dy));a=np.array(frame);a[:,:,3]=np.where(a[:,:,3]>=128,255,0);a[a[:,:,3]==0,:3]=0;frame=Image.fromarray(a);box=frame.getbbox();b=a[:,:,3]>0
        iou=np.count_nonzero(b&mask)/max(1,np.count_nonzero(b|mask))
        minimum_iou=.70 if m['action'] in ('attack','hit','death') else .65
        if iou<minimum_iou:raise ValueError(f'{clip} frame{i} IoU{iou:.3f} fails {minimum_iou}')
        if not box or box[0]<=0 or box[1]<=0 or box[2]>=w or box[3]>=h:raise ValueError(f'Empty/clipped{clip} frame{i}{box}')
        if np.count_nonzero(np.array(slot)[:,:,3]>=128)!=np.count_nonzero(b):raise ValueError('Crop would discardforeground')
        name=Path(ref['src']).name;pending.append((frame,ROOT/'candidates'/name))
        report['frames'].append({'file':name,'reference':ref,'bbox':box,'silhouette_iou':float(iou)})
        preview.alpha_composite(frame.resize((w*2,h*2),Image.Resampling.NEAREST),((i%4)*w*2,(i//4)*192))
    for (frame,p),entry in zip(pending,report['frames']):frame.save(p);entry['sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    preview.save(ROOT/'review'/f'{clip}-2x.png');(ROOT/'review'/f'{clip}.json').write_text(json.dumps(report,indent=2))
    print(clip,'OK',round(best[0],3),[round(f['silhouette_iou'],3) for f in report['frames']])
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--asset-root',type=Path);p.add_argument('--raw');p.add_argument('--clip');a=p.parse_args()
    if a.asset_root:ROOT=a.asset_root
    recover(a.raw,a.clip)
