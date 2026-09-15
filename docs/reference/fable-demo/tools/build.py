#!/usr/bin/env python3
"""Build the playable single-file demo: base64-encode ./assets and inject into src/game_template.html."""
import base64, json, pathlib
root = pathlib.Path(__file__).resolve().parent.parent
assets = {p.stem: 'data:image/png;base64,' + base64.b64encode(p.read_bytes()).decode()
          for p in sorted((root/'assets').glob('*.png'))}
tpl = (root/'src'/'game_template.html').read_text()
out = tpl.replace('__ASSETS__', json.dumps(assets))
(root/'dist').mkdir(exist_ok=True)
(root/'dist'/'songs-of-the-mire.html').write_text(out)
print(f"built dist/songs-of-the-mire.html ({len(out)//1024} KB, {len(assets)} assets)")
