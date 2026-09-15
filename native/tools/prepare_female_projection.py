"""Build female-only surface paint views without accepting their generated silhouettes."""
import argparse,hashlib,json,sys,shutil
from pathlib import Path
import numpy as np
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice';WORK=ROOT/'starter-v2';REF=ROOT/'starter-v2-female-refine';OUT=ROOT/'starter-projection/female-source-prep'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def build(respect_root):
 sys.path.insert(0,str(respect_root))
 from pixel_perfecter.reconstructor import PixelArtReconstructor
 OUT.mkdir(parents=True,exist_ok=True)
 for source_name in ['call-456.js','call-618.js']:
  shutil.copy2(WORK/'generation-calls'/source_name,OUT/source_name)
 (OUT/'.gitattributes').write_text('*.js -text\n')
 atlas=Image.new('RGBA',(1536,1024));views=[]
 for col,direction in enumerate(['front','right','back','left']):
  clip='female-walk-'+direction;raw=WORK/'originals'/f'{clip}.png'
  if direction=='front':
   path=WORK/'accepted/frames'/f'{clip}-00.png';frame=Image.open(path).convert('RGBA');report=json.loads((WORK/'accepted/reports'/f'{clip}.json').read_text());c=report['selected_grid'];reg=report['registrations'][0]
  else:
   report=json.loads((WORK/'review'/f'{clip}-rejected.json').read_text());c=report['selected_grid'];reg=report['registrations'][0];data=np.array(Image.open(raw));data[:,:,3]=np.where(data[:,:,3]>=128,255,0);data[data[:,:,3]==0,:3]=0
   r=PixelArtReconstructor(image=data);r.use_hough=False;r.run();assert any(x['size']==4 and x['offset']==c['offset'] and not x['gated_out'] for x in r.last_fit_debug['candidates'])
   r.cell_size=4;r.offset=tuple(c['offset']);rec=Image.fromarray(r._empirical_pixel_reconstruction());ox,oy=c['offset'];dx,dy=reg['translation'];x=round(-ox/4);y=round(-oy/4)
   frame=rec.crop((x-dx,y+16-dy,x+96-dx,y+112-dy));a=np.array(frame);a[:,:,3]=np.where(a[:,:,3]>=128,255,0);a[a[:,:,3]==0,:3]=0;frame=Image.fromarray(a)
  target=OUT/f'{direction}.png';frame.save(target);atlas.alpha_composite(frame.resize((384,384),Image.Resampling.NEAREST),(col*384,64))
  ref=REF/'references'/f'{clip}-00.png';blend=REF/'blender/female-walk-exterior.blend'
  views.append({'direction':direction,'image':str(target),'image_sha256':sha(target),'source_image':str(raw),'source_image_sha256':sha(raw),'geometry_file':str(blend),'geometry_sha256':sha(blend),'frame':72,'shared_sheet_registration':reg,'native_grid':c,'scale':4,'original_native_reference':str(ref),'original_native_reference_sha256':sha(ref),'source_guide':str(REF/'guides'/f'{clip}.png'),'exact_generation_call':str(OUT/('call-456.js' if direction=='front' else 'call-618.js')),'exact_generation_call_sha256':sha(OUT/('call-456.js' if direction=='front' else 'call-618.js')),'use':'Surface appearance only; Blender geometry and visibility determine silhouette. Generated side/back silhouettes failed geometry gate and are not standalone accepted sprites.','geometry_input_verified':'Exact tool call618 uses starter-v2-female-refine guide for this direction; front call456 uses refined front guide.'})
 atlas.save(OUT/'appearance-atlas.png');(OUT/'views.json').write_text(json.dumps({'sex':'female','atlas':str(OUT/'appearance-atlas.png'),'atlas_sha256':sha(OUT/'appearance-atlas.png'),'views':views},indent=2));bg=Image.new('RGBA',atlas.size,(35,44,31,255));bg.alpha_composite(atlas);bg.save(OUT/'appearance-preview.png');print(OUT)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--respect-root',type=Path,default=Path('Z:/Code/Python/pixel-perfecter'));build(p.parse_args().respect_root)
