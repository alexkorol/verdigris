"""Hash the exact inputs, selected pixels and review evidence; never edit PNGs."""
from pathlib import Path
import hashlib, json
from PIL import Image

BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[3]
ART=ROOT/'native/client/assets/raster'
SELECTED=BASE/'selected'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def rel(p): return p.relative_to(ROOT).as_posix()
def item(p,role): return {'path':rel(p),'sha256':sha(p),'role':role}
def write(p,v): p.write_text(json.dumps(v,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')

published=[
 {'url':'https://x.com/pureso_studio/status/2097520619193868590',
  'access':'Original creator post and photo2 opened in browser; short Japanese walking prompt read in the original result image.',
  'creator_claim':'Creator reports improved walking, clothing identity and several motions from a free Images2.5 chat. This is a firsthand report, not a benchmark or a wight test.',
  'adaptation':'Retain the short reference-character walking-sheet structure; reduce sixteen4x4 poses to eight4x2, use accepted wight identity and existing hero gait guide, target the native actor scale.'},
 {'url':'https://x.com/higgsfield/status/2097514684811554911',
  'access':'Original post and its full geometry-guide prompt read in browser.',
  'scope':'Published example is an orange helix studio turntable with strict reference geometry, pose, composition and unchanged camera/scale. It does not establish skeletal or human gait accuracy.',
  'adaptation':'Give accepted hero motion the geometry/order role and accepted wight idle the appearance-only role. This is our explicit adaptation, not a published wight recipe.'},
 {'url':'https://x.com/Mayz1169/status/2097535082248671625',
  'access':'Previously read original idle prompt; current raster RESEARCH/PROMPTING reread before the task.',
  'adaptation':'Retain fixed alignment and baseline across cells. Published idle alignment is not proof that the generated walk has a perfect shared pivot.'}
]
output_ids=['exec-596a43d4-c966-4d99-8f58-bc6e1decfc62.png','exec-98179c70-1c72-4f53-97b8-7e2a8cb361d3.png','exec-f5e9ab1c-69c6-45b5-968e-b6b9684fb0fd.png']
refs=[
 [(BASE/'hero-sw-gait-guide-4x.png','Edit target: eight chronological lower-body poses and order; no hero identity/equipment.'),(ART/'runtime/wight_sw.png','Only wight appearance, skull/ribs/cloth/palette reference.')],
 [(ART/'runtime/hero_walk4_sw.png','Strict opposite-contact lower-body geometry guide.'),(BASE/'wight-frame4-reference.png','Exact rawv1 frame4 crop to preserve upper body, clothes and scale.'),(ART/'runtime/wight_sw.png','Original wight identity reference.')],
 [(ART/'runtime/hero_walk4_sw.png','Sole edit target: preserve pose geometry and leg overlap.'),(ART/'runtime/wight_sw.png','Appearance only; do not copy idle pose.')]
]
observations=[
 {'result':'Selected original eight poses after source-preserving alpha reconstruction.',
  'good':'All eight face lower-left, retain bare skull/ribs, curled empty hands and torn brown cloth. Stride opens, closes and transfers weight; all frames are distinct.',
  'limits':'Torso is narrower and more leg is exposed than idle. Cloth partly obscures hip-to-knee ownership; this is not certified perfect anatomical alternation. Phases0/1 and4/5 are close. All generated backgrounds are opaque painted checkers.',
  'review_correction':'The initial same-leading-leg concern based on screen-left foot position was too strong: in a SW walk either forward foot naturally appears lower-left. Anatomical support must follow hip/knee and occlusion. Current verdict is partly obscured ownership, not proven duplicate halves.'},
 {'result':'Rejected isolated clarity repair; not spliced into selected cycle.',
  'limits':'Retained the preceding wight pose closely and did not make the opposite hip-to-knee connection unambiguous. The additional prior-frame edit target may have over-anchored the pose; that is an inference.'},
 {'result':'Rejected isolated geometry-first alternative; not spliced into selected cycle.',
  'good':'Wider contact and more visible femur connection.',
  'limits':'Upper-body and cloth silhouette drifted and became more hunched/thin relative to v1. Root preferred v1 consistency. Alpha still painted checker; the experimental v3 native result is not accepted.'}
]
for i in range(3):
    version=i+1; source=ART/f'source/wight-walk-sw-v{version}.png';prompt=ART/f'prompts/wight-walk-sw-v{version}.txt';im=Image.open(source)
    write(ART/f'prompts/wight-walk-sw-v{version}.provenance.json',{
      'schema_version':1,'date':'2026-09-10','use_case':'style-transfer' if i!=1 else 'identity-preserve',
      'tool':'built-in image_gen.imagegen','model_selection':'No image model or variant selector was exposed or claimed.',
      'prompt':item(prompt,'Exact submitted prompt, unchanged.'),
      'references':[dict(index=j+1,**item(p,role),viewed_before_generation=True) for j,(p,role) in enumerate(refs[i])],
      'original_output':'C:/Users/Alex/.codex/generated_images/01a0896f-29de-7452-bb4b-6fb64351c1eb/'+output_ids[i],
      'saved_source':item(source,'Unmodified built-in output copied into the project.'),
      'source_dimensions':list(im.size),'source_mode':im.mode,'source_actual_alpha':False,
      'published_workflow':published,'observations':observations[i],
      'conversion':rel(SELECTED/'import.json') if i==0 else (rel(BASE/'v3/import.json') if i==2 else None),
      'acceptance':'Root approved cleaned originalv1 eight poses for production import and actual game review; final gameplay acceptance pending.' if i==0 else 'Not selected; preserved experiment.'
    })

review=json.loads((SELECTED/'native-review.json').read_text())
acceptance={
 'date':'2026-09-10','state':'root_native_review_accepted_for_import_game_review_pending',
 'selection':'All eight original v1 poses in original row-major order; no v2/v3 splice.',
 'source_sha256':sha(ART/'source/wight-walk-sw-v1.png'),
 'manifest_sha256':sha(SELECTED/'import.json'),'report_sha256':sha(SELECTED/'import.provenance.json'),
 'canvas':[80,96],'pivot':[40,96],'source_cell_size':6,'source_origin':[210,486],
 'column_stride':405,'row_stride':450,'frame_order':list(range(8)),
 'preview_frame_ms':100,'preview_idle_hold_ms':320,'production_timing':'Parent selects actual runtime cadence.',
 'native_body_height_range':[59,63],'idle_body_height':61,'palette':'30 visible colors, all drawn from accepted wight_sw palette of at most32.',
 'source_alpha':'RGB painted checker; reconstructed via actual engine with reviewed neutral interior windows, no retained RGB changes.',
 'frozen_pngs':review['frames'],
 'visual_checks':['Original and selected alpha pairs reviewed for all eight poses.','Phase1 pale skeletal hand survives; enclosed arm/finger gaps are transparent.','All eight native poses and1x/3x idle transitions viewed.','Browser1x/3x ordered playback uses fixed canvas and original order.'],
 'caveats':['Narrower torso and more exposed legs than accepted idle.','Hip-to-knee ownership is partly obscured by cloth; do not claim perfect alternating anatomy.','Phases0/1 and4/5 are relatively close; actual production motion remains the acceptance gate.','Native ground bottoms93–95 versus idle96; preserve shared source registration and phase lift rather than snapping each pose.'],
 'root_authorization':'Root viewed selected native1x/3x strips and approved exact cleaned v1SW8 for production import/runtime review. Renderer owns active manifest/runtime/catalog; this lane changes none of those.',
 'final_gameplay_review':False
}
write(SELECTED/'acceptance.json',acceptance)

source_deps=[ART/'source/wight-walk-sw-v1.png',ART/'prompts/wight-walk-sw-v1.txt',ART/'prompts/wight-walk-sw-v1.provenance.json',BASE/'hero-sw-gait-guide-4x.png',BASE/'hero-sw-gait-guide.provenance.json',BASE/'build_guide.py',ROOT/'native/tools/raster/import_assets.py',ART/'runtime/wight_sw.png',*[ART/f'runtime/hero_walk{i}_sw.png' for i in range(8)]]
selected_files=[SELECTED/'import.json',SELECTED/'import.provenance.json',SELECTED/'acceptance.json',SELECTED/'native-review.json',SELECTED/'alpha-recipe-review.json',SELECTED/'alpha-audit.json',SELECTED/'idle-cycle-idle-1x.png',SELECTED/'idle-cycle-idle-3x.png',SELECTED/'contact-3x.png',SELECTED/'idle-cycle-idle.gif',SELECTED/'walk-loop.gif',SELECTED/'alpha-source-comparison.png',SELECTED/'hand-alpha-comparison.png',SELECTED/'chromatic-edge-review.png',SELECTED/'review_walk.py',SELECTED/'audit_alpha.py',SELECTED/'playback.html',BASE/'alpha-study/build_safe_recipe.py',BASE/'alpha-study/neutral-components.json',BASE/'alpha-study/neutral-gap-review.png']
selected_files.extend([SELECTED/'README.md',SELECTED/'idle-reference.png',BASE/'alpha-study/prepare_alpha.py',BASE/'v1/import.json'])
pngs=[SELECTED/f'wight_walk{i}_sw.png' for i in range(8)]
deps={
 'schema_version':1,'state':'frozen_selected_pixels_root_import_authorized',
 'reconstruct_command':"& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' 'native/tools/raster/import_assets.py' 'native/tools/raster/wight-walk-sw-candidates/selected/import.json'",
 'working_directory':ROOT.as_posix(),
 'reconstruction_direct_inputs':[item(p,'Required exact reconstruction input') for p in [SELECTED/'import.json',ART/'source/wight-walk-sw-v1.png',ART/'runtime/wight_sw.png',ROOT/'native/tools/raster/import_assets.py']],
 'generation_provenance_inputs':[item(p,'Source, original prompt, actual reference or reproducible reference assembly') for p in source_deps],
 'selected_output_pngs':[item(p,'Frozen candidate pixels; renderer must verify byte-identical runtime import.') for p in pngs],
 'review_and_recipe_evidence':[item(p,'Selected alpha/native/motion review and recipe evidence') for p in selected_files],
 'external_engine':'Exact external engine source hashes and Python/package versions are recorded in selected/import.provenance.json; external project was read and called, never modified.',
 'runtime_promotion_owner':'raster_renderer; no runtime/catalog/main edits by this asset lane.'
}
write(SELECTED/'dependencies.json',deps)
existing=[ROOT/'native/tools/raster/import_assets.py',ART/'runtime/wight_sw.png',*[ART/f'runtime/hero_walk{i}_sw.png' for i in range(8)]]
stage=[p for p in source_deps if p not in existing]+selected_files+pngs+[SELECTED/'dependencies.json',BASE/'record_package.py']
(SELECTED/'staging-selected.txt').write_text('\n'.join(dict.fromkeys(rel(p) for p in stage))+'\n')
experiments=[*[ART/f'source/wight-walk-sw-v{i}.png' for i in (2,3)],*[ART/f'prompts/wight-walk-sw-v{i}.{ext}' for i in (2,3) for ext in ('txt','provenance.json')],BASE/'wight-frame4-reference.png',BASE/'contact-ownership-comparison-4x.png',BASE/'v1/import.json',BASE/'v1/import.provenance.json',BASE/'v1/alpha-audit.json',BASE/'v1/alpha-source-comparison.png',BASE/'v3/import.json',BASE/'v3/import.provenance.json',BASE/'v3/wight_walk4_sw.png']
write(SELECTED/'experiment-dependencies.json',{'state':'Rejected alternatives/cleanup evidence; never runtime inputs.','files':[item(p,'Preserved failed pose/alpha experiment') for p in experiments]})
(SELECTED/'staging-experiments.txt').write_text('\n'.join(rel(p) for p in experiments)+'\n'+rel(SELECTED/'experiment-dependencies.json')+'\n')
print(json.dumps({'selected_manifest_sha256':sha(SELECTED/'import.json'),'source_sha256':sha(ART/'source/wight-walk-sw-v1.png'),'selected_staging_count':len(set(stage)),'dependencies':rel(SELECTED/'dependencies.json')}))
