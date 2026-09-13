# NE reference palette candidate

The importer now accepts a manifest-level option:

```json
"palette_reference": {"source": "relative/path/to/accepted.png", "max_colors": 32}
```

The path resolves relative to the manifest. Only RGB colors with alpha greater
than zero enter the palette. The reference must be a readable, single-frame
PNG with at least one visible color and no more than the declared limit
(default 32; supported range 1–256). The palette is never reduced. Conflicting
`shared_palette_max_colors`, named `palette`, or per-asset `max_colors` options
are rejected instead of silently applying multiple color transforms.

The actual external `pixel_perfecter.palettes.snap_to_palette` API performs
CIELAB matching after reconstruction/normalization. Alpha and canvas geometry
are verified unchanged, then invisible RGB is normalized to zero. Provenance
records the reference path, raw PNG SHA-256, palette RGB entries, API and
matching policy; the existing engine record fingerprints `palettes.py`.
Manifests without this option retain the previous behavior.

This candidate uses six existing 80×96 native images, in this exact order:
accepted `hero_ne`, current windup, current commit, contact-v6 at cell 11,
followthrough-v3, current recovery. They pass through actual reconstruction at
cell 1 with preserved alpha, untrimmed canvas and source anchor `[40,96]`.
No per-pose fitting or rescaling occurs. Full source paths and hashes are in
the manifest and candidate provenance.

`before-after-sequence-1x.png` and `before-after-sequence-3x.png` were inspected.
Top row is original, bottom row snapped, with the same fixed pixel origin.
The colors are more coherent against the accepted idle, especially the orange
tunic and olive trousers. Followthrough remains visibly broader/darker and
the source head/body differences remain; palette snapping cannot infer
material identity or correct poses. The GIF is a slow paired review (before
left, after right), not a production timing demonstration.

`palette-review.json` verifies six identical alpha arrays and canvases,
unchanged ready0 RGBA, and zero output colors outside the reference's 32
colors. Output color counts are 32, 20, 20, 22, 24, 21. The full importer suite
passed 10 tests, including three new focused tests for actual reference
mapping/geometry, invalid references, and conflicting color operations.

Candidate only: active runtime PNGs, catalog and freeze are untouched. Root
owns visual acceptance and any later promotion; equipment fingerprints must
use these final RGB values even though hand coordinates did not change.
