# Village construction props

Four restrained props for the native Village Palisade. These are material-painted derivatives of the existing editable village composition, not new gameplay or an alternate demo.

`manifest.json` is ready for `native/tools/install-first-slice-art.py`. It is installed in the native Village Palisade scene. All canvases are 384×384, source projection48pixels/metre and pivot192,354. Longhouse pivot is the front doorway ground line; tree and palisade pivots are ground roots, well pivot is ground centre. The house wall footprint is3.8×5.8metres (roof4.6×6.5); palisade section is3metres; well opening construction is1.16metres across. Geometry stays in world scale; no object was resized by its bounding box.

The saved Blender scene was isolated at a straight45-degree elevated orthographic camera. `source/render_isolates.py` reproduces native guides from the composition blend. `source/transfer.py` produces the final cutouts and reviews. `review-1x.png` is the actual source scale composited on grey; `review-2x.png` is nearest-neighbour enlargement.

Two built-in ImageGen calls were used. The first square atlas drifted in layout and is rejected. The correction used a1536×1024 guide plus a coarse nearest-neighbour sprite example. It retained ground contacts but changed fine geometry slightly. Original guides, generated RGBA files and exact prompts are retained in `source/`. Generated images have true alpha; the transfer reconstructs binary coverage, selects a real RGB sample per fixed2×2 block and clears hidden RGB. This is deterministic fixed-grid sampling, not a claim that ImageGen obeyed a perfect logical-pixel lattice. See `review.json` for measured bounds and limitations.

Agent visual review passed for these four cutouts at native scale. The integration lane also reviewed the production native scene. Owner acceptance remains open. Do not infer approval of the rejected first atlas or the older huts/trees.
