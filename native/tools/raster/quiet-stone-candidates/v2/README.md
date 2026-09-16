# Quiet stone v2: half-offset seam edit

The second and final built-in call used the first native tile as a material
reference. An exact32px wrap in both axes exposed its original boundaries at
the center. The image tool was asked to repair that crossing and preserve the
rest. Actual Pixel Respecter reconstruction then produced64×64; an exact
negative32px wrap restored the original phase. No pixels were procedurally
painted, blended, edge-averaged or feathered.

The result remains a held candidate. Broad quiet stone masses survive, but the
viewed3×3 repetition retains obvious square structure and uneven joints.
The edit also changed some texture outside the requested narrow crossing.
This is not a successful seamless repair and has not been promoted.

The [existing implementation](https://github.com/ianlintner/ai-pixel-art-image-generation/blob/main/scripts/lib/seamless.py)
uses half-image offset, central repair and reverse offset as part of a tiling
pipeline. Its actual central repair uses blending. We substituted a reference-led
image edit for that step; the result is our adaptation of an older-model asset
pipeline, not published Image2.5 success. The original source was read before
this call. Higgsfield’s already-read geometry/material reference separation
provided the explicit input roles, not evidence that the repair would work.

## Review

| Property | v1 | v2 |
|---|---:|---:|
| Native size |64×64|64×64|
| Visible colors |23|22, from v1 palette|
| Alpha |255 throughout|255 throughout|
| Left/right edge RGB MAE |12.25|27.16|
| Top/bottom edge RGB MAE |17.26|21.48|
| Mean adjacent luminance change |5.18|5.55|

Edge differences alone do not prove a broken join: adjacent pixels across a
real stone joint may differ. Here the independent visual repetition review also
finds the repeated boundary structure obvious. No metric pass threshold is used
as a substitute for production review.

`repeat-3x3-3x.png`, `actors-comparison-1x.png` and `actors-comparison-2x.png`
contain the reviewed mechanical composites. `source-restored-phase.png` is an
exact wrapped source diagnostic. The unmodified generated source remains in
`source/terrain-quiet-stone-v2.png`, in the offset layout returned by the tool.

The outside-band comparison combines generation, reconstruction and sampling:
412/3,136 outside pixels retain exact RGB, with MAE4.15. It is not a source-only
model-change measurement, and this result cannot be described as preserving
every outside pixel. The palette reference preserves colors, not geometry.

## Reproduce

Run from `C:/Users/Alex/Documents/ChatGPT/verdigris-diablo-study`:

```powershell
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' 'native/tools/raster/import_assets.py' 'native/tools/raster/quiet-stone-candidates/v2/import.json'
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' 'native/tools/raster/quiet-stone-candidates/restore_phase.py'
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' 'native/tools/raster/quiet-stone-candidates/review.py' v2
```

The importer’s direct output is `terrain_quiet_stone_offset.png`; the second
command produces the final-phase `terrain_quiet_stone.png`. Exact source,
reference, importer, external engine, palette and both output hashes are recorded
in the provenance/dependency JSONs. No runtime/catalog/code/build/staging changes
were made. Both allowed image calls are consumed; further generation is held.
