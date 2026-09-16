"""Diagnostic contacts for rejected death candidates. Never installs artwork."""
from pathlib import Path
import argparse,json,sys,hashlib,shutil
import numpy as np
from PIL import Image,ImageDraw
sys.path.insert(0,'Z:/Code/Python/pixel-perfecter')
from pixel_perfecter.reconstructor import PixelArtReconstructor

def cohort(root,actor='pack-wolf',direction='left'):
    base=root.parents[1];key=f'{actor}-death-{direction}';legacy=actor=='pack-wolf' and direction=='left';w=96 if actor=='pack-wolf' else 128
    out=root/'single-cohort' if legacy else root/'singles'/key
    if legacy:shutil.copytree(root/'attempts/pack-wolf-death-left/v03-single',out/'phase-01',dirs_exist_ok=True)
    contact=Image.new('RGBA',(w*12,540),(36,42,35,255));draw=ImageDraw.Draw(contact);records=[];anim=[]
    for n in range(4):
        folder=out/f'phase-{n:02d}';frame=Image.open(folder/'fixed-placement.png')
        ref=Image.open(root/'death-v2'/f'{key}-{n:02d}.png')
        guide=base/'projection-proof/v3-faces'/f'pack-wolf-death-left-phase{n}-single-guide.png' if legacy else base/'death-v2/single-guides'/f'{key}-{n:02d}.png'
        report=json.loads((folder/'reconstruction.json').read_text())
        contact.alpha_composite(ref,(n*w*3+w,8));contact.alpha_composite(frame,(n*w*3+w,108))
        contact.alpha_composite(frame.resize((w*3,288),Image.Resampling.NEAREST),(n*w*3,230))
        draw.text((n*w*3+8,518),f'Phase {n}: fixed IoU '+format(report['fixed_placement_iou'],'.3f'),fill=(255,230,170,255))
        records.append({'phase':n,'frame':f'phase-{n:02d}/fixed-placement.png',
            'sha256':hashlib.sha256((folder/'fixed-placement.png').read_bytes()).hexdigest(),
            'source':f'phase-{n:02d}/source.png','source_sha256':report['source_sha256'],
            'prompt':f'phase-{n:02d}/prompt.txt','guide':str(guide),'guide_sha256':hashlib.sha256(guide.read_bytes()).hexdigest(),
            'fixed_iou':report['fixed_placement_iou'],'diagnostic_translation':report['diagnostic_only_translation']})
        canvas=Image.new('RGBA',(w*3,288),(36,42,35,255));canvas.alpha_composite(frame.resize((w*3,288),Image.Resampling.NEAREST));anim.append(canvas)
    contact.save(out/f'{actor}-{direction}-native-and-3x.png')
    anim[0].save(out/f'{actor}-{direction}-animation.gif',save_all=True,append_images=anim[1:],duration=[220,180,180,1300],loop=0,disposal=2)
    (out/'cohort.json').write_text(json.dumps({'accepted':False,'clip':key,'frame':[w,96],
        'anchor':[48,76] if actor=='pack-wolf' else [64,64],'pixels_per_metre':48,'alignment':'authored position unchanged; no per-frame translation','frames':records},indent=2))
    print('Cohort ready:',out/f'{actor}-{direction}-native-and-3x.png')

def single(root,raw,phase=1,actor='pack-wolf',direction='left'):
    original=Path(raw);image=Image.open(original)
    if image.mode!='RGBA' or image.size!=(1536,1024):raise ValueError('Expected original RGBA1536x1024')
    a=np.array(image);zero=float(np.mean(a[:,:,3]==0));a[:,:,3]=np.where(a[:,:,3]>=128,255,0);a[a[:,:,3]==0,:3]=0
    rec=PixelArtReconstructor(image=a);rec.use_hough=False;rec.run();detected=rec.cell_size
    fit=next(c for c in rec.last_fit_debug['candidates'] if c['size']==8)
    rec.cell_size=8;rec.offset=tuple(fit['offset']);recovered=Image.fromarray(rec._empirical_pixel_reconstruction())
    w=96 if actor=='pack-wolf' else 128;inset=48 if actor=='pack-wolf' else 32;key=f'{actor}-death-{direction}'
    ox,oy=fit['offset'];x=round(-ox/8)+inset;y=round(-oy/8)+16
    refpath=root/f'death-v2/{key}-{phase:02d}.png';ref=Image.open(refpath).convert('RGBA');mask=np.array(ref)[:,:,3]>=128
    def frame_at(dx,dy):return recovered.crop((x-dx,y-dy,x+w-dx,y+96-dy))
    def iou(dx,dy):
        b=np.array(frame_at(dx,dy))[:,:,3]>=128
        return float(np.count_nonzero(b&mask)/max(1,np.count_nonzero(b|mask)))
    best,dx,dy=max((iou(dx,dy),dx,dy) for dy in range(-12,13) for dx in range(-12,13))
    fixed=frame_at(0,0);p=np.array(fixed);p[:,:,3]=np.where(p[:,:,3]>=128,255,0);p[p[:,:,3]==0,:3]=0;fixed=Image.fromarray(p)
    legacy=actor=='pack-wolf' and direction=='left'
    out=root/('attempts/pack-wolf-death-left/v03-single' if phase==1 else f'single-cohort/phase-{phase:02d}') if legacy else root/f'singles/{key}/phase-{phase:02d}'
    out.mkdir(parents=True,exist_ok=True)
    (out/'source.png').write_bytes(original.read_bytes());fixed.save(out/'fixed-placement.png')
    contact=Image.new('RGBA',(w*6+64,450),(36,42,35,255));draw=ImageDraw.Draw(contact)
    contact.alpha_composite(ref,(24,16));contact.alpha_composite(fixed,(184,16))
    contact.alpha_composite(ref.resize((w*3,288),Image.Resampling.NEAREST),(12,125));contact.alpha_composite(fixed.resize((w*3,288),Image.Resampling.NEAREST),(w*3+44,125))
    draw.text((10,425),'reference / generated at AUTHORED PLACEMENT (no recentering)',fill=(255,230,170,255));contact.save(out/'diagnostic-contact.png')
    report={'accepted':False,'source_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),
        'reference_sha256':hashlib.sha256(refpath.read_bytes()).hexdigest(),'source_dimensions':list(image.size),'alpha_zero_fraction':zero,
        'detected_preferred_grid':detected,'selected_authored_grid':fit,'fixed_placement_iou':iou(0,0),
        'best_registration_iou':best,'diagnostic_only_translation':[dx,dy],
        'rule':'Best-registration score is diagnostic only; no individual animation-frame recentering is authorized.'}
    (out/'reconstruction.json').write_text(json.dumps(report,indent=2));print(json.dumps(report))

