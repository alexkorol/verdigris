"""Reconstruct the reference-led lineage sprites through actual Pixel Respecter.

Generation originals are immutable. This staging step removes only the measured
neutral checker matte, records that repair, and preserves one scale and pivot
for every animation in a direction. No silhouette warping or per-pose fitting.
"""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import os
import sys

import cv2
import numpy as np
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[3]
DIRECTIONS = ['n', 'ne', 'e', 'se', 's', 'sw', 'w', 'nw']
POSES = ['idle', 'walk0', 'walk1', 'strike0', 'strike1']
spec = importlib.util.spec_from_file_location('raster_import', ROOT.parent / 'import_assets.py')
importer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(importer)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def stage(path, destination):
    source = Image.open(path)
    data = np.asarray(source.convert('RGBA')).copy()
    had_alpha = source.mode == 'RGBA' and np.any(data[:, :, 3] < 255)
    if had_alpha:
        mask = (data[:, :, 3] >= 80).astype(np.uint8)
        method = 'original_alpha_threshold80_largest_connected_body'
    else:
        rgb = data[:, :, :3].astype(np.int16)
        neutral = (rgb.max(axis=2)-rgb.min(axis=2) <= 18) & (rgb.min(axis=2) >= 120)
        mask = (~neutral).astype(np.uint8)
        method = 'neutral_checker_chroma18_min120_largest_connected_body'
    count, labels, stats, _ = cv2.connectedComponentsWithStats(mask, connectivity=8)
    if count <= 1:
        raise ValueError(f'No body found: {path}')
    label = 1 + int(np.argmax(stats[1:, cv2.CC_STAT_AREA]))
    keep = labels == label
    body = stats[label]
    if body[cv2.CC_STAT_AREA] < data.shape[0]*data.shape[1]*.025:
        raise ValueError(f'Body extraction too small: {path}')
    data[~keep] = 0
    data[keep, 3] = 255
    destination.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(data).save(destination)
    x, y, w, h, area = map(int, body)
    return {'source': os.path.relpath(path, ROOT).replace('\\', '/'),
            'source_sha256': digest(path), 'source_mode': source.mode,
            'source_size': list(source.size), 'actual_source_alpha': bool(had_alpha),
            'repair': method, 'body_bounds': [x, y, x+w, y+h], 'body_pixels': area,
            'removed_component_pixels': int(mask.sum())-area,
            'derived': os.path.relpath(destination, ROOT).replace('\\', '/'),
            'derived_sha256': digest(destination)}


