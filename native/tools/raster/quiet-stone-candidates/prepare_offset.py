"""Expose periodic boundaries at the center; no RGB values are changed."""
from pathlib import Path
import json,hashlib
import numpy as np
from PIL import Image
BASE=Path(__file__).resolve().parent
source=BASE/'v1/terrain_quiet_stone.png'
original=Image.open(source).convert('RGBA')
assert original.size==(64,64)
offset=Image.fromarray(np.roll(np.array(original),(32,32),(0,1)))
out=BASE/'v2';out.mkdir(exist_ok=True)
offset.save(out/'offset-reference-native.png')
offset.resize((1024,1024),Image.Resampling.NEAREST).save(out/'offset-reference-16x.png')
original.resize((1024,1024),Image.Resampling.NEAREST).save(out/'v1-material-reference-16x.png')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
record={'input':'../v1/terrain_quiet_stone.png','input_sha256':sha(source),'native_offset_xy':[32,32],'native_canvas':[64,64],'reference_enlargement':16,'reference_canvas':[1024,1024],'method':'Exact periodic integer roll followed by nearest-neighbor enlargement; no color creation, blending or repainting.','images':{p.name:sha(p) for p in [out/'offset-reference-native.png',out/'offset-reference-16x.png',out/'v1-material-reference-16x.png']}}
(out/'offset-reference.provenance.json').write_text(json.dumps(record,indent=2)+'\n')
print(json.dumps(record))
