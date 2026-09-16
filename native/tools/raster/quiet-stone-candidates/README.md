# Quiet limestone floor candidate

One built-in image call produced `terrain-quiet-stone-v1.png`. The root paused
further generation while reviewing firsthand workflow research. No second call,
production import, runtime replacement, catalog edit, build or staging occurred.
The quiet earth texture remains unchanged.

Root subsequently viewed `v1/actors-comparison-1x.png` and
`v1/repeat-3x3-3x.png`: noise is reduced, but square repetition and inconsistent
seam joints remain obvious. Root explicitly held this candidate without
production acceptance and instructed no second image call. The terrain prompt
remains our unverified adaptation of the published reference-role workflows.

The candidate has broad limestone slabs, a subdued warm-gray palette and fewer
small outlines than the existing stone texture. Native actor composites show
clearer silhouettes, but the field is brighter than both accepted ground
references. The slab arrangement remains visibly periodic. It is **not
seamless** and is not accepted as a finished repeating crypt floor.

## Review files

- `v1/native-1x.png` and `native-3x.png`: candidate at native and3× scale.
- `v1/repeat-3x3-1x.png` and `repeat-3x3-3x.png`: exact repetition, no blending.
- `v1/actors-comparison-1x.png` and `actors-comparison-2x.png`: unchanged native
  hero/wight over existing stone, quiet earth and candidate;2× is nearest-neighbor.
- `v1/metrics.json`: alpha, colors, neighboring-pixel contrast and edge metrics.

These are mechanical asset compositions, not production-game captures. All
source texels are shown1:1: the old32×32 stone repeats6×6 across192px, while
the two64×64 textures repeat3×3. This compares native detail density, not the
renderer’s world-coordinate sampling. Root owns the actual camera review.

## Reconstruction

Run from `C:/Users/Alex/Documents/ChatGPT/verdigris-diablo-study`:

```powershell
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' 'native/tools/raster/import_assets.py' 'native/tools/raster/quiet-stone-candidates/v1/import.json'
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' 'native/tools/raster/quiet-stone-candidates/review.py' v1
```

The real external Pixel Respecter auto-grid detector finds a mesh with nominal
cell11. Its114×121 reconstruction is normalized to an opaque64×64 terrain
canvas with nearest-neighbor sampling, following the existing quiet-earth
import method. The result has23 colors under the24-color limit. Source pixels
were not painted, offset, mirrored, blended or otherwise repaired. The external
engine is unchanged; exact source and engine hashes are in the import report.

Opposite-edge RGB mean absolute differences are12.25 left/right and17.26
top/bottom on a0–255 scale. Only8/64 and0/64 edge pixels respectively match
exactly. Adjacent luminance differences average5.18 versus21.90 for existing
stone and1.62 for quiet earth. These are descriptive measurements, not visual
acceptance scores. The viewed3×3 grid confirms obvious repeated slab shapes.

## Reference workflow and limits

The original expanded P12 prompt on
[12ui’s comparison](https://12ui.com/gpt-image-2.5-vs-2) assigns distinct jobs
to its references. Our adaptation assigns quiet earth to detail density/value
masses and the gate’s limestone uprights to material/color only. It does not
copy gate geometry or wood. The linked example is UI work, not terrain.

The [game-room developer’s firsthand feedback](https://www.reddit.com/r/codex/comments/1waxfbk/comment/p8m88mg/)
reports persistent microtexture on walls and floors. That supplied a concrete
review concern, not a corrective prompt. No source reviewed here supplies a
verified seamless terrain recipe. Exact submitted prompt, reference roles,
hashes and local observations live in the source’s prompt provenance JSON.

The noisy floor and `remote-wight-arrived.png` were viewed as failure/context
evidence only and were not given to the generator. No image model variant was
selected or claimed; the built-in tool exposes no such selector.
