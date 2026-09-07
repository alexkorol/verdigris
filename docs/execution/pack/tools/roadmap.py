#!/usr/bin/env python3
"""Read-only Verdigris planning validator and conservative wave suggester.

No network, Git changes, file writes, task claims, or agent launches. Planning
mode is a what-if projection and never turns DRAFT into READY.
"""
from __future__ import annotations
import argparse
import fnmatch
import json
import re
import sys
from pathlib import Path
from typing import Any

STATES = {'DRAFT','AUTO_RELEASE','READY','CLAIMED','IMPLEMENTED','REVIEW_REQUESTED',
          'ACCEPTED','INTEGRATED','REVISE','BLOCKED','SUPERSEDED'}
ROOT = Path(__file__).resolve().parents[1]


def normalized(path: str) -> str:
    p = path.replace('\\','/').strip().rstrip('/')
    if not p or p.startswith('/') or ':' in p or '..' in p.split('/'):
        raise ValueError(f'unsafe/non-relative path: {path!r}')
    return p.casefold()  # conservative across case-insensitive target systems


def overlaps(a: str, b: str) -> bool:
    """Conservative overlap, including directory prefixes and uncertain globs."""
    a, b = normalized(a), normalized(b)
    if a == b or a.startswith(b + '/') or b.startswith(a + '/'):
        return True
    if fnmatch.fnmatchcase(a,b) or fnmatch.fnmatchcase(b,a):
        return True
    # For glob patterns, use the fixed prefix; ambiguity is treated as conflict.
    def fixed(s: str) -> str:
        return re.split(r'[?*\[]',s,maxsplit=1)[0].rstrip('/')
    pa,pb = fixed(a),fixed(b)
    if a != pa or b != pb:
        return not pa or not pb or pa.startswith(pb) or pb.startswith(pa)
    return False


def validate(data: dict[str,Any]) -> list[str]:
    errors: list[str] = []
    if data.get('schema_version') != 1:
        errors.append('unsupported registry schema_version')
    tasks=data.get('tasks')
    if not isinstance(tasks,list):
        return errors+['tasks must be a list']
    indexed: dict[str,dict[str,Any]] = {}
    for task in tasks:
        if not isinstance(task,dict):
            errors.append('task is not an object'); continue
        ident=task.get('id','')
        if not isinstance(ident,str) or not re.fullmatch(r'VG-[A-Z]+-\d{3}',ident):
            errors.append(f'invalid ID: {ident!r}')
        if ident in indexed: errors.append(f'duplicate ID: {ident}')
        indexed[ident]=task
        for field in ('title','outcome','negative_control','role','milestone'):
            if not isinstance(task.get(field),str) or not task[field].strip():
                errors.append(f'{ident}: missing {field}')
        if task.get('state') not in STATES: errors.append(f'{ident}: invalid state')
        if not isinstance(task.get('acceptance'),list) or not task['acceptance']:
            errors.append(f'{ident}: missing acceptance assertions')
        if not isinstance(task.get('depends_on'),list):
            errors.append(f'{ident}: depends_on must be list'); task_deps=[]
        else: task_deps=task['depends_on']
        if len(set(task_deps)) != len(task_deps): errors.append(f'{ident}: duplicate dependencies')
        for field in ('owned_paths','proposed_owned_paths','forbidden_paths'):
            for p in task.get(field,[]):
                try: normalized(p)
                except (ValueError,AttributeError) as exc: errors.append(f'{ident}: {exc}')
        if task.get('state') in {'READY','CLAIMED','IMPLEMENTED','REVIEW_REQUESTED','ACCEPTED','INTEGRATED'}:
            if not re.fullmatch(r'[a-f0-9]{40}',str(task.get('base_sha',''))):
                errors.append(f'{ident}: executable state needs exact base_sha')
            if not task.get('owned_paths'): errors.append(f'{ident}: executable state needs stamped owned_paths')
            if not task.get('acceptance_commands'): errors.append(f'{ident}: executable state needs acceptance_commands')
            if not task.get('owner_ruling'): errors.append(f'{ident}: executable state needs authority record')
            if task.get('state') not in {'READY','INTEGRATED'} and not task.get('claim'):
                errors.append(f'{ident}: claimed/review state needs claim record')
    for ident, task in indexed.items():
        for dep in task.get('depends_on',[]):
            if dep not in indexed: errors.append(f'{ident}: missing dependency {dep}')
            if dep == ident: errors.append(f'{ident}: self-dependency')
    visiting:set[str]=set();visited:set[str]=set()
    def visit(ident: str) -> None:
        if ident in visiting:
            errors.append(f'dependency cycle at {ident}');return
        if ident in visited:return
        visiting.add(ident)
        for dep in indexed[ident].get('depends_on',[]):
            if dep in indexed:visit(dep)
        visiting.remove(ident);visited.add(ident)
    for ident in indexed: visit(ident)
    return errors


