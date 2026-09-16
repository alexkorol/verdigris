from pathlib import Path
import json,hashlib
import numpy as np
from PIL import Image,ImageDraw
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[3]
ART=ROOT/'native/client/assets/raster'
OUT=BASE/'v2'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def item(p,role):return {'path':p.relative_to(ROOT).as_posix(),'sha256':sha(p),'role':role}
def write(p,v):p.write_text(json.dumps(v,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
source=ART/'source/terrain-quiet-stone-v2.png';prompt=ART/'prompts/terrain-quiet-stone-v2.txt'
original=Path('C:/Users/Alex/.codex/generated_images/01a0896f-29de-7452-bb4b-6fb64351c1eb/exec-cb666a6e-ec00-456d-9c63-93dc0247db10.png')
assert sha(original)==sha(source)
refs=[item(OUT/'offset-reference-16x.png','Image1 geometry edit target: exact v1 native32px toroidal offset, enlarged16x.'),item(OUT/'v1-material-reference-16x.png','Image2 material/palette/quiet-mass reference only: original v1 native image enlarged16x.')]
provenance={
 'schema_version':1,'date':'2026-09-10','use_case':'precise-object-edit',
 'tool':'built-in image_gen.imagegen','model_selection':'No image model or variant selector was exposed or claimed.',
 'call_number_in_bounded_task':2,'total_calls_allowed':2,'additional_calls':0,
 'exact_prompt':item(prompt,'Unchanged submitted prompt.'),
 'references':[dict(index=i+1,viewed_before_generation=True,**r) for i,r in enumerate(refs)],
 'reference_preparation':item(OUT/'offset-reference.provenance.json','Exact wrap, enlargement and reference hashes.'),
 'original_output':original.as_posix(),'original_output_sha256':sha(original),
 'saved_source':item(source,'Unmodified generated output, offset arrangement.'),
 'source_dimensions':list(Image.open(source).size),'source_mode':Image.open(source).mode,
 'workflow':[
  {'url':'https://github.com/ianlintner/ai-pixel-art-image-generation/blob/main/scripts/lib/seamless.py','access':'Read original source directly before call, including torus_blend roll, center crossing repair and reverse roll.','original_step':'Half-image offset exposes periodic boundaries, central blending repairs them, then inverse offset restores phase.','adaptation':'Use built-in imagegen to visually repair the crossing; do not copy procedural feathering or edge averaging. This is our adaptation of an existing older-model pipeline, not a published Image2.5 successful seam edit.'},
  {'url':'https://x.com/higgsfield/status/2097514684811554911','access':'Original geometry-guide prompt read during the earlier wight work in this conversation.','adaptation':'Explicit geometry edit target versus material/style reference roles; original is a helix turntable, not terrain.'}
 ],
 'conversion':item(OUT/'import.json','Actual Pixel Respecter auto-grid reconstruction and fixed v1 palette, native64x64 offset-phase output.'),
 'phase_restoration':item(OUT/'phase.provenance.json','Exact negative32px native wrap; no repainting/blending.'),
 'observed_result':{'positive':'Retains broad stone layout, quiet materials, opaque64px field and22 colors from v1.','failures':'Visible square repetition and uneven joints remain. Edge MAE worsened in both axes. Outside-center surface texture changed, so strict localized preservation failed.','measurement_scope':'Outside-band pixel comparison includes reconstruction and sampling, not solely model edits. Edge scores are descriptive; visual review remains decisive.'},
 'reviewed':['Unmodified generated source','Original and offset reference images','Restored source diagnostic','Native1x tile','3x3 repeat enlarged3x','Native hero/wight comparison'],
 'acceptance':'Held candidate; seam-repair objective not achieved. No production promotion or further generation.',
 'root_disposition':'Root instructed preserving v2 as a failed seam repair with no further imagegen. A different lane may test the published deterministic edge_match method on v1 as an independent candidate; that is outside this package.'
}
record=ART/'prompts/terrain-quiet-stone-v2.provenance.json';write(record,provenance)
pair=Image.new('RGB',(384,216),(34,32,29));draw=ImageDraw.Draw(pair)
for i,v in enumerate(('v1','v2')):
 im=Image.open(BASE/v/'repeat-3x3-1x.png').convert('RGB');pair.paste(im,(i*192,24));draw.text((i*192+5,5),v+' exact3x3 native repeat',fill=(236,230,215))
pair.save(OUT/'v1-v2-repeat-1x.png');pair.resize((1152,648),Image.Resampling.NEAREST).save(OUT/'v1-v2-repeat-3x.png')
files=[source,prompt,record,OUT/'offset-reference-native.png',OUT/'offset-reference-16x.png',OUT/'v1-material-reference-16x.png',OUT/'offset-reference.provenance.json',OUT/'import.json',OUT/'import.provenance.json',OUT/'phase.provenance.json',OUT/'terrain_quiet_stone_offset.png',OUT/'terrain_quiet_stone.png',OUT/'source-restored-phase.png',OUT/'metrics.json',OUT/'native-1x.png',OUT/'native-3x.png',OUT/'repeat-3x3-1x.png',OUT/'repeat-3x3-3x.png',OUT/'actors-comparison-1x.png',OUT/'actors-comparison-2x.png',OUT/'v1-v2-repeat-1x.png',OUT/'v1-v2-repeat-3x.png',OUT/'README.md',BASE/'prepare_offset.py',BASE/'restore_phase.py',BASE/'review.py',BASE/'record_v2.py']
deps={
 'schema_version':1,'state':'held_candidate_failed_seam_repair_no_production_promotion',
 'working_directory':ROOT.as_posix(),
 'reconstruct_steps':['native/tools/raster/import_assets.py native/tools/raster/quiet-stone-candidates/v2/import.json','native/tools/raster/quiet-stone-candidates/restore_phase.py','native/tools/raster/quiet-stone-candidates/review.py v2'],
 'python':'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe',
 'direct_inputs':[item(p,'Exact reconstruction/reference/phase input') for p in [source,OUT/'import.json',BASE/'v1/terrain_quiet_stone.png',ROOT/'native/tools/raster/import_assets.py',BASE/'restore_phase.py']],
 'references':refs,
 'upstream_v1_provenance':item(BASE/'dependencies.json','Unchanged first attempt generation/reference/reconstruction chain.'),
 'new_files':[item(p,'Preserved second attempt source/reference/pixel/review artifact.') for p in files],
 'quiet_earth_unchanged_sha256':sha(ART/'runtime/terrain_quiet_earth.png'),
 'candidate_sha256':sha(OUT/'terrain_quiet_stone.png'),
 'limits':'Both image calls consumed. No runtime/catalog/code/build/staging changes. No procedural seam blending or repainting.'
}
assert deps['quiet_earth_unchanged_sha256']=='059b101504c96b84a3541f5021dfd1811a7bb1ddb81ebd40fd03be9dbad2af70'
write(OUT/'dependencies.json',deps)
print(json.dumps({'source_sha256':sha(source),'candidate_sha256':deps['candidate_sha256'],'new_files':len(files),'status':deps['state']}))
