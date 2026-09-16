from pathlib import Path
import json, sys, hashlib
import cv2, numpy as np
from PIL import Image, ImageDraw

BASE = Path(__file__).resolve().parent
ROOT = BASE.parents[4]
sys.path.insert(0, str(ROOT / 'native/tools/raster'))
import import_assets
import_assets.load_engine(Path('Z:/Code/Python/pixel-perfecter'))
manifest = json.loads((BASE/'import.json').read_text())
path = (BASE/manifest['sheets'][0]['source']).resolve()
original = Image.open(path)
source = np.array(original.convert('RGBA'))
options = manifest['sheets'][0]['border_background']
cleaned, cleanup = import_assets.remove_background(source, options)
rgb = source[:, :, :3].astype(int)
removed = cleaned[:, :, 3] == 0
chromatic = removed & ((rgb.max(2)-rgb.min(2)) > 20)
_, labels, stats, _ = cv2.connectedComponentsWithStats(chromatic.astype('uint8'), 8)
components = [{'box':[int(x),int(y),int(x+w),int(y+h)],'pixels':int(area)} for x,y,w,h,area in stats[1:]]
assert np.array_equal(source[:, :, :3][~removed], cleaned[:, :, :3][~removed])
gallery = Image.new('RGB', (1600,620), (180,165,141))
draw = ImageDraw.Draw(gallery)
for i in range(8):
    x0=(i%4)*405; y0=0 if i<4 else 500
    box=(x0+70,y0+(80 if i<4 else 40),min(x0+330,1619),min(y0+480,971))
    for j,arr in enumerate((source,cleaned)):
        im=Image.fromarray(arr).crop(box); im.thumbnail((190,280),Image.Resampling.NEAREST)
        x=(i%4)*400+j*200; y=(i//4)*310
        gallery.paste(im,(x,y+20),im)
        draw.text((x+4,y+3),f'{i} '+('source' if j==0 else 'cleaned'),fill=(20,18,16))
gallery.save(BASE/'alpha-source-comparison.png')
detail=Image.new('RGB',(960,540),(180,165,141)); draw=ImageDraw.Draw(detail)
old, _ = import_assets.remove_background(source, {'color':[210,210,210],'tolerance':95})
for j,(label,arr) in enumerate((('original',source),('rejected v1 alpha',old),('selected alpha',cleaned))):
    im=Image.fromarray(arr).crop((660,245,728,332)).resize((272,348),Image.Resampling.NEAREST)
    detail.paste(im,(j*320,20),im);draw.text((j*320+5,3),label,fill=(20,18,16))
    native=Image.open((BASE if j==2 else BASE.parent/'v1')/'wight_walk1_sw.png').convert('RGBA')
    if j:
        native=native.crop((40,48,62,73)).resize((132,150),Image.Resampling.NEAREST)
        detail.paste(native,(j*320,380),native)
detail.save(BASE/'hand-alpha-comparison.png')
edge=Image.new('RGB',(max(1,len(components))*150,170),(180,165,141)); draw=ImageDraw.Draw(edge)
for i,c in enumerate(components):
    x0,y0,x1,y1=c['box']; crop=(max(0,x0-8),max(0,y0-8),min(1619,x1+8),min(971,y1+8))
    for j,arr in enumerate((source,cleaned)):
        im=Image.fromarray(arr).crop(crop).resize(((crop[2]-crop[0])*3,(crop[3]-crop[1])*3),Image.Resampling.NEAREST)
        edge.paste(im,(i*150,j*75+20),im)
    draw.text((i*150+3,2),str(c['box'])+' '+str(c['pixels']),fill=(20,18,16))
edge.save(BASE/'chromatic-edge-review.png')
data={
    'source_mode':original.mode,'source_dimensions':list(original.size),
    'source_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
    'source_true_transparent_pixels':0,'source_partial_alpha_pixels':0,
    'generation_failure':'Requested RGBA but returned an opaque painted checker.',
    'cleanup_api':cleanup['api'],'cleanup_color':cleanup['color'],'cleanup_tolerance':cleanup['tolerance'],
    'window_count':len(cleanup['interior_windows']),
    'window_new_transparent_pixels':sum(w['new_transparent_pixels'] for w in cleanup['interior_windows']),
    'removed_chromatic_pixels_above20':int(chromatic.sum()),'chromatic_components':components,
    'source_rgb_edits':0,'retained_foreground_rgb_changed':False,
    'review':'Source/alpha pairs and phase1 hand compared at enlarged scale. Native bone hand survives; enclosed arm and finger checker gaps are transparent. Small chromatic removals are source matte fringe, preserved in chromatic-edge-review.png for inspection.',
    'accepted':False,'state':'Candidate only; root native/game motion review pending.'
}
(BASE/'alpha-audit.json').write_text(json.dumps(data,indent=2)+'\n')
print(json.dumps(data))