def review(root,key):
    m=json.loads((root/'transfer-manifest-v4.json').read_text())['clips'][key]
    source=root/'source'/f'{key}.png'
    raw=np.array(Image.open(source))
    raw[:,:,3]=np.where(raw[:,:,3]>=128,255,0);raw[raw[:,:,3]==0,:3]=0
    rec=PixelArtReconstructor(image=raw);rec.use_hough=False;rec.run()
    layout=m['layout'];scale=layout['scale'];w,h=m['frame'];sw,sh=layout['slot'];ix,iy=layout['inset']
    fit=next(c for c in rec.last_fit_debug['candidates'] if c['size']==scale)
    rec.cell_size=scale;rec.offset=tuple(fit['offset'])
    recovered=Image.fromarray(rec._empirical_pixel_reconstruction())
    ox,oy=fit['offset'];slots=[];refs=[]
    for i,ref in enumerate(m['frames']):
        x=round(-ox/scale)+(i%layout['columns'])*sw;y=round(-oy/scale)+(i//layout['columns'])*sh+layout.get('row_offset',0)
        slots.append(recovered.crop((x,y,x+sw,y+sh)))
        refs.append(Image.open(root/ref['src']).convert('RGBA'))
    def score(dx,dy):
        values=[]
        for slot,ref in zip(slots,refs):
            a=np.array(slot.crop((ix-dx,iy-dy,ix+w-dx,iy+h-dy)))[:,:,3]>=128
            b=np.array(ref)[:,:,3]>=128
            values.append(float(np.count_nonzero(a&b)/max(1,np.count_nonzero(a|b))))
        return values
    _,dx,dy=max((float(np.mean(score(dx,dy))),dx,dy) for dy in range(-12,13) for dx in range(-12,13))
    values=score(dx,dy)
    contact=Image.new('RGBA',(w*8,480),(36,42,35,255));draw=ImageDraw.Draw(contact)
    for i,(slot,ref) in enumerate(zip(slots,refs)):
        frame=slot.crop((ix-dx,iy-dy,ix+w-dx,iy+h-dy))
        contact.alpha_composite(ref,(i*w*2+int(w/2),12))
        contact.alpha_composite(frame,(i*w*2+int(w/2),112))
        contact.alpha_composite(frame.resize((w*2,h*2),Image.Resampling.NEAREST),(i*w*2,230))
        draw.text((i*w*2+6,432),f'phase {i}, IoU {values[i]:.3f}',fill=(255,230,170,255))
    draw.text((5,460),'DIAGNOSTIC ONLY: reference / candidate native / candidate 2x',fill=(255,230,170,255))
    folder=root/'diagnostics';folder.mkdir(exist_ok=True)
    contact.save(folder/f'{key}.png')
    report={'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'accepted':False,'grid':fit,
        'whole_cycle_translation':[dx,dy],'mean_iou':float(np.mean(values)),'per_phase_iou':values}
    (folder/f'{key}.json').write_text(json.dumps(report,indent=2));print(json.dumps(report))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('root',type=Path);p.add_argument('key',nargs='?');p.add_argument('--single',action='store_true');p.add_argument('--cohort',action='store_true');p.add_argument('--phase',type=int,default=1);p.add_argument('--actor',default='pack-wolf');p.add_argument('--direction',default='left');a=p.parse_args()
    if a.cohort:cohort(a.root,a.actor,a.direction)
    elif a.single:single(a.root,a.key,a.phase,a.actor,a.direction)
    else:review(a.root,a.key)
