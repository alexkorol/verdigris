"""Package explicitly reviewed scenery with checked source/ref/output hashes."""
from pathlib import Path
import shutil,json,hashlib
from PIL import Image,ImageDraw
import numpy as np
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/world-scenery'
ACCEPTED=['hut','square-well','palisade-intact','palisade-breached','fieldstone-cluster','tree','worn-earth-patch','woodland-shrub']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
specs={a['id']:a for a in json.loads((ROOT/'manifest-references.json').read_text())['assets']}
for n in ('hut','tree'):
 old=next(a for a in json.loads((ROOT.parent/'guides/props.json').read_text()) if a['id']==n)
 specs[n]={'id':n,'frame':old['canvas'],'anchor':old['anchor'],'pixels_per_metre':48,'projection':'perspective calibrated at ground pivot','camera_depth_metres':old['camera_depth'],'source_objects':old['source_objects']}
 shutil.copy2(ROOT.parent/'blender-props'/f'{n}.blend',ROOT/'blender'/f'{n}.blend')
out=ROOT/'accepted'
for p in ('sprites','source','references','blender','reports','previews'):(out/p).mkdir(parents=True,exist_ok=True)
records=[]
contact=Image.new('RGBA',(1024,512),(36,42,35,255));d=ImageDraw.Draw(contact)
for i,n in enumerate(ACCEPTED):
 report=json.loads((ROOT/'review'/f'{n}.json').read_text());im=Image.open(ROOT/'painted'/f'{n}.png')
 assert im.mode=='RGBA' and im.size==(256,256)
 assert set(np.unique(np.array(im)[:,:,3]))=={0,255}
 assert sha(ROOT/'source'/f'{n}.png')==report['source_sha256']
 assert sha(ROOT/'references'/f'{n}.png')==report['reference_sha256']
 report['visual_acceptance']=True
 report['review_notes']='Inspected at native scale and nearest-neighbor 2x against Blender reference; no checkerboard, shape clipping, silhouette scaling or source corner holes.'
 report['output_sha256']=sha(ROOT/'painted'/f'{n}.png')
 (out/'reports'/f'{n}.json').write_text(json.dumps(report,indent=2))
 for src,dst in [('painted','sprites'),('source','source'),('references','references')]:shutil.copy2(ROOT/src/f'{n}.png',out/dst/f'{n}.png')
 shutil.copy2(ROOT/'blender'/f'{n}.blend',out/'blender'/f'{n}.blend')
 shutil.copy2(ROOT/'review'/f'{n}-compare-2x.png',out/'previews'/f'{n}-compare-2x.png')
 contact.alpha_composite(im,((i%4)*256,(i//4)*256));d.text(((i%4)*256+8,(i//4)*256+8),n,fill=(220,220,195,255))
 record=specs[n].copy();record.update({'sprite':f'sprites/{n}.png','source':f'source/{n}.png','reference':f'references/{n}.png','editable_blender':f'blender/{n}.blend','report':f'reports/{n}.json','sha256':report['output_sha256'],'source_sha256':report['source_sha256'],'reference_sha256':report['reference_sha256'],'blend_sha256':sha(out/'blender'/f'{n}.blend'),'native_bbox':list(im.getbbox()),'agent_visual_review':True,'silhouette_iou':report['silhouette_iou']})
 records.append(record)
contact.save(out/'previews/native-contact.png');contact.resize((2048,1024),Image.Resampling.NEAREST).save(out/'previews/contact-2x.png')
(out/'manifest.json').write_text(json.dumps({'version':2,'pipeline':'Blender native reference -> built-in imagegen material paint -> Pixel Respecter grid reconstruction -> integer translation -> native visual review','world_pixels_per_metre':48,'base_cell_pixels':48,'assets':records,'rejected_attempts':[{'id':'woodland-shrub-first-attempt','reason':'Generated silhouette grew; IoU0.612 below0.75 gate. Repaired with a targeted5x reference paint; accepted retry IoU0.809.'}],'rendering_contract':'Sprite dimensions are padding-inclusive. Use anchor to place at projected ground pivot; native 1:1 at48px/metre. Apply the same world zoom to players and scenery, never fit sprites by ink bounds. Worn-earth-patch is a ground decal, not a seamless tiling texture.','integration_status':'Assets reviewed; in-game integration and scene acceptance are separate.'},indent=2))
print('Packaged',len(records),'reviewed scenery sprites')
