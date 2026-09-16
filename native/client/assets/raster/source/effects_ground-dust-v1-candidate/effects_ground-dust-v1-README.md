# Ground dust candidate

Four chronological frames: `effect_dust0.png` lift, `effect_dust1.png` spread, `effect_dust2.png` break-up, `effect_dust3.png` sparse dissipation. Every file is 64x48 RGBA, binary alpha, shared 24-color palette, fixed pivot(32,42). Place each canvas by that pivot; do not fit or resize phases independently.

Viewed at native 1x and 3x on the accepted quiet earth. The GIF is an offline timing preview with one blank end hold. Runtime integration and game gates belong to the root task.

This is our dust adaptation of Practical_Low29's published single-subject pixel motion-sheet recipe, not a verified creator dust recipe. Exact prompt, reference hashes, alpha observations and generation provenance: `../../prompts/effects_ground-dust-v1.provenance.json`. Actual Pixel Respecter engine/source/output hashes: `effects_ground-dust-v1-import.provenance.json`.

## Minimal authoring dependencies

- Existing external Pixel Respecter checkout at `Z:/Code/Python/pixel-perfecter`.
- Its existing Python venv executable: `Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe`.
- Repository importer: `native/tools/raster/import_assets.py`. No new packages, servers or credentials.
- Existing NumPy, Pillow and OpenCV versions are recorded in the engine provenance; Python is only for authoring. Runtime needs the four PNGs.

From the isolated repository root:

```powershell
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' native/tools/raster/import_assets.py native/client/assets/raster/prompts/effects_ground-dust-v1-import.json
```

No source background cleanup was applied: reconstruction consumed the generated alpha directly.
