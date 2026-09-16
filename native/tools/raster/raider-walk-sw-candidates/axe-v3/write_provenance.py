from pathlib import Path
import hashlib,json

BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[4]
def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def write(path,data):
    path.write_text(json.dumps(data,indent=2)+'\n')

idle=ROOT/'native/client/assets/raster/runtime/raider_sw.png'
for version,original,target in [
    (2,'exec-9992f1d7-444d-4688-881e-325d22164e20.png','raider-walk-sw-d2poses-v1.png'),
    (3,'exec-d0ab7248-b66c-480a-b40b-4a4548a41680.png','raider-walk-sw-axe-v2.png')]:
    stem=f'raider-walk-sw-axe-v{version}'
    source=ROOT/f'native/client/assets/raster/source/{stem}.png'
    prompt=ROOT/f'native/client/assets/raster/prompts/{stem}.txt'
    edit_target=ROOT/f'native/client/assets/raster/source/{target}'
    candidate=ROOT/f'native/tools/raster/raider-walk-sw-candidates/axe-v{version}'
    manifest=ROOT/f'native/tools/raster/{stem}-candidate.json'
    report=candidate/f'{stem}.provenance.json'
    data={
        'schemaVersion':1,
        'generationTool':'built-in image_gen',
        'selectedModel':'Not exposed by tool',
        'modelIdentityReturned':False,
        'source':source.relative_to(ROOT).as_posix(),
        'sourceSha256':sha(source),
        'prompt':prompt.relative_to(ROOT).as_posix(),
        'promptSha256':sha(prompt),
        'submittedPrompt':prompt.read_text().rstrip('\n'),
        'generatedOriginal':'C:/Users/Alex/.codex/generated_images/01a0896f-29de-7452-bb4b-6fb64351c1eb/'+original,
        'references':[
            {'order':1,'path':edit_target.relative_to(ROOT).as_posix(),'sha256':sha(edit_target),'inspectedBeforeCall':True,
             'role':'EDIT TARGET: keep ordered eight walking poses, body placement, camera and character. Repair only the specified equipment/clothing property.'},
            {'order':2,'path':idle.relative_to(ROOT).as_posix(),'sha256':sha(idle),'inspectedBeforeCall':True,
             'role':'ORIGINAL APPEARANCE AND EQUIPMENT ONLY: same anatomical-right axe hand, bronze blade, wooden shaft, charcoal clothes and knee openings; do not copy the standing pose.'}
        ],
        'recipe':{
            'directProjectExample':'native/client/assets/raster/prompts/actors_hero-walk-ne-d2poses-v2.txt',
            'publishedContext':[
                {'url':'https://www.flixly.ai/blog/stop-motion-chatgpt-images-2-5','relevance':'Published previous-image edit target plus opening identity reference, one physical change. Its clay-fox output does not demonstrate our sprite or walk.'},
                {'url':'https://12ui.com/gpt-image-2.5-vs-2','relevance':'Explicit separate jobs for reference images in published UI tests, not a gait benchmark.'}
            ],
            'adaptation':'Preserve our actual motion candidate as edit target and restore identity from the accepted original. V2 repairs the mistakenly empty hand and trousers; V3 changes only the blade silhouette/size after independent review found a small hatchet.',
            'claimBoundary':'These raider edits and their results are local observations, not published proof of this exact workflow or a claimed model variant.'
        },
        'observedOutput':{
            'dimensions':[1575,999],
            'mode':'RGB',
            'alpha':'The request for transparent output failed: the source contains an opaque checker pattern.',
            'facing':'All eight frames remain lower-left SW with the bone mask visible.',
            'phaseReview':'F0/F1 narrow; F2 passing; F3 opens into broad F4/F5; F6 narrows; F7 closes. No repeated half-cycle was inserted or reordered. Close adjacent phases remain.',
            'handReview':'Axe remains attached to the anatomical-right hand, on screen-left in this view; opposite hand free in all eight frames.',
            'appearance':'Charcoal trousers and exposed knee tears restored; natural bronze, ochre, leather and bone remain recognizable.',
            'editLimit':'Broad body/pose silhouettes were retained, but an image edit does not preserve every body pixel or shading value exactly.',
            'acceptance':'V2 superseded: its axe was a smaller square hatchet. V3 reviewed for integration as a whole monster sprite; final review stored beside the candidate.'
        },
        'conversion':{
            'engine':'Actual Pixel Respecter workspace.reconstruct and palettes.reduce_colors through existing import_assets.py',
            'manifest':manifest.relative_to(ROOT).as_posix(),'manifestSha256':sha(manifest),
            'report':report.relative_to(ROOT).as_posix(),'reportSha256':sha(report),
            'nativeCanvas':[80,96],'anchorPx':[40,96],'cellSize':6,'sharedColors':32,
            'commonSourceOrigin':[197,480],'commonColumnStride':406,'commonRowStride':435,
            'geometry':'No per-frame resizing or centering; measured shared placement retains original pose offsets.',
            'alphaCleanup':'Border-connected background removal, followed only by explicit windows for visually inspected enclosed hair-loop and axe/knee background gaps. cleanup-inspection.json records the scopes and exact windows.',
            'paletteScope':'All eight frames, one shared32-color palette; crisp binary alpha.',
            'candidateFolder':candidate.relative_to(ROOT).as_posix()
        },
        'ownership':'Sources, prompts, provenance and raider-only candidates/previews. No runtime, main, equipment, catalog, header or shared importer edits. No raw D2 art copied into the repository.'
    }
    write(prompt.with_suffix('.provenance.json'),data)
print('Wrote source, prompt, reference and conversion provenance for axe v2 and v3.')
