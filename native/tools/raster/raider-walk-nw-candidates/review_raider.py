"""Review assembly and measurements only; Pixel Respecter performs reconstruction."""
from pathlib import Path
import base64
import hashlib
import json
import numpy as np
from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[3]
OUT = HERE / 'selected'
IDLE = REPO / 'native/client/assets/raster/runtime/raider_nw.png'
frames = [Image.open(OUT / f'raider_walk{i}_nw.png').convert('RGBA') for i in range(8)]
idle = Image.open(IDLE).convert('RGBA')
strip = Image.new('RGB', (9 * 84, 114), (65, 48, 58))
draw = ImageDraw.Draw(strip)
for i, im in enumerate([idle] + frames):
    strip.paste(im, (i * 84, 16), im)
    draw.text((i * 84 + 2, 1), 'idle' if i == 0 else str(i - 1), fill='white')
strip.save(OUT / 'native-1x.png')
strip.resize((strip.width * 3, strip.height * 3), Image.Resampling.NEAREST).save(OUT / 'identity-3x.png')
playback = []
for frame in frames:
    panel = Image.new('RGBA', (80, 96), (65, 48, 58, 255))
    panel.alpha_composite(frame)
    playback.append(panel.convert('RGB').resize((240, 288), Image.Resampling.NEAREST))
playback[0].save(OUT / 'walk-3x.gif', save_all=True, append_images=playback[1:], duration=110, loop=0, disposal=2)
arrays = [np.array(im) for im in frames]
metrics = {'canvas': [80, 96], 'pivot': [40, 96], 'body_heights': [], 'frames': [], 'exact_duplicates': []}
for i, (im, arr) in enumerate(zip(frames, arrays)):
    bbox = im.getbbox()
    metrics['body_heights'].append(bbox[3] - bbox[1])
    metrics['frames'].append({'index': i, 'bounds': list(bbox), 'visible_colors': len(np.unique(arr[:, :, :3][arr[:, :, 3] > 0], axis=0)), 'partial_alpha': int(np.count_nonzero((arr[:, :, 3] > 0) & (arr[:, :, 3] < 255))), 'sha256': hashlib.sha256((OUT / f'raider_walk{i}_nw.png').read_bytes()).hexdigest(), 'adjacent_pixel_changes': int(np.count_nonzero(np.any(arr != arrays[(i + 1) % 8], axis=2)))})
    for j in range(i):
        if np.array_equal(arr, arrays[j]):
            metrics['exact_duplicates'].append([j, i])
metrics['cycle_visible_colors'] = len(np.unique(np.concatenate([a[:, :, :3][a[:, :, 3] > 0] for a in arrays]), axis=0))
(OUT / 'measurements.json').write_text(json.dumps(metrics, indent=2) + '\n')
data = ['data:image/png;base64,' + base64.b64encode((OUT / f'raider_walk{i}_nw.png').read_bytes()).decode() for i in range(8)]
idle_data = 'data:image/png;base64,' + base64.b64encode(IDLE.read_bytes()).decode()
html = '''<!doctype html><meta charset="utf-8"><title>NW raider candidate playback</title>
<style>body{background:#352a32;color:#eee;font:16px monospace}section{display:flex;gap:36px;align-items:end}img{image-rendering:pixelated;background:#41303a}button,input{font:inherit}p{max-width:850px}</style>
<h1>NW raider candidate</h1><p>8 chronological frames. 110 ms per frame. Shared 80×96 canvas; pivot 40,96. Candidate review only.</p>
<section><figure><img id="idle" width="80" height="96"><figcaption>Idle 1×</figcaption></figure><figure><img id="native" width="80" height="96"><figcaption>Walk 1×</figcaption></figure><figure><img id="large" width="240" height="288"><figcaption>Walk 3×</figcaption></figure></section>
<p><button id="toggle">Pause</button> Frame <output id="number">0</output> <input id="scrub" type="range" min="0" max="7" value="0"></p>
<p>Known limits: smaller axe than idle, modest crown shift, adjacent contact holds, opaque source background removed by recorded Pixel Respecter cleanup. No game capture acceptance.</p>
<script>const frames=FRAME_DATA;let playing=true,frame=0,last=0;const native=document.getElementById('native'),large=document.getElementById('large'),number=document.getElementById('number'),scrub=document.getElementById('scrub'),toggle=document.getElementById('toggle');document.getElementById('idle').src=IDLE_DATA;function paint(){native.src=large.src=frames[frame];number.textContent=frame;scrub.value=frame;}toggle.onclick=()=>{playing=!playing;toggle.textContent=playing?'Pause':'Play'};scrub.oninput=()=>{playing=false;toggle.textContent='Play';frame=Number(scrub.value);paint()};function tick(now){if(playing&&now-last>=110){frame=(frame+1)%8;last=now;paint()}requestAnimationFrame(tick)}paint();requestAnimationFrame(tick);</script>'''
html = html.replace('FRAME_DATA', json.dumps(data)).replace('IDLE_DATA', json.dumps(idle_data))
(OUT / 'playback.html').write_text(html, encoding='utf-8')
print(json.dumps(metrics, indent=2))
