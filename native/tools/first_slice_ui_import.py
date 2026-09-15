"""Recover the fixed eightfold inventory sheet without resizing its pixels."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import sys

import numpy as np
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice/ui-art/accepted'
IDS = ['wooden_club', 'starter_flax_tunic_male', 'starter_flax_tunic_female', 'starter_woven_footwear']

def package_approved():
    correction=ROOT.parent/'club-correction'
    proof=json.loads((correction/'report.json').read_text())
    # This exact correction was reviewed by the root agent at native and 3x.
    expected='1866983440047e229bcad37f44b28c7970139354274af1c30c9f3eed4f21bced'
    if hashlib.sha256((correction/'wooden_club.png').read_bytes()).hexdigest()!=expected:
        raise ValueError('Unreviewed club correction cannot replace the accepted icon')
    proof['visual_acceptance']=True
    proof['visual_review']='Root agent viewed the corrected irregular branch club at native and nearest-neighbor 3x scale on 2026-09-15 and accepted it against the owner-approved exterior concept. In-game acceptance is separate.'
    shutil.copy2(correction/'wooden_club.png',ROOT/'icons/wooden_club.png')
    shutil.copy2(correction/'source.png',ROOT/'source/wooden-club-correction.png')
    shutil.copy2(correction/'prompt.txt',ROOT/'club-correction-prompt.txt')
    (ROOT/'review/club-correction.json').write_text(json.dumps(proof,indent=2))
    shutil.copy2(ROOT.parent/'references/club-correction-scale-8x.png',ROOT/'references/club-correction-scale-8x.png')
    manifest=json.loads((ROOT/'manifest.json').read_text())
    old={x['id']:x for x in manifest['items']}
    old['wooden_club']={'id':'wooden_club','file':'icons/wooden_club.png','bbox':proof['bbox'],
        'size':[48,96],'sha256':expected,'usage':'inventory only; not a world drop or held-weapon sprite'}
    manifest['items']=[old[x] for x in IDS]
    for item in manifest['items']:
        item['source']='source/wooden-club-correction.png' if item['id']=='wooden_club' else 'source/starter-inventory-sheet.png'
    manifest['club_correction']=proof
    manifest['visual_acceptance']='Root agent visually approved all four final inventory icons at native and nearest-neighbor 3x scale on 2026-09-15. In-game acceptance is separate.'
    manifest['prompt']='prompt.txt'
    manifest['club_correction_prompt']='club-correction-prompt.txt'
    manifest['references']=[{'file':'references/'+name,'role':role,
        'sha256':hashlib.sha256((ROOT/'references'/name).read_bytes()).hexdigest()} for name,role in [
        ('starter-player-exterior.png','Owner-approved starter appearance'),
        ('pixel-parity-8x.png','Eightfold native pixel-size example for the four-icon sheet'),
        ('club-correction-scale-8x.png','Eightfold framing and pixel-size guide; its rejected mace design must not be reused')]]
    preview=Image.new('RGBA',(768,480),(35,37,31,255))
    draw=ImageDraw.Draw(preview)
    clips=[]
    for i,item in enumerate(manifest['items']):
        frame=Image.open(ROOT/item['file'])
        if frame.mode!='RGBA' or list(frame.size)!=item['size'] or set(frame.getchannel('A').getdata())!={0,255}:
            raise ValueError('Invalid accepted icon: '+item['id'])
        if hashlib.sha256((ROOT/item['file']).read_bytes()).hexdigest()!=item['sha256']:
            raise ValueError('Changed accepted icon: '+item['id'])
        preview.alpha_composite(frame,(i*192+72,20))
        preview.alpha_composite(frame.resize((frame.width*3,frame.height*3),Image.Resampling.NEAREST),(i*192+24,150))
        draw.text((i*192+8,450),item['id'],fill=(224,213,194,255))
        clips.append({'identity':item['id'],'action':'icon','direction':'front','frames':[item['file']],
            'frame':item['size'],'anchor':[frame.width//2,frame.height//2],'pixels_per_metre':48,'fps':1,'loop':False})
    preview.save(ROOT/'review/native-and-3x.png')
    (ROOT/'manifest.json').write_text(json.dumps(manifest,indent=2))
    (ROOT/'install.json').write_text(json.dumps({'accepted':True,'clips':clips},indent=2))
    print('Packaged four reviewed inventory icons. Ready for separate native integration.')

def recover_club(source, respecter):
    sys.path.insert(0, respecter)
    from pixel_perfecter.reconstructor import PixelArtReconstructor
    source = Path(source)
    image = Image.open(source)
    if image.mode != 'RGBA' or image.width != image.height:
        raise ValueError('Expected transparent square canvas')
    pixels = np.array(image)
    zero_fraction = float(np.mean(pixels[:,:,3] == 0))
    if zero_fraction < .7:
        raise ValueError('Club source lacks generous real transparent canvas')
    pixels[:,:,3] = np.where(pixels[:,:,3] >= 128,255,0)
    pixels[pixels[:,:,3] == 0,:3] = 0
    rec = PixelArtReconstructor(image=pixels)
    rec.use_hough = False
    rec.run()
    grid=rec.cell_size
    if abs(grid - image.width/128) > .5 or rec.last_metrics.get('warnings'):
        raise ValueError('Recovered pixel scale no longer matches the coarse input example')
    selected = next((c for c in rec.last_fit_debug['candidates'] if c['size'] == grid),None)
    if selected is None or selected['gated_out']:
        raise ValueError('Authored coarse pixel grid rejected; regenerate instead of resizing')
    rec.cell_size = grid
    rec.offset = tuple(selected['offset'])
    recovered = Image.fromarray(rec._empirical_pixel_reconstruction())
    # The built-in tool may return 1254px instead of the requested 1024px
    # canvas. Detect actual blocks, then crop its canvas centre. Do not resize
    # the recovered pixels or centre on the foreground bounding box.
    x,y = (recovered.width-48)//2,(recovered.height-96)//2
    frame = recovered.crop((x,y,x+48,y+96))
    p=np.array(frame)
    p[:,:,3] = np.where(p[:,:,3] >=128,255,0)
    p[p[:,:,3] == 0,:3] = 0
    frame=Image.fromarray(p)
    if np.count_nonzero(np.array(recovered)[:,:,3] >=128) != np.count_nonzero(p[:,:,3]):
        raise ValueError('Fixed club crop would discard foreground')
    box=frame.getbbox()
    if not box or box[0] ==0 or box[1]==0 or box[2]>=48 or box[3]>=96:
        raise ValueError('Empty or clipped club')
    out=ROOT.parent/'club-correction'
    out.mkdir(parents=True,exist_ok=True)
    (out/'source.png').write_bytes(source.read_bytes())
    frame.save(out/'wooden_club.png')
    preview=Image.new('RGBA',(320,400),(35,37,31,255))
    preview.alpha_composite(frame,(16,16))
    preview.alpha_composite(frame.resize((144,288),Image.Resampling.NEAREST),(112,32))
    preview.save(out/'native-and-3x.png')
    report={'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'source_size':list(image.size),
            'requested_logical_canvas':[128,128],'recovered_canvas':list(recovered.size),
            'frame':[48,96],'canvas_centre_crop':[x,y,x+48,y+96],
            'pixel_grid':selected,'alpha_zero_fraction':zero_fraction,'bbox':box,
            'sha256':hashlib.sha256((out/'wooden_club.png').read_bytes()).hexdigest(),
            'method':'Built-in imagegen targeted edit. PixelRespecter detects coarse blocks and crops a fixed-size frame about the canvas centre. No resize, foreground recentering, or RGB keying.',
            'quality_metrics':rec.last_metrics,
            'visual_acceptance':False}
    (out/'report.json').write_text(json.dumps(report,indent=2))
    print(json.dumps(report,indent=2))

def recover(source, respecter):
    sys.path.insert(0, respecter)
    from pixel_perfecter.reconstructor import PixelArtReconstructor
    source = Path(source)
    image = Image.open(source)
    if image.mode != 'RGBA' or image.size != (1536, 1024):
        raise ValueError(f'Expected original RGBA 1536x1024; got {image.mode} {image.size}')
    pixels = np.array(image)
    alpha = pixels[:, :, 3]
    native_alpha_zero_fraction = float(np.mean(alpha == 0))
    if np.mean(alpha == 0) < .4 or alpha.max() < 250:
        raise ValueError('Source lacks required native transparent blank canvas')
    pixels[:, :, 3] = np.where(alpha >= 128, 255, 0)
    pixels[pixels[:, :, 3] == 0, :3] = 0
    rec = PixelArtReconstructor(image=pixels)
    rec.use_hough = False
    rec.run()
    fits = rec.last_fit_debug['candidates']
    selected = next((c for c in fits if c['size'] == 8), None)
    if selected is None or selected['gated_out']:
        raise ValueError('Intended eightfold pixel grid rejected; regenerate rather than resize')
    rec.cell_size = 8
    rec.offset = tuple(selected['offset'])
    recovered = Image.fromarray(rec._empirical_pixel_reconstruction())
    report = {'version': 1, 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
              'source_size': list(image.size), 'alpha_zero_fraction': native_alpha_zero_fraction,
              'native_frame': [48, 96], 'pixel_grid': selected, 'grid_candidates': fits,
              'processing': 'Pixel Respecter empirical reconstruction on fixed 8x grid. No arbitrary resize, per-item fitting, RGB keying, or interpolation.',
              'visual_acceptance': False,
              'alpha_policy': 'Original native alpha thresholded at 128; no RGB-based removal. Transparent pixels may retain brown RGB in the original but alpha is zero.',
              'generation': {'method': 'ChatGPT web image generation',
                             'conversation': 'https://chatgpt.com/g/g-p-68faa7735964819182de7eb32b19560d-pixel-art-and-game-dev/c/6aa8eecc-fa48-83ea-9f6e-5cb3e4d3f341',
                             'background': 'Transparent requested in prompt; actual original RGBA checked. No hidden parameter claim.'},
              'items': []}
    ox, oy = selected['offset']
    origin_x, origin_y = round(-ox / 8), round(-oy / 8)
    frames = []
    for i, asset in enumerate(IDS):
        slot = recovered.crop((origin_x+i*48, origin_y, origin_x+(i+1)*48, origin_y+128))
        frame = slot.crop((0,16,48,112))
        p = np.array(frame)
        p[:,:,3] = np.where(p[:,:,3] >= 128, 255, 0)
        p[p[:,:,3] == 0,:3] = 0
        frame = Image.fromarray(p)
        bbox = frame.getbbox()
        if not bbox or bbox[0] == 0 or bbox[1] == 0 or bbox[2] >= 48 or bbox[3] >= 96:
            raise ValueError(f'{asset}: empty or edge-clipped icon')
        if np.count_nonzero(np.array(slot)[:,:,3] >= 128) != np.count_nonzero(p[:,:,3]):
            raise ValueError(f'{asset}: fixed cell crop would lose visible pixels')
        if asset == 'starter_woven_footwear':
            # Shoes occupy one inventory cell. This is a fixed transparent-pad
            # crop, not an ink-dependent fit or a change to their pixel scale.
            square = frame.crop((0,48,48,96))
            if np.count_nonzero(np.array(square)[:,:,3]) != np.count_nonzero(p[:,:,3]):
                raise ValueError('Footwear is outside its authored square inventory cell')
            frame = square
            bbox = frame.getbbox()
        frames.append(frame)
        report['items'].append({'id': asset, 'file': 'icons/'+asset+'.png', 'bbox': bbox,
                               'size': list(frame.size), 'usage': 'inventory only; not a world drop or held-weapon sprite'})
    for folder in ['icons','source','review']:
        (ROOT / folder).mkdir(parents=True, exist_ok=True)
    (ROOT/'source/starter-inventory-sheet.png').write_bytes(source.read_bytes())
    recovered.save(ROOT/'review/recovered.png')
    preview = Image.new('RGBA',(768,480),(35,37,31,255))
    draw = ImageDraw.Draw(preview)
    for i,(frame,entry) in enumerate(zip(frames, report['items'])):
        target = ROOT/entry['file']
        frame.save(target)
        entry['sha256'] = hashlib.sha256(target.read_bytes()).hexdigest()
        preview.alpha_composite(frame,(i*192+72,20))
        preview.alpha_composite(frame.resize((frame.width*3,frame.height*3),Image.Resampling.NEAREST),(i*192+24,150))
        draw.text((i*192+8,450),entry['id'],fill=(224,213,194,255))
    preview.save(ROOT/'review/native-and-3x.png')
    (ROOT/'manifest.json').write_text(json.dumps(report,indent=2))
    print(json.dumps({'report': str(ROOT/'manifest.json'), 'pixel_grid':selected,'items':report['items']},indent=2))

if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source',nargs='?')
    parser.add_argument('--club',action='store_true')
    parser.add_argument('--package-approved',action='store_true')
    parser.add_argument('--respecter',default='Z:/Code/Python/pixel-perfecter')
    args=parser.parse_args()
    if args.package_approved:
        package_approved()
    else:
        if not args.source:
            parser.error('source is required for reconstruction')
        (recover_club if args.club else recover)(args.source,args.respecter)
