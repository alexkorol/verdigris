"""Fixed atlas-grid material transfer. Never fits an object bounding box."""
from pathlib import Path
import hashlib,json
import numpy as np
from PIL import Image
ROOT=Path(__file__).resolve().parent.parent
SRC=ROOT/'source'
im=Image.open(SRC/'generated-atlas-retry.png').convert('RGBA')
if im.size!=(1536,1024):raise ValueError('This transfer requires the exact authored atlas grid')
a=np.array(im)
blocks=a.reshape(512,2,768,2,4).transpose(0,2,1,3,4).reshape(512,768,4,4)
# Choose a real sample closest to the block median; do not blur RGB by averaging.
median=np.median(blocks[:,:,:,:3],axis=2)
distance=((blocks[:,:,:,:3].astype(float)-median[:,:,None,:])**2).sum(3)
sample=distance.argmin(2)
native=np.take_along_axis(blocks,sample[:,:,None,None],axis=2)[:,:,0,:].copy()
# Generated RGB has colour in alpha-zero pixels; clear it after binary coverage.
native[:,:,3]=np.where((blocks[:,:,:,3]>=240).sum(2)>=2,255,0)
native[native[:,:,3]==0]=0
atlas=Image.fromarray(native)
atlas.save(ROOT/'atlas-native.png')
clips=[];measurements=[]
out=ROOT/'sprites';out.mkdir(exist_ok=True)
for i,(identity,guide) in enumerate([('village-longhouse','longhouse'),('village-tree','mature-tree'),('village-palisade','palisade'),('village-well','square-well')]):
 x=(i%2)*384
 if i<2:frame=atlas.crop((x,0,x+384,384))
 else:
  frame=Image.new('RGBA',(384,384));frame.paste(atlas.crop((x,384,x+384,512)),(0,256))
 frame.save(out/(identity+'.png'))
 alpha=frame.getchannel('A')
 assert set(alpha.getdata())=={0,255}
 clips.append({'identity':identity,'action':'idle','direction':'front','frames':['sprites/'+identity+'.png'],'frame':[384,384],'anchor':[192,354],'pixels_per_metre':48,'fps':1,'loop':True})
 measurements.append({'identity':identity,'guide_bbox':Image.open(SRC/(guide+'-guide.png')).getbbox(),'paint_bbox':frame.getbbox(),'sha256':hashlib.sha256((out/(identity+'.png')).read_bytes()).hexdigest()})
for factor in [1,2]:
 preview=Image.new('RGBA',atlas.size,(45,45,45,255));preview.alpha_composite(atlas)
 preview.convert('RGB').resize((768*factor,512*factor),Image.Resampling.NEAREST).save(ROOT/('review-'+str(factor)+'x.png'))
(ROOT/'manifest.json').write_text(json.dumps({'accepted':True,'acceptance_scope':'Agent viewed native-scale cutouts on solid grey; owner acceptance and integrated live-game review pending.','clips':clips},indent=2)+'\n')
(ROOT/'review.json').write_text(json.dumps({'pipeline':'saved Blender geometry -> render at 48 px/metre -> nearest 2x guide -> imagegen paint -> fixed 2x2 representative sample -> binary true alpha','source_size':list(im.size),'logical_atlas':[768,512],'alpha_threshold':240,'coverage_rule':'at least 2 of 4 samples','bbox_fit':False,'per_object_scaling':False,'native_measurements':measurements,'limitations':['ImageGen did not reproduce an exact flat-colour 2x2 lattice; this is fixed-grid material sampling, not proof of lossless recovery of an inferred pixel lattice.','Longhouse roof ridge is 13 logical pixels higher than the Blender guide and doorway was painted closed; ground contact is retained.','Tree canopy grew 16 logical pixels wider overall and 13 pixels higher; ground contact is retained.','Palisade width grew 8 logical pixels overall through stake texture; section overlap may need placement review.','Player-scale source exemplar was a nearest 8x enlargement of a 96 px mannequin sprite, used for pixel cadence only.','First square atlas attempt rejected because canvas and layout changed; retained under source/generated-atlas.png.']},indent=2)+'\n')
print(json.dumps(measurements,indent=2))
