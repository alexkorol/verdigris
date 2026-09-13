from pathlib import Path
import hashlib,json
from PIL import Image
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[3]
sheet=Image.new('RGBA',(320,192))
refs=[]
for i in range(8):
    p=ROOT/f'native/client/assets/raster/runtime/hero_walk{i}_sw.png'
    im=Image.open(p).convert('RGBA')
    assert im.size==(80,96)
    sheet.paste(im,((i%4)*80,(i//4)*96))
    refs.append({'frame':i,'path':p.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'native_canvas':[80,96],'native_pivot':[40,96]})
sheet.save(BASE/'hero-sw-gait-guide-native.png')
sheet.resize((1280,768),Image.Resampling.NEAREST).save(BASE/'hero-sw-gait-guide-4x.png')
(BASE/'hero-sw-gait-guide.provenance.json').write_text(json.dumps({'method':'Mechanical row-major assembly of eight existing accepted PNGs. One uniform4x nearest-neighbor enlargement; no pose changes.','role':'Lower-body gait geometry/order only, not appearance or equipment.','frames':refs},indent=2)+'\n')
idle=Image.open(ROOT/'native/client/assets/raster/runtime/wight_sw.png').convert('RGBA')
print(idle.size,idle.getchannel('A').getbbox())
