"""Freeze the reviewed 832-frame starter projection family and editable sources."""
import argparse,hashlib,json,shutil
from pathlib import Path
from package_starter_projection import package
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice';WORK=ROOT/'starter-projection';OUT=WORK/'accepted'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def freeze(male_cohort='male-v7',female_cohort='female-v4',output='accepted',audit='male-v7-female-v4-audit.json',visual_review='male-v7-female-v4-visual-review.json'):
 global OUT
 assert output not in ['.','..'] and '/' not in output and '\\' not in output
 OUT=WORK/output
 assert not OUT.exists(),'Accepted cohorts are immutable; choose a new version for changes'
 OUT.mkdir();(OUT/'.gitattributes').write_text('* -text\n')
 result={'accepted':True,'complete':True,'status':'Player and projection agents visually reviewed final native contacts; ready for native integration acceptance','technique':'ImageGen painted appearance reconstructed to native pixels, depth-tested on editable Blender rigs, rendered at48px/metre with Box1.0 and binaryalpha128','clips':[],'provenance':{},'sources':{}}
 for sex,cohort in [('male',male_cohort),('female',female_cohort)]:
  src=WORK/cohort;dest=OUT/sex;package(src/'candidate-binary',dest,True);manifest=json.loads((dest/'manifest.json').read_text());assert manifest['complete'] and len(manifest['clips'])==48
  for folder in ['scenes','reports','previews']:(dest/folder).mkdir()
  for p in (src/'blender').glob('*.blend'):shutil.copy2(p,dest/'scenes'/p.name)
  for p in (src/'reports').glob('*.json'):shutil.copy2(p,dest/'reports'/p.name)
  for pattern in ['*-contact-native.png','*-four-directions.gif']:
   for p in (src/'candidate-binary').glob(pattern):shutil.copy2(p,dest/'previews'/p.name)
  for clip in manifest['clips']:
   clip['frames']=[sex+'/'+f for f in clip['frames']];result['clips'].append(clip)
  for key,prov in manifest['provenance'].items():
   gait=key[len(sex)+1:].rsplit('-',1)[0];scene=dest/'scenes'/f'{sex}-{gait}-projection.blend';report=dest/'reports'/f'{sex}-{gait}-projection.json';j=json.loads(report.read_text())
   assert j['raster_pass']['filter_type']=='BOX' and j['raster_pass']['filter_width']==1
   assert j['render_complete'];prov.update({'scene':str(scene.relative_to(OUT)).replace('\\','/'),'scene_sha256':sha(scene),'projection_report':str(report.relative_to(OUT)).replace('\\','/'),'projection_report_sha256':sha(report),'frozen_cohort':cohort});result['provenance'][key]=prov
  atlas=WORK/('v4/appearance-atlas.png' if sex=='male' else 'female-source-prep/appearance-atlas.png');shutil.copy2(atlas,dest/'appearance-atlas.png');result['sources'][sex]={'atlas':sex+'/appearance-atlas.png','atlas_sha256':sha(atlas),'cohort':cohort,'cohort_manifest_sha256':sha(src/'candidate-binary/manifest.json')}
 assert len(result['clips'])==96 and sum(len(c['frames']) for c in result['clips'])==832
 (OUT/'source-paint').mkdir()
 originals=['male-walk-front','male-walk-back','female-walk-front','female-walk-right','female-walk-back','female-walk-left','player-unarmed-idle']
 for name in originals:shutil.copy2(ROOT/'starter-v2/originals'/f'{name}.png',OUT/'source-paint'/f'{name}.png')
 for name in ['call-126.js','call-456.js','call-563.js','call-618.js','male-web-exact-call.js']:shutil.copy2(ROOT/'starter-v2/generation-calls'/name,OUT/'source-paint'/name)
 for name in ['painted-branch-patch.png','provenance.json']:shutil.copy2(WORK/'weapon-source'/name,OUT/'source-paint'/('weapon-'+name))
 wood=ROOT/'starter-combat/originals/male-attack-front-0-3-imagegen-v1.png';shutil.copy2(wood,OUT/'source-paint'/wood.name)
 for name in [audit,visual_review]:shutil.copy2(WORK/name,OUT/Path(name).name)
 result['source_paint_sha256']={str(p.relative_to(OUT)).replace('\\','/'):sha(p) for p in (OUT/'source-paint').iterdir()}
 (OUT/'manifest.json').write_text(json.dumps(result,indent=2))
 (OUT/'README.md').write_text('832 frozen player frames: male/female, unarmed/club, idle/walk/sprint/attack/hit/death, four cardinal directions. All96clips form complete appearance/equipment cohorts. Native96px frames use anchor48,80;128px combat/death frames use64,96, retaining48pixels/metre throughout. All pixels are binaryRGBA with sanitized transparentRGB.\n\nAppearance derives from ImageGen-painted references, not independently invented animation sheets. Depth/visibility-aware projection binds that paint to editable source geometry. Final raster uses Box1.0 consistently; native frames are never resized or individually centered. Source PNGs, exact available generation calls, atlases, packed editable scenes and reports are archived here. Scene/report references in the top manifest are self-contained. Historical input paths inside reports identify original production inputs and are not runtime dependencies.\n\nBoth16-frame attack types contact atindex8 and return to their exact idle endpoints. Punches use real Punch_Cross motion with closed striking fingers. Corpse grounding and exposed skin masks were checked in3D; the female braid has one recorded wider, tapered forward-draped profile across all actions. Final contacts were inspected by player/projection agents; game integration acceptance remains a separate native check.\n\nRuntime packaging must include only imported runtime PNGs/registry, not this editable source archive. Superseded candidate passes are excluded.\n')
 print(json.dumps({'manifest':str(OUT/'manifest.json'),'clips':96,'frames':832}))
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--male-cohort',default='male-v7');parser.add_argument('--female-cohort',default='female-v4')
 parser.add_argument('--output',default='accepted');parser.add_argument('--audit',default='male-v7-female-v4-audit.json')
 parser.add_argument('--visual-review',default='male-v7-female-v4-visual-review.json')
 freeze(**vars(parser.parse_args()))
