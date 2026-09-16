from pathlib import Path
import hashlib,json
from PIL import Image
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[3]
ART=ROOT/'native/client/assets/raster'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def item(p,role):return {'path':p.relative_to(ROOT).as_posix(),'sha256':sha(p),'role':role}
def write(p,data):p.write_text(json.dumps(data,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
source=ART/'source/terrain-quiet-stone-v1.png'
prompt=ART/'prompts/terrain-quiet-stone-v1.txt'
original=Path('C:/Users/Alex/.codex/generated_images/01a0896f-29de-7452-bb4b-6fb64351c1eb/exec-aeb032d5-737a-4b1c-9785-ce1968a1aa5b.png')
assert sha(source)==sha(original)
earth=ART/'runtime/terrain_quiet_earth.png'
assert sha(earth)=='059b101504c96b84a3541f5021dfd1811a7bb1ddb81ebd40fd03be9dbad2af70'
references=[item(earth,'Image1: quiet detail density, broad value masses, restrained contrast only.'),item(ART/'runtime/gate.png','Image2: limestone upright color/material only; no gate geometry, wood or perspective.')]
context=[item(ART/'runtime/terrain_stone.png','Viewed noisy-floor comparison only; not submitted.'),item(ART/'reviews/2026-09-10-monster-motion/remote-wight-arrived.png','Viewed production scene/context only; not submitted.')]
provenance={
 'schema_version':1,'date':'2026-09-10','baseline_supplied_by_parent':'e0e1beaa9',
 'tool':'built-in image_gen.imagegen','use_case':'stylized-concept','call_count':1,
 'model_selection':'No image model or variant selector was exposed or claimed.',
 'exact_prompt':item(prompt,'Unchanged exact submitted prompt.'),
 'references':[dict(index=i+1,viewed_before_generation=True,**r) for i,r in enumerate(references)],
 'viewed_context':context,'original_output':original.as_posix(),'original_output_sha256':sha(original),
 'saved_source':item(source,'Unmodified generated output copied to project.'),
 'source_dimensions':list(Image.open(source).size),'source_mode':Image.open(source).mode,
 'published_workflow':[
  {'url':'https://12ui.com/gpt-image-2.5-vs-2','access':'Opened original comparison in browser and expanded P12 Film Portfolio Collision prompt and reference roles.',
   'excerpt':'Treat Image 1 as a functional specification, not a design reference',
   'adaptation':'Use explicit reference roles: density/value masses and stone material. Original is interface design; this is our floor adaptation, not creator-tested terrain.'},
  {'url':'https://x.com/higgsfield/status/2097514684811554911','access':'Original expanded geometry-guide prompt read during the preceding wight task in the same conversation.',
   'scope':'Adjacent reference-role evidence: geometry and appearance constraints are separate. Published helix turntable does not validate tile seams.'},
  {'url':'https://www.reddit.com/r/codex/comments/1waxfbk/comment/p8m88mg/','access':'Original game-room comment and API follow-up read directly with web tool before generation.',
   'scope':'Creator reports more stable editing but persistent repetitive microtexture on large surfaces. This is feedback, not a verified corrective prompt.'}
 ],
 'local_observation':{'style':'Broad warm-gray limestone slabs; much less dense outline texture than current stone. Bright value and mottled surfaces remain.','repetition':'Visibly periodic and not seamless; opposite-edge metrics recorded in v1/metrics.json.','actor_review':'Unchanged hero/wight are clearer at native scale; previews are mechanical compositions, not gameplay captures.','alpha':'Requested opaque floor; reconstructed alpha255 throughout.','processing':'Actual external Pixel Respecter mesh auto-grid and24-color reduction;114×121 to64×64 nearest-neighbor terrain fill. No procedural repaint.'},
 'root_review':{'viewed':['v1/actors-comparison-1x.png','v1/repeat-3x3-3x.png'],'verdict':'Noise is reduced, but square repetition and inconsistent seam joints remain obvious. Hold as candidate; no production acceptance.','workflow_limit':'Terrain prompt is our unverified adaptation of reference-role workflows, not a demonstrated seamless terrain recipe.','instruction':'No second image call.'},
 'acceptance':'Held candidate after root visual review; square repeat and inconsistent seam joints prevent production acceptance. No second call, production import or staging.',
 'conversion':item(BASE/'v1/import.json','Exact candidate-only importer manifest.'),
 'conversion_report':item(BASE/'v1/import.provenance.json','Actual engine, package, source and pixel hashes.')
}
record=ART/'prompts/terrain-quiet-stone-v1.provenance.json'
write(record,provenance)
files=[source,prompt,record,BASE/'v1/import.json',BASE/'v1/import.provenance.json',BASE/'v1/terrain_quiet_stone.png',BASE/'v1/metrics.json',BASE/'v1/native-1x.png',BASE/'v1/native-3x.png',BASE/'v1/repeat-3x3-1x.png',BASE/'v1/repeat-3x3-3x.png',BASE/'v1/actors-comparison-1x.png',BASE/'v1/actors-comparison-2x.png',BASE/'review.py',BASE/'record_package.py',BASE/'README.md']
dependencies={
 'schema_version':1,'state':'root_reviewed_held_candidate_repeat_and_seam_failures_no_production_edits',
 'working_directory':ROOT.as_posix(),
 'reconstruct_command':"& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' 'native/tools/raster/import_assets.py' 'native/tools/raster/quiet-stone-candidates/v1/import.json'",
 'direct_inputs':[item(p,'Required reconstruction input') for p in [source,BASE/'v1/import.json',ROOT/'native/tools/raster/import_assets.py']],
 'source_reference_inputs':references,'context_inputs':context,
 'review_actor_inputs':[item(ART/f'runtime/{actor}.png','Unchanged native-size review actor; not a generation reference.') for actor in ('hero_sw','wight_sw')],
 'new_candidate_files':[item(p,'New source/provenance/candidate/review artifact; no staging performed.') for p in files],
 'external_engine':'Exact source hashes/version/package hashes in v1/import.provenance.json. Existing external project called without mutation.',
 'quiet_earth_unchanged_sha256':sha(earth),
 'native_result':{'canvas':[64,64],'visible_colors':23,'alpha':[255,255],'sha256':sha(BASE/'v1/terrain_quiet_stone.png')},
 'root_review':'Root viewed native actor composition and3x3 repeat: quieter, but square repeat and inconsistent seam joints are obvious; candidate only, no second call.',
 'next_step':'Hold this preserved candidate. Root owns any later direction or production decision.'
}
write(BASE/'dependencies.json',dependencies)
print(json.dumps({'source_sha256':sha(source),'candidate_sha256':sha(BASE/'v1/terrain_quiet_stone.png'),'new_candidate_files':len(files),'quiet_earth_unchanged':True}))
