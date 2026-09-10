# Raster asset import

Rebuild the current runtime library and verify all active asset hashes:

```powershell
./native/tools/raster/import_all.ps1 -Previews
```

This imports the current sprite library, all native RGBA with binary alpha and at most
32 visible colors, then writes `native/client/assets/raster/runtime/catalog.json`.
The catalog resolves import order explicitly: `hero-single.json` replaces the
SE idle frame from the older actor sheet, and `hero-directions.json` replaces
SW/NE/NW with isolated single-subject sources. Each source and actual engine file is
hashed. The contact sheets show 3x nearest-neighbor pixels; the runtime files
remain native resolution.

This is a conversion milestone; live gameplay and motion require separate
inspection. The older actor sheet's enclosed checker pockets were corrected
through 18 individually inspected local source windows, removing 2,420 background
pixels with Pixel Respecter's connected-fill API. Bone-mask highlights and
matching colors elsewhere remain intact. The new single-subject SE idle has
genuine source alpha. Rejected duplicate NW sheet poses are replaced by isolated
`wight_nw` and `artisan_nw` sources. Remaining review notes live in the catalog.

Run the owner's actual **Pixel Respecter** project at
`Z:/Code/Python/pixel-perfecter`. The similarly named
`C:/Users/Alex/Documents/ChatGPT/Pixel Perfecter` directory is an empty Git
repository and contains no implementation. The working project has uncommitted
release code; every import records hashes of its Python sources, not just HEAD.

The authoritative alpha-preserving API is
`pixel_perfecter.workspace.reconstruct(source, Options(...))`. The older batch
CLI (`python -m pixel_perfecter.cli`) invokes the core directly and uses binary
majority alpha. This importer uses the desktop workspace API to retain cell
coverage and alpha-weighted RGB at translucent edges, avoiding dark halos.
For a fixed grid, the core selects a modal color per cell; it can internally
quantize label colors for complex inputs. No additional palette is imposed by
the importer. `cell_size: 0` invokes the project's grid detector; an explicit
cell size invokes its deterministic rigid reconstruction on each sheet cell.

Use the existing environment (Python 3.12, NumPy, OpenCV, Pillow):

```powershell
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' `
  native/tools/raster/import_assets.py native/tools/raster/actors.json `
  --contact-sheet native/tools/raster/actors-preview.png
```

`--project` or `PIXEL_RESPECTER_ROOT` selects another checkout. The project
requires Python >=3.10, numpy >=1.23,<3, opencv-python >=4.7,<5 and
Pillow >=9.3,<13. No ML package, Qt, network access, or API key is needed.

Manifest version 1:

```json
{
  "version": 1,
  "output_dir": "../../client/assets/raster/runtime",
  "defaults": {
    "cell_size": 4,
    "alpha_mode": "preserve",
    "require_transparency": true,
    "canvas": [96, 128],
    "trim": true,
    "align": "bottom_center"
  },
  "sheets": [{
    "source": "../../client/assets/raster/source/actors.png",
    "grid": [4, 4],
    "assets": [{"name": "hero_se", "cell": [0, 0]}]
  }]
}
```

Paths resolve relative to the manifest. `grid` is columns/rows; `cell` is
zero-based column/row. Sheets may set `margin` and `gutter` as x/y integers.
An asset can override extraction with an exact `source_box` of
`[left, top, right, bottom]`. Per-asset values override sheet `defaults`,
which override manifest `defaults`.

Current actors share an 80x96 canvas. Small props use 64x96 or 96x128;
the approved larger static props use 192x256. Terrain
can set `canvas: [64,64]`, `trim: false`, `fill: true` and
`require_transparency: false`. Terrain `fill` explicitly resamples to the
whole target tile. Sprites shrink only as needed to fit the canvas; set
`allow_upscale: true` only when intentional. Optional `fit` bounds visible art
inside a larger canvas. All resampling is nearest-neighbor, and all placements
use integer pixels. Runtime PNGs are native-resolution RGBA, not enlarged previews.

The default sprite pivot is the bottom center of the entire image and there
is no bottom padding. Optional integer `padding` is recorded as
`baseline_offset_px`. The JSON provenance includes source/cropped/reconstructed
dimensions, trim bounds, placed content bounds, pivot, engine hashes, dependency
versions, output hashes, alpha range, and detector warnings. `alpha_floor: 1`
keeps all nonzero alpha; increasing it explicitly removes faint edge pixels.
`alpha_mode: crisp` instead requests Pixel Respecter's binary alpha mode.
Transparent asset imports reject fully opaque source reconstructions, so an
accidental flattened background cannot silently enter the sprite library.

