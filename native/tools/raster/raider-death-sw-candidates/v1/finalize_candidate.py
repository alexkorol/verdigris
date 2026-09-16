from pathlib import Path
import hashlib,json
import numpy as np
from PIL import Image
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
TOOLS=ROOT/'native/tools/raster'
RASTER=ROOT/'native/client/assets/raster'
def raw_sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def rec(p,role):
    b=p.read_bytes();binary=p.suffix.lower() in ('.png','.gif')
    return {'path':p.relative_to(ROOT).as_posix(),'role':role,'sha256':hashlib.sha256(b if binary else b.replace(b'\r\n',b'\n')).hexdigest(),'hash_policy':'raw_binary' if binary else 'canonical_lf_crlf_to_lf_only'}
source=RASTER/'source/raider-death-sw-v1.png';prompt=RASTER/'prompts/raider-death-sw-v1.txt';idle=RASTER/'runtime/raider_sw.png'
manifest=TOOLS/'raider-death-sw-candidate.json';report=BASE/'raider-death-sw-candidate.provenance.json'
conversion=json.loads(report.read_text())
colors=set()
for i,entry in enumerate(conversion['assets']):
    path=BASE/f'raider_death{i}_sw.png';a=np.array(Image.open(path).convert('RGBA'))
    assert raw_sha(path)==entry['sha256']
    assert entry['before_resize']==entry['placed_size']
    assert entry['anchor_px']==[64,96]
    assert set(np.unique(a[:,:,3])).issubset({0,255})
    colors.update(map(tuple,a[:,:,:3][a[:,:,3]>0]))
assert len(colors)<=32
provenance={'version':1,'use_case':'stylized-concept','tool':'built-in image_gen','selected_model':'Tool exposes no selector and returned no specific model identity.','generation_calls':1,
    'source':source.relative_to(ROOT).as_posix(),'source_raw_sha256':raw_sha(source),'dimensions':[1437,1094],'original_mode':'RGB',
    'generated_original':'C:/Users/Alex/.codex/generated_images/01a0896f-29de-7452-bb4b-6fb64351c1eb/exec-136c0083-6321-4945-9ee5-39567185ff0d.png',
    'prompt':prompt.relative_to(ROOT).as_posix(),'prompt_raw_sha256':raw_sha(prompt),'submitted_prompt':prompt.read_text().rstrip('\n'),
    'references':[{'order':1,'path':idle.relative_to(ROOT).as_posix(),'raw_sha256':raw_sha(idle),'role':'Sole supplied character, camera, pixel style, clothing and baked right-hand axe identity reference.','inspected_before_generation':True}],
    'viewed_but_not_supplied':['native/client/assets/raster/runtime/raider_strike0_sw.png'],
    'recipe':{'primary_source':'https://x.com/Mayz1169/status/2097540160611287452','original_inspection':'Complete Kiki planned combat prompt was read directly in the browser earlier in this same lane; current PROMPTING.md and RESEARCH.md reread before this call.','retained_structure':'Single reference identity and equipment, explicit ordered action beats, actual body mechanics, fixed camera/apparent scale, generous weapon clearance and separately authored frame holds.','local_prior_recipe':'native/client/assets/raster/prompts/raider-strike-sw-v1.txt','adaptation':'Replace combat preparation/attack/contact/recovery with four collapse beats: recoil, knees buckle, torso falls, body settles. Use a 2x2 adult raider study, no chibi conversion, gore or effects. Tools perform slicing, alpha cleanup, native reconstruction and preview.','claim_boundary':'Neither Kiki nor other cited creators are claimed to have tested this death sequence, adult raider or production integration. These are local observed results.'},
    'source_review':{'identity':'Recognizable bone mask, tied dark hair, charcoal clothes, ochre sash, brown boots and warm bronze axe; torn knee openings remain. No green bias observed.','motion':'Four distinct SW collapse poses: recoil backward, deep buckle, hands/knees fall, and fully prone body with head lower-left and legs trailing upper-right. Same anatomical-right axe arm remains screen-left; the final handle meets the loosened hand. Opposite forearm breaks the fall.','failure':'Actual alpha request returned a painted opaque checker. Nine enclosed-gap scopes retained background after border cleanup; these were inspected against the original before neutral-only local removal.','iteration':'No second generation required. Initial border-only candidate is preserved in the local border-only folder.'},
    'conversion':{'manifest':manifest.relative_to(ROOT).as_posix(),'manifest_raw_sha256':raw_sha(manifest),'report':report.relative_to(ROOT).as_posix(),'report_raw_sha256':raw_sha(report),'engine':'Actual Pixel Respecter workspace.reconstruct and palettes.reduce_colors through existing import_assets.py','cell_size':8,'shared_colors':len(colors),'canvas':[128,112],'pivot':[64,96],'standing_reference_original_pivot':[40,96],'standing_reference_translation':[24,0],'standing_reference_rescale':False,'source_origin':[400,608],'source_column_stride':704,'source_row_stride':416,'normalization':'One shared cell8 scale and row/column registration; no independent pose fitting, resizing or centering.','alpha_audit':'native/tools/raster/raider-death-sw-candidates/v1/alpha-audit.json'},
    'acceptance':'Usable native candidate for root review; no runtime promotion or production acceptance.',
    'limitations':['Recoil visible body height61px versus idle57px, including the raised/backward head pose; small material and silhouette variation remains.','Four key poses form a rapid collapse, not interpolated high-frame-rate motion. Preview uses recoil100ms, buckle130ms, fall110ms, then holds settled.','Preview restarts from idle for inspection only; production must play once and retain settled frame3 at the recorded world pivot.','Production death-event timing, corpse sorting/occlusion and gameplay gates remain root-owned.']}
