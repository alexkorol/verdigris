# Cutaway limestone wall candidate

Root reviewed the native module, repeated wall, and actor context on September 10, 2026 and approved an integrated production trial. Recurring cap motifs and imperfect mortar joins remain explicit limits. Live-game acceptance and production gates belong to root.

The frozen candidate is `selected/wall_stone_cutaway.png`: 64×96 RGBA, 24 colors, every alpha value255, pivot[32,96]. Its cap is[0,0,64,64], south face[0,64,64,96], and actual ground footprint[0,32,64,96]. Rows advance64 pixels during depth composition; a southern cap covers the preceding internal face. No projection or collision was changed.

Two built-in calls were used. The first was too speckled and had an extra rim. The second reduced the grain but still returned an RGB painted background, stretched the cap, and retained the rim. Both exact prompts, reference roles/hashes and unchanged source PNGs are retained in `native/client/assets/raster/prompts/wall-stone-cutaway-provenance.json`.

Higgsfield's published spring-turntable geometry-guide instruction was adapted to module shape, with the accepted column supplying only stone material/pixel style. This is our masonry adaptation, not a published or verified Image2.5 wall-tiling recipe. The prompt explicitly replaced guide colors, shading and background.

## Reproduce the candidate

From the repository root, with the owner's unchanged external engine installed:

```powershell
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' native/tools/raster/import_assets.py native/tools/raster/wall-stone-candidates/wall-stone-cutaway-v2-planes.json
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' native/tools/raster/wall-stone-candidates/assemble_review.py
```

The existing importer calls `pixel_perfecter.workspace.reconstruct` at measured source pitch10. Cap crop[194,261,828,959] reconstructs63×69 then nearest-normalizes to64×64. Face crop[194,997,828,1277] reconstructs63×28 then nearest-normalizes to64×32. The extra source rim between those crops is omitted. The source is unchanged; there is no foreground repaint or background color-key operation. The cropped module is intentionally fully opaque for shared-edge coverage.

The existing importer calls the owner's shared palette reducer once across both planes. `assemble_review.py` pastes those unaltered imported planes at[0,0] and[0,64]. Its other Pillow work creates review compositions only. `wall-stone-cutaway-candidate.json` records this two-stage assembly; it is not itself a directly runnable importer manifest.

The full engine source/dependency hashes and dirty-worktree state are in `v2-planes/wall-stone-cutaway-v2-planes.provenance.json`. `dependencies.json` records retained workspace paths/hashes and read-only reference dependencies. Root owns any derived-source copy, cell1 production manifest, runtime/catalog registration and gates.

## Viewed evidence and limits

- `selected/wall_stone_cutaway.png`: native1x geometry and material.
- `selected/native-3x.png`: nearest3x module.
- `selected/wall-3x3-1x.png` and `wall-3x3-3x.png`: correct64px row spacing/depth cover.
- `selected/actor-context-1x.png`: actor canvas height1.75×64=112px, matching main's relative world scale. The north actor keeps its ground position; a second panel approximates the renderer's fixed alpha80 overlap fade. This offline composition is not a production capture.
- `selected/comparison-1x.png` and `comparison-3x.png`: first versus corrected source reconstruction.
- `source-crops-review.png`: annotated source plane extraction only; never imported.

Cap opposite-edge RGB MAD is32.63 left/right and20.724 top/bottom, versus ordinary adjacent-pixel MAD6.18 and5.753. Face left/right edge MAD16.104 versus ordinary7.463. The module has straight gap-free geometry but visible repeated motif and color/mortar discontinuities; no seamless claim is made.

Frozen output SHA256: `69d9bd08e777a9a17f45d11e09836fa81b6918fd076fd5e202a936a56c859e0b`.

Plane recipe SHA256: `5d05fa0ac827bc9d40c226f3f047ff2628f35e686c612ce4af3c7d0ec77b6cd1`.

Plane report SHA256: `ce94082039e48a393130bbbd46dca51afd45781a06d05ccb6332a9d4e1dd836d`.