def task_conflicts(a: dict[str,Any], b: dict[str,Any], planning: bool=False) -> bool:
    field='proposed_owned_paths' if planning else 'owned_paths'
    if any(overlaps(x,y) for x in a.get(field,[]) for y in b.get(field,[])):
        return True
    resource='proposed_exclusive_resources' if planning else 'exclusive_resources'
    return bool(set(a.get(resource,[])) & set(b.get(resource,[])))


def suggest_wave(data: dict[str,Any], limit: int=8, planning: bool=False,
                 assumed: set[str] | None=None) -> dict[str,Any]:
    tasks=data['tasks'];indexed={t['id']:t for t in tasks}
    assumed=assumed or set()
    unknown=assumed-set(indexed)
    if unknown: raise ValueError('unknown assumed IDs: '+', '.join(sorted(unknown)))
    if assumed and not planning: raise ValueError('assumptions require --planning')
    integrated={t['id'] for t in tasks if t['state']=='INTEGRATED'} | assumed
    # Verify that a what-if completed set is closed over dependencies.
    for ident in assumed:
        missing=set(indexed[ident]['depends_on'])-integrated
        if missing: raise ValueError(f'assumed completion {ident} lacks '+', '.join(sorted(missing)))
    eligible=[t for t in tasks if t['id'] not in integrated and
        t['state'] in ({'DRAFT','AUTO_RELEASE','READY'} if planning else {'READY'}) and
        set(t['depends_on']) <= integrated]
    eligible.sort(key=lambda t:(t['priority'],t['milestone'],t['id']))
    active=[t for t in tasks if t['state'] in {'CLAIMED','IMPLEMENTED','REVIEW_REQUESTED','ACCEPTED'}]
    selected=[];excluded=[]
    for t in eligible:
        blockers=[x['id'] for x in active+selected if task_conflicts(t,x,planning)]
        if blockers: excluded.append({'id':t['id'],'conflicts_with':blockers})
        elif len(selected)<limit:selected.append(t)
    return dict(mode='WHAT_IF_DRAFT_CANDIDATES_NOT_CLAIMABLE' if planning else 'READY_CANDIDATES_REVALIDATE_AND_CLAIM',
        assumptions=sorted(assumed),candidate_count=len(eligible),selected=[{'id':t['id'],'title':t['title']} for t in selected],
        excluded_conflicts=excluded,
        warning='Read-only suggestion. It does not verify live Git head, rulings, external resources, semantic compatibility or exclusive claims. Integration hooks require separate reservations.')


def main() -> int:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--registry',type=Path,default=ROOT/'backlog/tasks.json')
    subs=parser.add_subparsers(dest='command',required=True)
    subs.add_parser('validate')
    show=subs.add_parser('show');show.add_argument('id')
    wave=subs.add_parser('wave');wave.add_argument('--limit',type=int,default=8)
    wave.add_argument('--planning',action='store_true')
    wave.add_argument('--assume-integrated',default='',help='Comma-separated IDs; planning mode only')
    args=parser.parse_args()
    try:
        data=json.loads(args.registry.read_text(encoding='utf-8'))
        errors=validate(data)
        if errors:
            print(json.dumps({'valid':False,'errors':errors},indent=2));return 1
        if args.command=='validate':
            print(json.dumps({'valid':True,'goals':len(data['tasks']),
               'edges':sum(len(t['depends_on']) for t in data['tasks']),
               'draft':sum(t['state']=='DRAFT' for t in data['tasks']),
               'warning':'Structural plan validation only; no game implementation or test completion implied.'},indent=2))
        elif args.command=='show':
            found=next((t for t in data['tasks'] if t['id']==args.id),None)
            if found is None:raise ValueError('unknown task ID: '+args.id)
            print(json.dumps(found,indent=2))
        else:
            if not 1 <= args.limit <= 8:raise ValueError('limit must be 1..8 under this proposed initial policy')
            assumed={x.strip() for x in args.assume_integrated.split(',') if x.strip()}
            print(json.dumps(suggest_wave(data,args.limit,args.planning,assumed),indent=2))
        return 0
    except (OSError,ValueError,KeyError,TypeError) as exc:
        print(f'error: {exc}',file=sys.stderr);return 2

if __name__=='__main__':raise SystemExit(main())
