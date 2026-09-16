import json
from pathlib import Path
r=Path(__file__).resolve().parents[1]
prompts=json.loads((r/'imagegen-prompts.json').read_text());boss=json.loads((r/'boss-imagegen-prompts.json').read_text())
boss['well-alpha-walk-right-first-softedge']=boss['well-alpha-walk-right']
boss['well-alpha-walk-right']=json.loads((r/'scratch/boss-style-prompt.json').read_text())
prompts.update(boss);(r/'imagegen-prompts.json').write_text(json.dumps(prompts,indent=2))
approved=json.loads((r/'approved-sheets.json').read_text())
approved=list(dict.fromkeys(approved+[f'well-alpha-{a}-{d}' for a in ('idle','walk') for d in ('front','right','back','left')]))
(r/'approved-sheets.json').write_text(json.dumps(approved,indent=2))
