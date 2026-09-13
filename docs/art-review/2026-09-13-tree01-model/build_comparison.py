"""Owner-requested logical pixel normalization and integer NN review sheets."""
import hashlib
import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont, ImageOps
import numpy as np
ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[2]
font = ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf', 17)
small = ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf', 14)
title = ImageFont.truetype('C:/Windows/Fonts/segoeuib.ttf', 23)
hero_path = REPO / 'native/client/assets/raster/runtime/hero_se.png'
hero_full = Image.open(hero_path).convert('RGBA')
hero = hero_full.crop(hero_full.getbbox())
assert hero.height == 63
hashes = {}
for kind in ['original','revised']:
    raw = Image.open(ROOT / (kind+'-logical-raw.png')).convert('RGBA')
    # Structure guide only: discrete grayscale and binary alpha, no resizing.
    alpha = raw.getchannel('A').point(lambda p:255 if p >= 128 else 0)
    gray = ImageOps.grayscale(raw).point(lambda p:round(p/17)*17)
    logical = Image.merge('RGBA',(gray,gray,gray,alpha))
    logical.save(ROOT / (kind+'-logical.png'))
    if kind == 'revised':
        reference = logical.resize((logical.width*4,logical.height*4),Image.Resampling.NEAREST)
        reference.save(ROOT/'preappearance-reference-4x.png')
        assert reference.resize(logical.size,Image.Resampling.NEAREST).tobytes() == logical.tobytes()
        blocks = np.asarray(reference).reshape(logical.height,4,logical.width,4,4)
        assert np.all(blocks == np.asarray(logical)[:,None,:,None,:])
        assert set(alpha.getdata()) <= {0,255}

def board(size, heading, sub):
    im = Image.new('RGB',size,(28,32,34))
    d=ImageDraw.Draw(im)
    d.text((24,15),heading,font=title,fill=(236,232,215))
    d.text((24,50),sub,font=small,fill=(181,190,187))
    return im,d

im,d=board((1050,650),'Tree 01 | model-stage review | UNAPPROVED',
           'Recovered scene; same camera, lighting and framing. Geometry study, before appearance generation.')
for x,name,label in [(28,'original-model','Original Blender guide'),(288,'revised-model','Revised crown + twig structure'),(548,'revised-branches','Revised branches, foliage hidden')]:
    img=Image.open(ROOT/(name+'.png')).convert('RGBA').resize((240,480),Image.Resampling.NEAREST)
    im.paste(img,(x,110),img)
    d.text((x,80),label,font=small,fill=(235,231,213))
old=Image.open(ROOT/'baseline/tree-01-dense.png').convert('RGBA')
old.thumbnail((205,480),Image.Resampling.NEAREST)
im.paste(old,(818,110),old)
d.text((818,80),'Existing appearance',font=small,fill=(235,231,213))
d.text((24,615),'Trunk and original main branches retained. Secondary forks and leaf planes replace solid ellipsoids.',font=small,fill=(181,190,187))
im.save(ROOT/'model-comparison.png')

im,d=board((900,675),'Tree 01 | logical pixels and player scale | UNAPPROVED',
           '136 x 272 canvas; existing hero shown at 63 visible pixels. Adult model height 1.8 is provisional.')
for x,kind,label in [(25,'original','Original guide | 1x'),(235,'revised','Revised guide | 1x')]:
    tree=Image.open(ROOT/(kind+'-logical.png')).convert('RGBA')
    im.paste(tree,(x,105),tree)
    ground=105+tree.getbbox()[3]
    im.paste(hero,(x+145,ground-hero.height),hero)
    d.text((x,80),label,font=small,fill=(235,231,213))
    d.line((x,ground,x+180,ground),fill=(87,99,94))
tree=Image.open(ROOT/'revised-logical.png').convert('RGBA')
twice=tree.resize((272,544),Image.Resampling.NEAREST)
im.paste(twice,(470,95),twice)
ground=95+tree.getbbox()[3]*2
h2=hero.resize((hero.width*2,hero.height*2),Image.Resampling.NEAREST)
im.paste(h2,(762,ground-h2.height),h2)
d.text((470,75),'Revised guide + same player | exact 2x',font=small,fill=(235,231,213))
d.line((470,ground,830,ground),fill=(87,99,94))
d.text((25,420),'Pre-generation guide:',font=font,fill=(235,231,213))
d.text((25,449),'16 gray levels; binary alpha.',font=small,fill=(181,190,187))
d.text((25,476),'Separate exact 4x reference retained.',font=small,fill=(181,190,187))
d.text((25,503),'No appearance pass has consumed it.',font=small,fill=(181,190,187))
d.text((25,635),'Scale calibration needs review; this is not a production-game acceptance capture.',font=small,fill=(181,190,187))
im.save(ROOT/'pixel-comparison.png')

for p in sorted(ROOT.rglob('*')):
    if p.is_file() and p.suffix in {'.png','.blend','.py'}:
        hashes[p.relative_to(ROOT).as_posix()] = hashlib.sha256(p.read_bytes()).hexdigest()
(ROOT/'reference-provenance.json').write_text(json.dumps({
    'reference_player':str(hero_path),'reference_player_sha256':hashlib.sha256(hero_path.read_bytes()).hexdigest(),
    'player_crop':list(hero_full.getbbox()),'logical_normalization':'16 gray levels, alpha threshold128; no resize',
    'enlargement':{'factor':4,'filter':'nearest-neighbor','path':'preappearance-reference-4x.png'},
    'reference_supplied_to_appearance_model':None,'appearance_generation_performed':False,
    'approval':'pending model-stage review','runtime_integrated':False,'sha256':hashes
},indent=2))
print('Comparison sheets written; exact 4x roundtrip and crisp alpha verified.')