def process(sex, direction, project):
    folder = ROOT / 'candidates' / sex / direction
    idle = folder / 'idle.png'
    if not idle.exists():
        return []
    repair_file = ROOT/'derived'/sex/direction/'repair.json'
    report_file = ROOT/'processed'/sex/direction/f'hero-{sex}-{direction}.provenance.json'
    current = {pose:digest(folder/(pose+'.png')) for pose in POSES if (folder/(pose+'.png')).exists()}
    if repair_file.exists() and report_file.exists():
        old = json.loads(repair_file.read_text())
        if current == {r['pose']:r['source_sha256'] for r in old['sources']}:
            print(f'{sex}-{direction}: source hashes unchanged; retaining reconstructed cycle.')
            return json.loads(report_file.read_text())['assets']
    records, sheets = [], []
    for pose in POSES:
        source = folder / (pose+'.png')
        if not source.exists():
            continue
        derived = ROOT / 'derived' / sex / direction / (pose+'.png')
        record = stage(source, derived)
        record['pose'] = pose
        records.append(record)
        name = f'hero_{sex}_{direction}' if pose == 'idle' else f'hero_{sex}_{pose}_{direction}'
        sheets.append({'source': os.path.relpath(derived, ROOT).replace('\\', '/'),
                       'grid': [1, 1], 'assets': [{'name': name, 'cell': [0, 0]}]})
    idle_record = records[0]
    # The directional idle establishes registration once for its entire clip.
    # The canvas center is the rotation axis; the lowest idle sole is the floor.
    anchor = [idle_record['source_size'][0]//2, idle_record['body_bounds'][3]]
    manifest = {'version': 1, 'output_dir': f'processed/{sex}/{direction}',
                'report_name': f'hero-{sex}-{direction}.provenance.json',
                'shared_palette_max_colors': 96,
                'defaults': {'cell_size': 10, 'alpha_mode': 'crisp', 'canvas': [160, 192],
                             'padding': 32, 'fit': [96, 128], 'trim': True,
                             'require_transparency': True, 'preserve_scale': True,
                             'anchor_source': anchor,
                             'acceptance': 'staged_pending_visual_and_runtime_review'},
                'known_limitations': ['Generated RGB checker mattes require explicitly recorded neutral removal.',
                                      'One directional idle defines each clip pivot; source scale variations remain measurable.',
                                      'Attack recovery reuses the accepted idle pose; two other poses are independently generated.'],
                'sheets': sheets}
    manifest_path = ROOT / f'hero-{sex}-{direction}.json'
    manifest_path.write_text(json.dumps(manifest, indent=2)+'\n')
    repair_path = ROOT / 'derived' / sex / direction / 'repair.json'
    repair_path.write_text(json.dumps({'anchor_source': anchor, 'sources': records}, indent=2)+'\n')
    output = ROOT / 'processed' / sex / direction
    report = importer.run(manifest_path, project, preview_path=ROOT/'previews'/f'{sex}-{direction}-3x.png', preview_scale=3)
    # Retain a real source-backed third recovery phase, explicitly identified.
    if len(sheets) == 5:
        recovery = output/f'hero_{sex}_strike2_{direction}.png'
        recovery.write_bytes((output/f'hero_{sex}_{direction}.png').read_bytes())
        (output/'recovery.json').write_text(json.dumps({'file': recovery.name,
            'source': f'hero_{sex}_{direction}.png', 'method': 'byte_identical_idle_recovery',
            'sha256': digest(recovery)}, indent=2)+'\n')
    return report['assets']



def build_previews():
    """Review only already imported frames; never count files as acceptance."""
    previews = ROOT / 'previews'
    previews.mkdir(exist_ok=True)
    reports, by_name = [], {}
    for sex in ['male', 'female']:
        for direction in DIRECTIONS:
            path = ROOT / 'processed' / sex / direction / f'hero-{sex}-{direction}.provenance.json'
            if not path.exists():
                continue
            report = json.loads(path.read_text())
            for record in report['assets']:
                image = path.parent / record['file']
                by_name[record['name']] = image
                reports.append({**record, 'sex': sex, 'direction': direction,
                                'processed': image.relative_to(ROOT).as_posix()})
            recovery = path.parent / f'hero_{sex}_strike2_{direction}.png'
            if recovery.exists():
                by_name[recovery.stem] = recovery

    def panel(name, scale=1, duration=None):
        frame = Image.new('RGB', (160, 212), (41, 39, 37))
        draw = ImageDraw.Draw(frame)
        draw.line((0, 180, 159, 180), fill=(85, 78, 63))
        draw.line((80, 176, 80, 184), fill=(126, 119, 95))
        draw.text((4, 4), name.replace('hero_', ''), fill=(224, 214, 198))
        if name in by_name:
            art = Image.open(by_name[name]).convert('RGBA')
            frame.paste(art, (0, 20), art)
        else:
            draw.text((22, 96), 'PENDING SOURCE', fill=(160, 142, 112))
        return frame.resize((160*scale, 212*scale), Image.Resampling.NEAREST)

    for sex in ['male', 'female']:
        for scale in [1, 3]:
            sheet = Image.new('RGB', (640*scale, 424*scale))
            for i, direction in enumerate(DIRECTIONS):
                sheet.paste(panel(f'hero_{sex}_{direction}', scale), ((i%4)*160*scale, (i//4)*212*scale))
            sheet.save(previews/f'{sex}-idle-eight-{scale}x.png')
    both = Image.new('RGB', (1280, 424))
    for j, sex in enumerate(['male', 'female']):
        for i, direction in enumerate(DIRECTIONS):
            both.paste(panel(f'hero_{sex}_{direction}'), (i*160, j*212))
    both.save(previews/'male-female-idle-eight-1x.png')

    loops = []
    for sex in ['male', 'female']:
        for direction in DIRECTIONS:
            for action, phases, timing in [('walk', ['walk0', 'walk1'], [140, 140]),
                                           ('attack', ['strike0', 'strike1', 'strike2'], [80, 100, 160])]:
                names = [f'hero_{sex}_{pose}_{direction}' for pose in phases]
                if not all(name in by_name for name in names):
                    continue
                for scale in [1, 3]:
                    frames = [panel(name, scale) for name in names]
                    target = previews/f'{sex}-{direction}-{action}-{scale}x.gif'
                    frames[0].save(target, save_all=True, append_images=frames[1:], duration=timing,
                                   loop=0, disposal=2, optimize=False)
                    strip = Image.new('RGB', (160*scale*len(frames), 212*scale))
                    for i, frame in enumerate(frames): strip.paste(frame, (i*160*scale, 0))
                    strip.save(previews/f'{sex}-{direction}-{action}-{scale}x.png')
                loops.append({'sex': sex, 'direction': direction, 'action': action,
                              'frames': names, 'durations_ms': timing, 'status': 'pending_playback_review'})
    for action, poses, timing in [('walk',['walk0','walk1'],[140,140]),
                                   ('attack',['strike0','strike1','strike2'],[80,100,160])]:
        for scale in [1,2]:
            frames = []
            for pose in poses:
                sheet = Image.new('RGB',(1280*scale,424*scale))
                for j,sex in enumerate(['male','female']):
                    for i,direction in enumerate(DIRECTIONS):
                        sheet.paste(panel(f'hero_{sex}_{pose}_{direction}',scale),(i*160*scale,j*212*scale))
                frames.append(sheet)
            frames[0].save(previews/f'all-sixteen-{action}-{scale}x.gif',save_all=True,
                           append_images=frames[1:],duration=timing,loop=0,disposal=2,optimize=False)
    index = {'native_canvas': [160,192], 'anchor_px': [80,160], 'pixel_pitch': 10,
             'palette': '96 colors shared per direction', 'assets': reports, 'loops': loops}
    (ROOT/'review-index.json').write_text(json.dumps(index, indent=2)+'\n')
    html = ['<!doctype html><meta charset="utf-8"><title>Lineage motion review</title>',
            '<style>body{background:#292725;color:#ddd;font:14px sans-serif}section{display:flex;flex-wrap:wrap}figure{margin:8px}img{image-rendering:pixelated}h2{width:100%}</style>',
            '<h1>Imported lineage candidates — review playback</h1>',
            '<p>Shared 160×192 canvas, anchor 80,160; GIF timing is a review fixture. These previews require separate hash-pinned runtime promotion.</p>']
    for sex in ['male','female']:
        html.append(f'<h2>{sex} — eight idle directions</h2><img src="{sex}-idle-eight-1x.png">')
        html.append('<section>')
        for loop in loops:
            if loop['sex'] != sex: continue
            stem=f"{sex}-{loop['direction']}-{loop['action']}"
            html.append(f'<figure><figcaption>{stem}</figcaption><img width="320" height="424" src="{stem}-1x.gif"></figure>')
        html.append('</section>')
    (previews/'index.html').write_text('\n'.join(html), encoding='utf-8')
    print(f'Review files assembled for {len(reports)} poses and {len(loops)} playable loops; visual status pending.')



def promote_approved():
    """Copy only explicit hash-pinned root approvals into additive runtime names."""
    import shutil
    approvals = json.loads((ROOT/'approvals.json').read_text())
    runtime = REPO/'native/client/assets/raster/runtime'
    source_root = REPO/'native/client/assets/raster/source/hero-lineage-20260910'
    reports = {}
    for sex in ['male','female']:
        for direction in DIRECTIONS:
            path = ROOT/'processed'/sex/direction/f'hero-{sex}-{direction}.provenance.json'
            if path.exists(): reports[(sex,direction)] = json.loads(path.read_text())
    records, provenance = [], []
    for approval in approvals['assets']:
        sex, direction, pose = approval['sex'], approval['direction'], approval['pose']
        report = reports[(sex,direction)]
        name = f'hero_{sex}_{direction}' if pose == 'idle' else f'hero_{sex}_{pose}_{direction}'
        image = ROOT/'processed'/sex/direction/(name+'.png')
        if digest(image) != approval['sha256']:
            raise ValueError(f'Approved pixels changed and need new review: {name}')
        original_pose = 'idle' if pose == 'strike2' else pose
        original_name = f'hero_{sex}_{direction}' if original_pose == 'idle' else f'hero_{sex}_{original_pose}_{direction}'
        record = dict(next(a for a in report['assets'] if a['name'] == original_name))
        record.update(name=name, file=name+'.png', sha256=approval['sha256'],
                      acceptance='root_visual_approved_pending_production_capture')
        if pose == 'strike2': record['recovery_method'] = 'byte_identical_idle_recovery'
        repairs = json.loads((ROOT/'derived'/sex/direction/'repair.json').read_text())
        repair = next(r for r in repairs['sources'] if r['pose'] == original_pose)
        for kind in ['candidates', 'derived']:
            source = ROOT/kind/sex/direction/(original_pose+'.png')
            target = source_root/kind/sex/direction/source.name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)
            record['original_source' if kind=='candidates' else 'source'] = os.path.relpath(target,runtime).replace('\\','/')
        record['original_source_sha256'] = repair['source_sha256']
        record['derived_source_sha256'] = repair['derived_sha256']
        record['matte_repair'] = repair['repair']
        record['directional_provenance'] = f'hero-lineage-20260910/hero-{sex}-{direction}.provenance.json'
        shutil.copyfile(image, runtime/image.name)
        records.append(record)
        provenance.append({'sex':sex,'direction':direction,'report':report})
    evidence = runtime/'hero-lineage-20260910'
    evidence.mkdir(exist_ok=True)
    for (sex,direction),report in reports.items():
        if any(a['sex']==sex and a['direction']==direction for a in approvals['assets']):
            (evidence/f'hero-{sex}-{direction}.provenance.json').write_text(json.dumps(report,indent=2)+'\n')
    result={'version':1,'assets':records,'max_colors':96,'palette_scope':'direction_cycle',
            'known_limitations':['Two walk contacts only; no passing phase.',
              'Attack recovery is byte-identical idle.',
              'Generated checker backgrounds were removed by documented source-preserving staging.',
              'Per-direction source scale is preserved; cross-direction height variation remains.',
              'Root production capture and equipment overlay acceptance remain separate.'],
            'approval_file_sha256':digest(ROOT/'approvals.json'),'prepare_sha256':digest(Path(__file__))}
    (runtime/'hero-lineage-20260910.provenance.json').write_text(json.dumps(result,indent=2)+'\n')
    print(f'Promoted {len(records)} approved additive lineage PNGs; legacy runtime names unchanged.')



def build_equipment():
    """Map reviewed source wrist hints through the actual importer geometry."""
    import math
    entries, pending = [], []
    headings = {'n':0,'ne':45,'e':90,'se':135,'s':180,'sw':225,'w':270,'nw':315}
    for sex in ['male','female']:
        for direction in DIRECTIONS:
            folder = ROOT/'candidates'/sex/direction
            manifest_path = folder/'manifest.json'
            if not manifest_path.exists(): manifest_path = folder/'source-manifest.json'
            provenance_path = ROOT/'processed'/sex/direction/f'hero-{sex}-{direction}.provenance.json'
            if not manifest_path.exists() or not provenance_path.exists():
                pending.append(f'{sex}-{direction}: awaiting source wrist manifest/import')
                continue
            source_manifest = json.loads(manifest_path.read_text(encoding='utf-8-sig'))
            report = json.loads(provenance_path.read_text())
            frames = source_manifest.get('frames', source_manifest.get('poses', source_manifest.get('assets', source_manifest.get('selected', []))))
            for frame in frames:
                pose = frame.get('pose')
                if pose not in POSES: continue
                wrist = frame.get('anatomical_left_wrist_px_approx', frame.get('left_wrist_approx_source_xy', frame.get('approximate_anatomical_left_wrist_pixels', frame.get('left_wrist_px', frame.get('left_wrist_source_px',frame.get('left_wrist',{}).get('source_px', frame.get('left_wrist',{}).get('source_xy')))))))
                point = frame.get('left_grip_px', wrist)
                if point is None:
                    pending.append(f'{sex}-{direction}-{pose}: missing wrist coordinate')
                    continue
                name = f'hero_{sex}_{direction}' if pose=='idle' else f'hero_{sex}_{pose}_{direction}'
                asset = next((a for a in report['assets'] if a['name']==name), None)
                if asset is None: continue
                source = folder/(pose+'.png')
                expected_hash = frame.get('sha256', frame.get('source_sha256'))
                if expected_hash and expected_hash != digest(source):
                    pending.append(f'{name}: source manifest hash stale')
                    continue
                repair_data = json.loads((ROOT/'derived'/sex/direction/'repair.json').read_text())
                imported_source = next(r for r in repair_data['sources'] if r['pose']==pose)
                if imported_source['source_sha256'] != digest(source):
                    pending.append(f'{name}: processed pixels precede current source; reimport needed')
                    continue
                # normalize() translates the reconstructed grid, without scaling.
                bounds, trim = asset['content_bounds'], asset['trim_box']
                offset, pitch = asset['grid_offset'], asset['cell_size']
                ratio = [asset['placed_size'][i]/asset['before_resize'][i] for i in range(2)]
                mapped = [bounds[i]+((point[i]-offset[i])/pitch-trim[i])*ratio[i] for i in range(2)]
                hand = [math.floor(v)+.5 for v in mapped]
                behind = frame.get('behind_actor', frame.get('left_hand_behind_actor', frame.get('weapon_behind_body',frame.get('held_item_behind_body_hint',frame.get('left_wrist',{}).get('behind_body', direction in ['ne','e'])))))
                angle = frame.get('melee_clockwise_degrees', frame.get('equipment_angle_clockwise_degrees', frame.get('suggested_equipment_angle_degrees', headings[direction] if pose=='strike1' else 0)))
                if pose.startswith('strike') and 'held_item_angle_hint_degrees_clockwise_screen' in frame:
                    angle = (frame['held_item_angle_hint_degrees_clockwise_screen']+90)%360
                if pose.startswith('strike') and 'forearm_screen_angle_degrees' in frame.get('left_wrist',{}):
                    angle = (frame['left_wrist']['forearm_screen_angle_degrees']+90)%360
                item={'name':name,'canvas':[160,192],'hand':hand,'hand_x':hand[0],'hand_y':hand[1],
                      'source_wrist':wrist,'source_grip':frame.get('left_grip_px'),'source_wrist_uncertainty_px':frame.get('wrist_uncertainty_px', frame.get('wrist_precision_px',15)),
                      'source':source.relative_to(ROOT).as_posix(),'source_sha256':digest(source),
                      'processed_sha256':asset['sha256'],'source_manifest':manifest_path.relative_to(ROOT).as_posix(),
                      'source_manifest_sha256':digest(manifest_path),'grid_offset':offset,'cell_size':pitch,
                      'canvas_translation':[bounds[i]-trim[i] for i in range(2)],
                      'faces_left':direction in ['sw','w','nw'],'behind_actor':bool(behind),
                      'melee_clockwise_degrees':int(angle),'melee_behind_actor':bool(behind),
                      'fingers':[int(hand[0])-1,int(hand[1])-1,int(hand[0])+2,int(hand[1])+2],
                      'status':'source_wrist_mapped_candidate_pending_equipment_overlay_review'}
                entries.append(item)
                if pose=='idle' and (ROOT/'processed'/sex/direction/f'hero_{sex}_strike2_{direction}.png').exists():
                    recovery=dict(item); recovery['name']=f'hero_{sex}_strike2_{direction}'; recovery['recovery']='byte_identical_idle'
                    entries.append(recovery)
    result={'canvas':[160,192],'anchor_px':[80,160],'angle_convention':'clockwise from vertical up; contact follows viewed heading unless source manifest overrides',
            'measurement':'Artist approximate source wrist mapped using imported grid and normalization; nearest native pixel center, not inferred from prompt targets.',
            'entries':entries,'pending':pending}
    (ROOT/'equipment-candidate.json').write_text(json.dumps(result,indent=2)+'\n')
    lines=['// Generated by hero-lineage-20260910/prepare.py --equipment-only.',
           '// Candidate wrist-derived sockets; equipment overlay review remains required.']
    for e in entries:
        boolean=lambda value:'true' if value else 'false'
        lines.append('    {"'+e['name']+'", {160, 192}, {'+f"{e['hand_x']:.1f}, {e['hand_y']:.1f}"+'}, {'+', '.join(map(str,e['fingers']))+'}, '+boolean(e['faces_left'])+', '+boolean(e['behind_actor'])+', false, '+str(e['melee_clockwise_degrees'])+', '+boolean(e['melee_behind_actor'])+'},')
    (REPO/'native/client/lineage_equipment.inc').write_text('\n'.join(lines)+'\n')
    for sex in ['male','female']:
        for direction in DIRECTIONS:
            own=[e for e in entries if e['name'] == f'hero_{sex}_{direction}' or e['name'].startswith(f'hero_{sex}_') and e['name'].endswith('_'+direction)]
            if not own:continue
            sheet=Image.new('RGB',(160*len(own),212),(41,39,37)); draw=ImageDraw.Draw(sheet)
            for i,e in enumerate(own):
                image=Image.open(ROOT/'processed'/sex/direction/(e['name']+'.png')).convert('RGBA')
                sheet.paste(image,(i*160,20),image)
                x,y=i*160+e['hand_x'],20+e['hand_y']
                draw.line((x-3,y,x+3,y),fill=(244,75,75));draw.line((x,y-3,x,y+3),fill=(244,75,75))
                draw.text((i*160+2,2),e['name'].replace('hero_',''),fill=(230,220,204))
            sheet.resize((sheet.width*3,sheet.height*3),Image.Resampling.NEAREST).save(ROOT/'previews'/f'{sex}-{direction}-sockets-3x.png')
    print(f'Mapped {len(entries)} lineage sockets; {len(pending)} manifest/import gaps remain.')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sex', choices=['male', 'female'])
    parser.add_argument('--direction', choices=DIRECTIONS)
    parser.add_argument('--project', type=Path, default=Path(r'Z:\Code\Python\pixel-perfecter'))
    parser.add_argument('--previews-only', action='store_true')
    parser.add_argument('--promote-approved', action='store_true')
    parser.add_argument('--equipment-only', action='store_true')
    args = parser.parse_args()
    if args.equipment_only:
        build_equipment()
        return
    if args.promote_approved:
        promote_approved()
        return
    if args.previews_only:
        build_previews()
        return
    all_records = []
    for sex in ([args.sex] if args.sex else ['male', 'female']):
        for direction in ([args.direction] if args.direction else DIRECTIONS):
            all_records.extend(process(sex, direction, args.project))
    print(f'Staged {len(all_records)} reconstructed poses; runtime unchanged.')
    build_previews()


if __name__ == '__main__':
    main()
