"""Recover generated scenery grid without per-object scaling or RGB alpha keying."""
from pathlib import Path
import sys,json,hashlib,argparse
import numpy as np
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/world-scenery'
def run(raw,name):
 sys.path.insert(0,'Z:/Code/Python/pixel-perfecter')
 from pixel_perfecter.reconstructor import PixelArtReconstructor
 im=Image.open(raw)
 if im.mode!='RGBA' or im.getchannel('A').getextrema()[0]!=0:raise ValueError('Genuine returned PNG alpha required')
 a=np.array(im); a[:,:,3]=np.where(a[:,:,3]>=128,255,0);a[a[:,:,3]==0,:3]=0
 rec=PixelArtReconstructor(image=a);rec.use_hough=False;rec.run()
 pitch=round(im.width/256)
 candidate=next(x for x in rec.last_fit_debug['candidates'] if x['size']==pitch)
 if candidate['gated_out']:raise ValueError('Expected pixel grid rejected by variance gate')
 rec.cell_size=pitch;rec.offset=tuple(candidate['offset']);recovered=Image.fromarray(rec._empirical_pixel_reconstruction())
 a=np.array(recovered);a[:,:,3]=np.where(a[:,:,3]>=128,255,0);a[a[:,:,3]==0,:3]=0;recovered=Image.fromarray(a)
 refpath=ROOT/'references'/f'{name}.png';ref=Image.open(refpath).convert('RGBA');mask=np.array(ref)[:,:,3]>=128
 best=(-1,0,0)
 for dy in range(-32,33):
  for dx in range(-32,33):
   b=np.array(recovered.crop((-dx,-dy,256-dx,256-dy)))[:,:,3]>=128
   iou=np.count_nonzero(b&mask)/max(1,np.count_nonzero(b|mask))
   if iou>best[0]:best=(iou,dx,dy)
 score,dx,dy=best;out=recovered.crop((-dx,-dy,256-dx,256-dy));bbox=out.getbbox()
 report={'id':name,'source_sha256':hashlib.sha256(Path(raw).read_bytes()).hexdigest(),'reference_sha256':hashlib.sha256(refpath.read_bytes()).hexdigest(),'returned_size':im.size,'pixel_grid':candidate,'all_grid_candidates':rec.last_fit_debug['candidates'],'translation':[dx,dy],'silhouette_iou':score,'canvas':[256,256],'bbox':bbox,'rescaling':False,'alpha_policy':'Original alpha threshold128; no color key','visual_acceptance':False}
 (ROOT/'source').mkdir(exist_ok=True);(ROOT/'source'/f'{name}.png').write_bytes(Path(raw).read_bytes())
 (ROOT/'review'/f'{name}.json').write_text(json.dumps(report,indent=2))
 if score<.75:raise ValueError(f'Silhouette drift IoU={score:.3f}')
 if np.count_nonzero(np.array(out)[:,:,3])!=np.count_nonzero(a[:,:,3]):raise ValueError('Registration clipped pixels')
 if bbox[0]==0 or bbox[1]==0 or bbox[2]==256 or bbox[3]==256:raise ValueError('Canvas clipping')
 out.save(ROOT/'painted'/f'{name}.png')
 preview=Image.new('RGBA',(512,256),(38,43,36,255));preview.alpha_composite(ref,(0,0));preview.alpha_composite(out,(256,0));preview.resize((1024,512),Image.Resampling.NEAREST).save(ROOT/'review'/f'{name}-compare-2x.png')
 print(json.dumps({'id':name,'iou':score,'pitch':pitch,'translation':[dx,dy],'bbox':bbox}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('raw');p.add_argument('name');a=p.parse_args();run(a.raw,a.name)
