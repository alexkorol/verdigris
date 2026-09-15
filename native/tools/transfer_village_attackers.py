"""Register ONE appearance atlas, then recover native cells with Pixel Respecter.

Only the appearance painting is registered. Animated frames always come from
Blender at 48 px/m, and are never scaled or aligned by their bounding boxes.
"""
import argparse,hashlib,json,sys,shutil
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw
from scipy.optimize import differential_evolution
from scipy.ndimage import map_coordinates

def main():
    p=argparse.ArgumentParser();p.add_argument('folder',type=Path);p.add_argument('source',type=Path);p.add_argument('--pixel-respecter',type=Path,default=Path('Z:/Code/Python/pixel-perfecter'));a=p.parse_args()
    sys.path.insert(0,str(a.pixel_respecter))
    from pixel_perfecter.workspace import reconstruct,Options
    original=Image.open(a.source);assert original.mode=='RGBA','ImageGen must deliver actual alpha'
    original.save(a.folder/'imagegen-original.png');src=np.array(original)
    assert src[:,:,3].min()==0 and src[:,:,3].max()==255
    assert (src[:,:,3]==0).mean()>.65,'Reject opaque/generated background'
    canvas=Image.new('RGBA',(256,256));review=Image.new('RGBA',(384,384),(37,41,37,255));draw=ImageDraw.Draw(review);report=[]
    for d,name in enumerate(('front','right','back','left')):
        size=original.width//2;slot=src[d//2*size:(d//2+1)*size,d%2*size:(d%2+1)*size]
        ref=np.array(Image.open(a.folder/'guides'/f'{name}.png'));mask=ref[:,:,3]>=128
        yy,xx=np.where(mask);sy,sx=np.where(slot[:,:,3]>=128)
        scale=(sy.max()-sy.min())/(yy.max()-yy.min());tx=(sx.max()+sx.min())/2-scale*(xx.max()+xx.min())/2;ty=sy.min()-scale*yy.min()
        gy,gx=np.mgrid[:96,:96]
        def sample(params,channel=3):
            s,x,y=params
            return map_coordinates(slot[:,:,channel].astype(float),[gy*s+y,gx*s+x],order=0,mode='constant',cval=0)
        def loss(params):
            m=sample(params)>=128;return 1-(m&mask).sum()/max(1,(m|mask).sum())
        fit=differential_evolution(loss,[(scale*.90,scale*1.10),(tx-scale*5,tx+scale*5),(ty-scale*5,ty+scale*5)],seed=12,popsize=10,tol=.0005,polish=False)
        s,x,y=fit.x
        gy4,gx4=np.mgrid[:384,:384];coords=[((gy4+.5)/4-.5)*s+y,((gx4+.5)/4-.5)*s+x]
        registered=np.stack([map_coordinates(slot[:,:,c],coords,order=0,mode='constant',cval=0) for c in range(4)],axis=-1)
        result=reconstruct(registered,Options(cell_size=4,alpha_mode='crisp')).image
        result[result[:,:,3]==0,:3]=0
        assert result.shape==(96,96,4)
        im=Image.fromarray(result);canvas.paste(im,(d%2*128+16,d//2*128+16))
        review.alpha_composite(Image.fromarray(ref),(d*96,0));review.alpha_composite(im,(d*96,112))
        overlap=np.zeros((96,96,4),dtype=np.uint8);overlap[:,:,0]=mask*255;overlap[:,:,1]=(result[:,:,3]>0)*255;overlap[:,:,3]=255
        review.alpha_composite(Image.fromarray(overlap),(d*96,224))
        report.append({'direction':name,'source_pixels_per_native_cell':float(s),'translation_in_source_slot':[float(x),float(y)],'silhouette_iou':1-float(fit.fun)})
    canvas.save(a.folder/'appearance-native.png');canvas.resize((1024,1024),Image.Resampling.NEAREST).save(a.folder/'appearance-review-4x.png')
    review.resize((1152,1152),Image.Resampling.NEAREST).save(a.folder/'registration-review.png')
    meta={'source_size':original.size,'source_sha256':hashlib.sha256(a.source.read_bytes()).hexdigest(),'native_atlas':[256,256],'method':'Per-view uniform appearance registration; Pixel Respecter manual4 dominant-color/majority-alpha reconstruction. Blender alone supplies final silhouette, animation pixels and pivots.','views':report}
    (a.folder/'transfer.json').write_text(json.dumps(meta,indent=2));print(json.dumps(meta))
if __name__=='__main__':main()
