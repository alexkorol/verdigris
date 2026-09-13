"""Native/integer previews and measured tile seams; no source/image corrections."""
from pathlib import Path
import json
import numpy as np
from PIL import Image
from import_assets import contact_sheet

ROOT=Path(__file__).resolve().parent
RUNTIME=ROOT/'../../client/assets/raster/runtime'
def main():
    assets=[(name,Image.open(RUNTIME/f'{name}.png').convert('RGBA')) for name in ('terrain_packed_earth','exit_stairs')]
    for scale in (1,3):
        contact_sheet(assets,ROOT/f'environment-singles-preview-{scale}x.png',scale)
    tile=assets[0][1]
    repeated=Image.new('RGBA',(tile.width*3,tile.height*3))
    for y in range(3):
        for x in range(3): repeated.paste(tile,(tile.width*x,tile.height*y))
    repeated.save(ROOT/'terrain-packed-earth-repeat-1x.png')
    repeated.resize((repeated.width*3,repeated.height*3),Image.Resampling.NEAREST).save(ROOT/'terrain-packed-earth-repeat-3x.png')
    rgb=np.asarray(tile)[:,:,:3].astype(float)
    def stats(values): return {'mean_absolute_rgb':round(float(values.mean()),4),'p95_absolute_rgb':round(float(np.percentile(values,95)),4),'max_absolute_rgb':round(float(values.max()),4)}
    report={'name':'terrain_packed_earth','canvas':list(tile.size),'colors':len(np.unique(rgb.reshape(-1,3),axis=0)),
            'horizontal_seam':stats(abs(rgb[:,0]-rgb[:,-1])), 'vertical_seam':stats(abs(rgb[0]-rgb[-1])),
            'horizontal_neighbors':stats(abs(np.diff(rgb,axis=1))), 'vertical_neighbors':stats(abs(np.diff(rgb,axis=0))),
            'method':'Opposing tile edges versus ordinary adjacent native pixels; no blending, wrapping correction, or hidden seam smoothing.',
            'acceptance':'repeated_preview_inspected_pending_live_game'}
    (ROOT/'terrain-packed-earth-seams.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))
    directions=[(f'hero_{d}',Image.open(RUNTIME/f'hero_{d}.png').convert('RGBA')) for d in ('se','sw','nw','ne')]
    for scale in (1,4): contact_sheet(directions,ROOT/f'hero-directions-preview-{scale}x.png',scale)

if __name__=='__main__': main()
