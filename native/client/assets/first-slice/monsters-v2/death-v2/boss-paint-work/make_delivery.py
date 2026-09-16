"""Check completed boss death clips; write a self-contained owner handoff manifest."""
from pathlib import Path
import json,hashlib,argparse
import numpy as np
from PIL import Image,ImageDraw
P=Path(__file__).parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main(directions):
 records=[];contact=Image.new('RGBA',(512,120*len(directions)),(36,42,35,255));draw=ImageDraw.Draw(contact)
 refs=json.loads((P/'input-provenance.json').read_text())['frames']
 for y,direction in enumerate(directions):
  key=f'well-alpha-death-{direction}'
  for phase in range(4):
   folder=P/'singles'/key/f'phase-{phase:02d}';report=json.loads((folder/'reconstruction.json').read_text())
   ref=next(r for r in refs if r['direction']==direction and r['phase']==phase)
   source=folder/'source.png';sprite=folder/'fixed-placement.png';guide=P/'guides'/f'{key}-{phase:02d}.png';geometry=P/'death-v2'/f'{key}-{phase:02d}.png';style=P/'style-exemplars'/f'idle-{direction}-4x.png';prompt=folder/'prompt.txt'
   raw=Image.open(source);native=Image.open(sprite)
   assert raw.mode=='RGBA' and raw.size==(1536,1024)
   assert raw.getchannel('A').getextrema()[0]==0
   assert native.mode=='RGBA' and native.size==(128,96)
   assert set(np.unique(np.array(native)[:,:,3]))=={0,255}
   assert sha(source)==report['source_sha256'] and sha(geometry)==report['reference_sha256']==ref['geometry_reference_sha256']
   assert sha(guide)==ref['guide_sha256']
   assert report['selected_authored_grid']['size']==8 and not report['selected_authored_grid']['gated_out']
   assert report['fixed_placement_iou']>=.85
   outside=np.array(raw)[:,:,3]>=128;outside[128:896,256:1280]=False
   assert not outside.any(),f'{key}phase{phase}:foreground outside authored128x96 frame'
   box=native.getbbox();assert box and box[0]>0 and box[1]>0 and box[2]<128 and box[3]<96
   record={'actor':'well-alpha','action':'death','direction':direction,'phase':phase,'frame':[128,96],'anchor':[64,64],'pixels_per_metre':48,'fixed_placement_iou':report['fixed_placement_iou'],'applied_translation':[0,0],'rescaled':False,'visual_review':'cohort reviewed against same-direction accepted idle/walk; owner packaging approval separate'}
   for n,path in [('sprite',sprite),('source',source),('guide',guide),('geometry_reference',geometry),('style_exemplar',style),('prompt',prompt),('reconstruction_report',folder/'reconstruction.json')]:record[n]=path.relative_to(P).as_posix();record[n+'_sha256']=sha(path)
   records.append(record);contact.alpha_composite(native,(phase*128,y*120+20));draw.text((phase*128+3,y*120+3),f'{direction} {phase}',fill=(255,230,170,255))
 manifest={'status':'reviewed art handoff; packaging owner controls promotion','frame':[128,96],'anchor':[64,64],'pixels_per_metre':48,'generator':'built-in imagegen; exact8x colored pose plus4x same-direction style exemplar','reconstruction':'PixelRespecter authored8px grid; fixed inset32,16; no per-frame translation or scale','clips':directions,'frames':records,'counts':len(records)}
 (P/'delivery-manifest.json').write_text(json.dumps(manifest,indent=2));contact.save(P/'delivery-contact-native.png');contact.resize((1024,240*len(directions)),Image.Resampling.NEAREST).save(P/'delivery-contact-2x.png');print('Checked delivery frames:',len(records))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('directions',nargs='+');main(p.parse_args().directions)
