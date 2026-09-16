from pathlib import Path
import hashlib,json
import numpy as np
from PIL import Image
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[3]
OUT=BASE/'v2'
path=OUT/'terrain_quiet_stone_offset.png'
im=Image.open(path).convert('RGBA');assert im.size==(64,64)
a=np.array(im)
restored=np.roll(a,(-32,-32),(0,1))
Image.fromarray(restored).save(OUT/'terrain_quiet_stone.png')
source=ROOT/'native/client/assets/raster/source/terrain-quiet-stone-v2.png'
raw=Image.open(source).convert('RGBA');width,height=raw.size
Image.fromarray(np.roll(np.array(raw),(-(height//2),-(width//2)),(0,1))).save(OUT/'source-restored-phase.png')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
assert np.array_equal(np.sort(a.reshape(-1,4),axis=0),np.sort(restored.reshape(-1,4),axis=0))
old=np.array(Image.open(OUT/'offset-reference-native.png').convert('RGBA'))
# Diagnostic comparison, not an editing mask. Prompt requested narrow center bands.
yy,xx=np.indices((64,64));outside=(abs(xx-31.5)>=4)&(abs(yy-31.5)>=4)
delta=np.abs(a[:,:,:3].astype(float)-old[:,:,:3].astype(float))
record={
 'method':'Exact periodic integer wrap after actual Pixel Respecter reconstruction; no blending, mirroring, repainting or changed RGB values.',
 'input':path.name,'input_sha256':sha(path),'native_offset_xy':[-32,-32],
 'output':'terrain_quiet_stone.png','output_sha256':sha(OUT/'terrain_quiet_stone.png'),
 'source_phase_preview':{'path':'source-restored-phase.png','input_source_sha256':sha(source),'offset_xy':[-(width//2),-(height//2)],'purpose':'Source-view diagnostic only; final native pixels come from importer then native32px wrap.'},
 'outside_requested_center_band_comparison':{'band':'8 logical pixels wide around each center axis; diagnostic interpretation of narrow bands.','pixels':int(outside.sum()),'exact_rgb_preserved_pixels':int(np.all(a[:,:,:3]==old[:,:,:3],axis=2)[outside].sum()),'rgb_mae':float(delta[outside].mean()),'limitation':'Generation plus reconstruction/normalization differences, not an isolated source-model metric.'}
}
(OUT/'phase.provenance.json').write_text(json.dumps(record,indent=2)+'\n')
print(json.dumps(record))
