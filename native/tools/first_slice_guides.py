"""Exact 4x guides: eight whole 96px frames in 96x128 slots. No cropping."""
import hashlib
import json
from pathlib import Path
import argparse
from PIL import Image

ROOT = Path(__file__).resolve().parents[1] / 'client/assets/first-slice'

def build(reference_root):
    refroot = Path(reference_root)
    manifest = json.loads((refroot / 'manifest-linen.json').read_text())
    out = ROOT / 'guides'
    out.mkdir(parents=True, exist_ok=True)
    for clip, record in manifest['clips'].items():
        sheet = Image.new('RGBA', (384, 256))
        for i, frame in enumerate(record['frames']):
            p = refroot / frame['src']
            assert hashlib.sha256(p.read_bytes()).hexdigest() == frame['sha256']
            source = Image.open(p).convert('RGBA')
            assert source.size == (96, 96)
            sheet.paste(source, ((i % 4) * 96, (i // 4) * 128 + 16))
        sheet.resize((1536, 1024), Image.Resampling.NEAREST).save(out / (clip + '.png'))
    (out / 'provenance.json').write_text(json.dumps({
        'source_manifest_sha256': hashlib.sha256((refroot / 'manifest-linen.json').read_bytes()).hexdigest(),
        'frame': [96, 96], 'slot': [96, 128], 'inset': [0, 16], 'scale': 4,
        'anchor': [48, 80], 'clips': manifest['clips']}, indent=2))
    print('16 whole-frame guides, 4x nearest-neighbor; native geometry unchanged')

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('reference_root')
    build(p.parse_args().reference_root)
