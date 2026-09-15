"""Recover 2x2 combat proofs on their measured lattice without sprite resizing."""
import argparse,json,hashlib,sys
from pathlib import Path
import numpy as np
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-combat'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def recover(name,sex,start,respect_root):
 sys.path.insert(0,str(respect_root))
 from pixel_perfecter.reconstructor import PixelArtReconstructor
 raw=ROOT/'originals'/f'{name}.png'; im=Image.open(raw);a=np.array(im)
 assert im.mode=='RGBA' and a[:,:,3].min()==0 and a[:,:,3].max()>=250
 a[:,:,3]=np.where(a[:,:,3]>=128,255,0);a[a[:,:,3]==0,:3]=0
 r=PixelArtReconstructor(image=a);r.use_hough=False;r.run()
 c=next(c for c in r.last_fit_debug['candidates'] if c['size']==5 and not c['gated_out'])
 r.cell_size=5;r.offset=tuple(c['offset']);rec=Image.fromarray(r._empirical_pixel_reconstruction());ox,oy=c['offset']
 refs=[ROOT/'references'/f'{sex}-attack-front-{i:02d}.png' for i in range(start,start+4)]
 masks=[np.array(Image.open(p))[:,:,3]>=128 for p in refs]
 # Actual raster dimensions determine equal sheet cells. Their inferred lattice
 # origins may be125/126px apart; transparent padding fills the authored128frame.
 slots=[];origins=[];counts=[]
 for i in range(4):
  col,row=i%2,i//2;x=round(col*im.width/10-ox/5);y=round(row*im.height/10-oy/5)
  endx=round((col+1)*im.width/10-ox/5);endy=round((row+1)*im.height/10-oy/5)
  slot=rec.crop((x,y,endx,endy));padded=Image.new('RGBA',(128,128));padded.paste(slot,(0,0));slots.append(padded);origins.append([x,y]);counts.append(np.count_nonzero(np.array(slot)[:,:,3]>=128))
 best=(-1,None,None,[])
 for dy in range(-12,13):
  for dx in range(-12,13):
   scores=[]
   for s,m in zip(slots,masks):
    b=np.array(s.crop((-dx,-dy,128-dx,128-dy)))[:,:,3]>=128;scores.append(float(np.count_nonzero(b&m)/max(1,np.count_nonzero(b|m))))
   if np.mean(scores)>best[0]:best=(float(np.mean(scores)),dx,dy,scores)
 score,dx,dy,scores=best;out=ROOT/'recovery'/name;out.mkdir(parents=True,exist_ok=True)
 report={'source':str(raw),'source_sha256':sha(raw),'actual_dimensions':list(im.size),'requested_dimensions':[1024,1024],'grid':c,'all_grid_candidates':r.last_fit_debug['candidates'],'frame':[128,128],'anchor':[64,96],'pixels_per_metre':48,'registration':[dx,dy],'mean_iou_alpha128':score,'visual_acceptance':False,'resizing':'none; measured native grid reconstructed directly','frames':[]}
 preview=Image.new('RGBA',(1024,512),(35,44,31,255))
 for i,(s,p,m) in enumerate(zip(slots,refs,masks)):
  frame=s.crop((-dx,-dy,128-dx,128-dy));b=np.array(frame);b[:,:,3]=np.where(b[:,:,3]>=128,255,0);b[b[:,:,3]==0,:3]=0;frame=Image.fromarray(b)
  count=int(np.count_nonzero(b[:,:,3]));box=frame.getbbox();assert count==counts[i],f'Foreground loss phase{i}';assert box and min(box[:2])>0 and max(box[2:])<128
  path=out/f'{sex}-club-attack-front-{start+i:02d}.png';frame.save(path)
  preview.alpha_composite(Image.open(p).resize((256,256),Image.Resampling.NEAREST),((i%2)*512,(i//2)*256));preview.alpha_composite(frame.resize((256,256),Image.Resampling.NEAREST),((i%2)*512+256,(i//2)*256))
  report['frames'].append({'file':path.name,'sha256':sha(path),'reference':str(p),'reference_sha256':sha(p),'raw_lattice_origin':origins[i],'foreground_pixels':count,'bbox':box,'iou_alpha128':scores[i]})
 report['geometry_gate_pass']=score>=.70 and min(scores)>=.65
 (out/'report.json').write_text(json.dumps(report,indent=2));preview.save(out/'comparison-2x.png');print(json.dumps({'name':name,'iou':scores,'gate':report['geometry_gate_pass'],'preview':str(out/'comparison-2x.png')}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('name');p.add_argument('sex');p.add_argument('start',type=int);p.add_argument('--respect-root',type=Path,default=Path('Z:/Code/Python/pixel-perfecter'));args=p.parse_args();recover(args.name,args.sex,args.start,args.respect_root)
