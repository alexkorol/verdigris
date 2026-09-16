import hashlib,json,shutil,subprocess,sys
from pathlib import Path
r=Path(__file__).resolve().parents[1];accepted=r/'accepted';out=r/'scratch/reproduction-check';out.mkdir(exist_ok=True)
manifest=json.loads((accepted/'manifest.json').read_text());key='well-alpha-walk-right'
shutil.copy2(accepted/'transfer-manifest-v4.json',out/'transfer-manifest-v4.json')
(out/'references').mkdir(exist_ok=True)
for f in manifest['clips'][key]['frames']:shutil.copy2(accepted/f['reference'],out/f['reference'])
tool=r.parents[3]/'tools/first_slice_monsters_transfer_v4.py'
subprocess.run([sys.executable,str(tool),'--asset-root',str(out),'--raw',str(accepted/'originals'/f'{key}.png'),'--clip',key],check=True)
for f in manifest['clips'][key]['frames']:
    generated=out/'candidates'/Path(f['file']).name
    assert hashlib.sha256(generated.read_bytes()).hexdigest()==f['sha256']
print('All8 reconstructed frames byte-identical from packaged inputs')