Import validation reconstructs all assets before writing outputs. Files from
other manifests are left intact. The optional contact sheet shows exactly 1:1
runtime pixels against a dark checkerboard and should be visually inspected
before integrating the assets into gameplay.

The current world-art direction uses `alpha_mode: crisp` and `max_colors: 32`
to retain deliberate, opaque pixel clusters and a limited palette. Alpha is
binary after the project's cell-majority reconstruction (source alpha >=128
counts as opaque); smooth translucent source edges do not enter the world
sprites. `max_colors` uses Pixel Respecter's own undithered color reduction.
This is explicit stylization requested for the game, not a claim that arbitrary
coarse reconstruction automatically converts a detailed painting to authored
pixel art. Start from deliberate pixel-art sheets and inspect the result.

Actor animation sets use `preserve_scale: true` and a common `cell_size`.
No frame is individually enlarged or shrunk. If a frame exceeds the canvas,
the importer fails instead of changing the actor's size. `anchor_source: [x,y]`
specifies a shared foot pivot in source-cell coordinates, mapped through the
rigid reconstruction. It retains authored horizontal/vertical offsets through
trimming. The alternative `anchor_reconstructed` specifies native-pixel values.
`anchor_x_source` keeps the source body center while putting the lowest visible
pixel on the output baseline when generated rows do not share a reliable y pivot.

When a source contains a baked light checkerboard, an explicit sheet
`border_background: {"color": [231,231,233], "tolerance": 68}` invokes the
project's own `colors.make_border_connected_transparent` before reconstruction.
It preserves disconnected interior colors. Explicit `interior_windows` can
name individually inspected `[left,top,right,bottom]` regions where the same
connected-fill API removes an enclosed checker pocket. This does not apply a
color mask globally. Every window and removed-pixel count is recorded in the
source provenance; genuine-alpha sheets omit the operation.

The SE walk importer uses four isolated single-pose images at a common source
grid pitch of 14 and source pivot `[572,1344]`, on an 80x96 runtime canvas. It
preserves authored head/hip alignment and lifted-foot offsets. It never fits
or bottom-aligns individual walking-frame bounds. Once all four frames are
present, `python native/tools/raster/preview_walk.py` produces native and 4x
strips plus a fixed-origin looping GIF for visual review.

`large-props.json` supersedes tree/hut/storehut/column/gate with fresh source
reconstructions at source grid sizes 3/2/2/3/8. No resizing follows reconstruction.
Original `props.json` and `gate.json` settings and provenance remain available.
The finer choices were inspected beside the hero at intended display heights;
these sources are not exact nearest-neighbor grids, so the import does not claim
to recover a mathematically exact original grid. `experiment_large_props.py`
reproduces the comparison candidates without replacing runtime files.

`environment-singles.json` adds quieter 64x64 packed earth and 128x128 exit
stairs. `preview_environment.py` writes native and integer previews, the
four-direction hero strip, and a 3x3 repeated earth preview with opposing-edge
statistics against ordinary native neighbors. The preview does not apply hidden
blending or seam corrections. Source prompts and rejected generated candidates
remain under the asset sources owned by their generation tasks.

Focused checks (including the actual external reconstruction engine):

```powershell
Push-Location native/tools/raster
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' -m unittest -v test_import_assets
Pop-Location
```

Equipment attachment checks use the production Windows renderer and current
runtime PNGs. From any working directory, invoke the script by its path:

```powershell
./native/tools/raster/test_equipment.ps1
```

The runner discovers Visual Studio C++ Build Tools, compiles both checks with
assertions enabled, and fails on a compile or check error. `-VcVars` accepts an
explicit `vcvars64.bat` path. `test_equipment.cpp` checks source grips, mirrored
placement, actor/finger occlusion, cache reuse, GDI handles, and clip restoration;
it also produces native and 3x contact sheets. `test_equipment_sampling.cpp`
compares the helper's finger clip with actual GDI+ source-coordinate sampling
at eight integer/fractional sizes using synthetic fixtures. All generated
files stay under `.ci-artifacts/raster-equipment`; runtime assets are read-only.
These checks preserve attachment evidence; live gameplay and visual acceptance
remain separate. Legacy attack poses are explicitly marked transitional in
`native/client/raster_equipment.hpp`.
