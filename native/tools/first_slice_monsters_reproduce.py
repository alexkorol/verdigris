"""Reconstruct selected packaged originals and compare every reviewed pixel hash."""
import argparse,json,shutil,tempfile,hashlib
from pathlib import Path
import first_slice_monsters_transfer_v4 as transfer
ROOT=Path(__file__).resolve().parents[1]/'client/assets/first-slice/monsters-v2'

def reproduce(accepted,keys):
    accepted=Path(accepted);manifest=json.loads((accepted/'manifest.json').read_text())
    expected={Path(f['file']).name:f['sha256'] for c in manifest['clips'].values() for f in c['frames']}
    with tempfile.TemporaryDirectory(prefix='verdigris-monsters-reproduce-') as temp:
        stage=Path(temp);transfer.ROOT=stage
        shutil.copy2(accepted/'transfer-manifest-v4.json',stage/'transfer-manifest-v4.json')
        shutil.copytree(accepted/'references',stage/'references')
        count=0
        for key in keys:
            transfer.recover(accepted/'originals'/f'{key}.png',key)
            report=json.loads((stage/'review'/f'{key}.json').read_text())
            for frame in report['frames']:
                assert frame['sha256']==expected[frame['file']],f'Reconstruction mismatch: {frame["file"]}'
                count+=1
    print('Byte-identical reviewed frames:',count)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--accepted',type=Path,default=ROOT/'accepted');p.add_argument('--keys',type=Path,required=True)
    a=p.parse_args();reproduce(a.accepted,json.loads(a.keys.read_text()))
