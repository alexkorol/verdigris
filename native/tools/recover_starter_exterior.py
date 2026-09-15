"""Reconstruct exterior candidates with unchanged Pixel Respecter acceptance gates."""
import argparse,json,sys,shutil
from pathlib import Path
sys.path.insert(0,str(Path(__file__).parent))
import first_slice_art
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/starter-v2'
p=argparse.ArgumentParser();p.add_argument('source');p.add_argument('clip');p.add_argument('--refs',default=str(ROOT));p.add_argument('--registration',choices=['sheet','row'],default='sheet');a=p.parse_args()
refroot=Path(a.refs)
manifest={'clips':{}}
for f in refroot.glob('*-references.json'):manifest['clips'].update(json.loads(f.read_text())['clips'])
(refroot/'manifest-exterior.json').write_text(json.dumps(manifest,indent=2))
first_slice_art.ROOT=ROOT
first_slice_art.recover(Path(a.source),a.clip,refroot,Path('Z:/Code/Python/pixel-perfecter'),'manifest-exterior.json',registration=a.registration)
