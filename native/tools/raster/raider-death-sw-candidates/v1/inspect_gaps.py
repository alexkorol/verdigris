from pathlib import Path
import sys,json
import numpy as np
from PIL import Image,ImageDraw
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
sys.path.insert(0,str(ROOT/'native/tools/raster'))
import import_assets
import_assets.load_engine(Path('Z:/Code/Python/pixel-perfecter'))
source=Image.open(ROOT/'native/client/assets/raster/source/raider-death-sw-v1.png').convert('RGBA')
rgba,record=import_assets.remove_background(np.array(source),{'color':[210,210,210],'tolerance':95})
clean=Image.fromarray(rgba)
scopes=[[480,120,565,220],[525,300,595,418],[300,440,390,510],[430,430,505,515],[1015,272,1070,350],[1090,435,1160,525],[220,765,325,835],[914,844,1010,914],[1230,850,1308,923]]
gallery=Image.new('RGB',(600,9*170),(32,31,29));d=ImageDraw.Draw(gallery)
for i,box in enumerate(scopes):
    for j,im in enumerate((source,clean)):
        c=im.crop(box);c=c.resize((c.width*2,c.height*2),Image.Resampling.NEAREST)
        gallery.paste(c,(j*290+5,i*170+18),c)
    d.text((5,i*170),f'{i} {box} original / border cleanup',fill=(230,225,214))
gallery.save(BASE/'source-gap-inspection.png')
(BASE/'gap-scopes.json').write_text(json.dumps(scopes,indent=2)+'\n')
print(json.dumps({'source':source.size,'mode_original':'RGB','border_transparent':record['transparent_pixels_after']}))