provpath=RASTER/'prompts/raider-death-sw-v1.provenance.json';provpath.write_text(json.dumps(provenance,indent=2)+'\n')
acceptance={'status':'native_candidate_ready_for_root_review','source_and_native_reviewed':True,'scales':[1,3],'backgrounds':['dark','light'],'browser_preview':'Opened animated native/3x preview and inspected the settled body; all four phase silhouettes also reviewed in native and 3x strips. This is not a production game capture.','phases':['recoil','buckle','fall','settled'],'axe_hand':'same anatomical-right hand; screen-left arm in SW poses, blade ends beside hand','canvas':[128,112],'pivot':[64,96],'shared_colors':len(colors),'standing_body_height':61,'idle_body_height':57,'idle_translation':[24,0],'fallen_visible_bounds':[17,72,100,97],'preview_action_durations_ms':[100,130,110],'settled_frame':3,'settled_hold':'indefinite in production;1000ms in repeating inspection preview','alpha':'binary RGBA; no remaining bright neutral gap pixels; zero chromatic pixels removed by scoped cleanup','runtime_written':False,'production_review_pending':True}
(BASE/'acceptance-review.json').write_text(json.dumps(acceptance,indent=2)+'\n')
dependencies={'version':1,'scope':'Raider SW4 death candidate only; no runtime/catalog/code changes.','hash_policy':'Text uses CRLF-to-LF canonical bytes only; PNG uses raw bytes. Exact generation raw hashes remain historical observations.','active_inputs':[rec(source,'sole generated source for all four selected poses')],'active_recipes':[rec(manifest,'complete candidate recipe including exact neutral-only alpha windows'),rec(TOOLS/'import_assets.py','actual Pixel Respecter adapter')],'historical_provenance':[rec(prompt,'exact submitted prompt'),rec(provpath,'generation reference roles and observed outcome'),rec(idle,'accepted sole identity reference'),rec(report,'actual reconstruction provenance including external engine fingerprints')],'review_evidence':[rec(BASE/name,'native/alpha candidate review') for name in ['acceptance-review.json','native-review.json','alpha-audit.json','source-gap-inspection.png','contact-3x.png','idle-death-1x.png','idle-death-3x.png','idle-death-preview.gif']],'external_project':{'path':'Z:/Code/Python/pixel-perfecter','vendored':False,'source_fingerprints_in':report.relative_to(ROOT).as_posix()},'reproduction':'Run existing import_assets.py with raider-death-sw-candidate.json. No rejected intermediate file or Python repainting is required.'}
deppath=TOOLS/'raider-death-sw-dependencies.json';deppath.write_text(json.dumps(dependencies,indent=2)+'\n')
paths=[source,prompt,provpath,manifest,deppath]+[p for p in BASE.iterdir() if p.is_file() and p.suffix.lower() in ('.png','.gif','.json','.py','.html')]
staging=TOOLS/'raider-death-sw-staging-paths.txt'
staging.write_text('\n'.join(sorted(set(p.relative_to(ROOT).as_posix() for p in paths)))+'\n'+staging.relative_to(ROOT).as_posix()+'\n')
print(json.dumps({'colors':len(colors),'frames':4,'candidate_dependencies':deppath.relative_to(ROOT).as_posix(),'candidate_verdict':acceptance['status'],'idle_unchanged':raw_sha(idle)=='13e515e678d001de0e764729048986d1d459c9df2de9c2b52d2d1c8538b3daf4'}))
