#!/usr/bin/env python3
"""Expand an approved-input content manifest into DRAFT task proposals.
No Git/API actions, claims or production edits. Writes only a new requested file.
"""
from __future__ import annotations
import argparse
import json
import re
from pathlib import Path

STEPS=[
 ('SPEC','Approve one content specification','Purpose, constraints, source/provenance and gameplay role approved.'),
 ('DATA','Implement one definition and behavior fixture','Runtime definition validates and its named positive/negative behavior tests pass.'),
 ('ART','Deliver one visual asset unit','Source/cooked asset, scale, pivots, material/animation contract and provenance accepted.'),
 ('AUDIO','Deliver one sound binding unit','Required sounds or explicit approved N/A, playback binding and provenance accepted.'),
 ('INTEGRATE','Bind the content unit into ordinary native play','Real client/session can encounter, use or traverse this unit without dev grants.'),
 ('REVIEW','Accept the content unit at game scale','Independent counterplay, presentation and performance review on the integrated build passes.')]

def expand(manifest:dict)->dict:
    lot=manifest.get('lot_id','')
    if not re.fullmatch(r'LOT-[A-Z0-9-]+',lot):raise ValueError('lot_id must be LOT-UPPERCASE-ID')
    assets=manifest.get('units')
    if not isinstance(assets,list) or not assets:raise ValueError('units must be a non-empty list')
    tasks=[];seen=set()
    for unit in assets:
        uid=unit.get('id','');name=unit.get('name','')
        if not re.fullmatch(r'[A-Z0-9-]+',uid) or uid in seen:raise ValueError('invalid or duplicate unit ID')
        if not isinstance(name,str) or not name.strip():raise ValueError('unit needs a name')
        seen.add(uid)
        generated={}
        for code,title,acceptance in STEPS:
            ident=f'{lot}-{uid}-{code}'
            dependencies=[] if code=='SPEC' else [generated['SPEC']]
            if code=='INTEGRATE':dependencies=[generated[c] for c in ['DATA','ART','AUDIO']]
            if code=='REVIEW':dependencies=[generated['INTEGRATE']]
            tasks.append(dict(id=ident,lot_id=lot,unit_id=uid,state='DRAFT',title=f'{title}: {name}',depends_on=dependencies,
                acceptance=acceptance,base_sha=None,owned_paths=[],acceptance_commands=[],
                note='Planning child only. Split further when this unit contains multiple independent assets or behavior changes. N/A requires explicit review; never fake completion.'))
            generated[code]=ident
    return dict(schema_version=1,lot_id=lot,status='DRAFT_PROJECTION_NOT_DISPATCH_AUTHORITY',
                required_backbone_gates=manifest.get('required_backbone_gates',[]),tasks=tasks)

def main()->int:
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('manifest',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
    try:
        data=expand(json.loads(a.manifest.read_text(encoding='utf-8')))
        with a.output.open('x',encoding='utf-8') as f:f.write(json.dumps(data,indent=2)+'\n')
        print(f'Wrote {len(data["tasks"])} DRAFT child proposals to {a.output}');return 0
    except (OSError,ValueError,TypeError) as e:p.error(str(e))
    return 2
if __name__=='__main__':raise SystemExit(main())
