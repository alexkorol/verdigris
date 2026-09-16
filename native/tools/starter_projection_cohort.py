"""Freeze completed noncombat/action clips into an explicitly versioned cohort.

Copies bytes without transforming pixels. Never copies old attacks or overwrites
an existing destination. Source report hashes and source cohort remain recorded.
"""
from pathlib import Path
import sys, shutil, json, hashlib
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-projection'
sex,old,new=sys.argv[1:4]
assert sex in ['male','female']
for name in [old,new]:
 assert '/' not in name and '\\' not in name and name not in ['.','..']
source=ROOT/old;dest=ROOT/new
for d in ['frames','reports','blender']:(dest/d).mkdir(parents=True,exist_ok=True)
copied=[]
for path in sorted((source/'reports').glob(f'{sex}-*-projection.json')):
 report=json.loads(path.read_text());action=report['gait']
 if 'attack' in action:continue
 target=dest/'reports'/path.name
 if target.exists():continue
 count=len(report.get('source_frames',range(4 if 'idle' in action else 8)))
 frames=[source/'frames'/f'{sex}-{action}-{direction}-{i:02d}.png' for direction in ['front','right','back','left'] for i in range(count)]
 assert all(p.exists() for p in frames), f'Incomplete source {action}'
 for p in frames:
  q=dest/'frames'/p.name
  assert not q.exists()
  shutil.copyfile(p,q)
  assert hashlib.sha256(p.read_bytes()).digest()==hashlib.sha256(q.read_bytes()).digest()
 blend=source/'blender'/f'{sex}-{action}-projection.blend'
 if blend.exists():shutil.copyfile(blend,dest/'blender'/blend.name)
 report['copied_from_cohort']=old;report['cohort']=new
 report['source_report_sha256']=hashlib.sha256(path.read_bytes()).hexdigest()
 report['render_complete']=True
 target.write_text(json.dumps(report,indent=2),encoding='utf-8');copied.append(action)
print(json.dumps({'source':old,'destination':new,'copied_actions':copied}))
