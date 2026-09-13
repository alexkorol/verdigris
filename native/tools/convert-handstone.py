"""Assemble the generated color and segmentation mask into native RGBA assets.

Preserves both originals. No palette reduction or pixelization of inventory art.
The world attachment is an aspect-fit thumbnail at the existing actor texel scale.
"""
from pathlib import Path
import hashlib,json,sys
import numpy as np
from PIL import Image
root=Path(__file__).resolve().parents[2]
folder=root/'native/client/assets/wizard/inventory'
color=Image.open(folder/'source/handstone-color.png').convert('RGBA')
mask=Image.open(folder/'source/handstone-mask.png').convert('L')
assert color.size==mask.size
mask=mask.point(lambda v: 0 if v<64 else 255 if v>192 else (v-64)*255//128)
color.putalpha(mask)
color.save(folder/'handstone_flint.png')
sys.path.insert(0,str(root/'native/tools/raster'))
from import_assets import load_engine, engine_provenance, DEFAULT_PROJECT
workspace=load_engine(DEFAULT_PROJECT)
source=color.crop(mask.getbbox())
result=workspace.reconstruct(np.asarray(source),workspace.Options(cell_size=78,alpha_mode='preserve',max_colors=16))
thumb=Image.fromarray(result.image).convert('RGBA')
thumb.thumbnail((12,18),Image.Resampling.NEAREST)
canvas=Image.new('RGBA',(12,18));canvas.alpha_composite(thumb,((12-thumb.width)//2,18-thumb.height))
runtime=root/'native/client/assets/raster/runtime'
canvas.save(runtime/'weapon_handstone.png')
manifest={'source':'Generated color and generated silhouette mask; image_gen',
          'conversion':'native/tools/convert-handstone.py',
          'alpha':'zero background, 255 interior, bounded antialiased mask edge',
          'world_attachment':'12x18 at existing actor scale; grip (5.5,14.5)',
          'pixel_respecter':engine_provenance(DEFAULT_PROJECT),
          'files':{p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest()
                   for p in [folder/'source/handstone-color.png',folder/'source/handstone-mask.png',folder/'handstone_flint.png',runtime/'weapon_handstone.png']}}
(folder/'handstone-provenance.json').write_text(json.dumps(manifest,indent=2)+'\n')
catalog=runtime/'catalog.json';data=json.loads(catalog.read_text())
data['assets']=[x for x in data['assets'] if x['name']!='weapon_handstone']
data['assets'].append({'name':'weapon_handstone','file':'weapon_handstone.png',
    'source':'native/client/assets/wizard/inventory/source/handstone-color.png',
    'acceptance':'package visual verification recorded in UI repair report'})
catalog.write_text(json.dumps(data,indent=2)+'\n')
print('RGBA handstone and attachment generated; originals preserved')

