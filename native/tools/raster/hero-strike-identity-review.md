# SE strike identity candidate comparison

V3 successfully reduces the enlarged head, but it does not finish the idle-to-strike style repair. V4 improves the edge treatment while retaining the enlarged head. Root viewed and adopted V3 as a measured improvement over the prior runtime. Production motion and socket review remain pending. V4 is rejected for this repair.

## Method and reproducibility

Both saved sources were reconstructed with the unchanged `import_assets.py` CLI and the actual `Z:/Code/Python/pixel-perfecter` workspace API. The candidate manifests are `hero-strike-identity-v3.json` and `hero-strike-identity-v4.json`. Outputs and complete source/tool hashes are under `.ci-artifacts/hero-strike-identity/v3` and `/v4`.

V3 uses the existing six source crops, cell size 7, authored row origins, an 80x96 canvas, crisp alpha and one 32-color palette across all six frames. Its RGB checker was removed through Pixel Respecter's border-connected fill; one explicit reviewed armpit window was included. Native content heights remain 62–63 pixels and the ground is y95.

V4 is an isolated source with a 610-pixel body. Cell size 10 reconstructs a 61-pixel body. Both this raw result and a whole-sprite nearest-neighbor normalization to the idle's 63-pixel body are retained. The latter is solely a common-height comparison of one ready pose; it is not individual fitting of frames in a motion cycle.

The native and 4x ready comparison, all six old/new pose pairs, and idle→six poses→idle contact sheets were visually inspected. An assembled GIF retains 400ms idle holds and 50ms attack slots for later playback review; the still inspections do not establish production motion acceptance.

## Measurements

| Asset | Body height | Head width, top six occupied rows | Near-black contour fraction |
|---|---:|---:|---:|
| Accepted idle | 63 | 10 | 0% |
| Current strike ready0 | 63 | 13 | 82% |
| V3 ready0 | 63 | 11 | 63% |
| V3 six-pose range | 62–63 | 10–11 | 40–78% |
| V4 raw cell10 ready0 | 61 | 12 | 0% |
| V4 normalized-height ready0 | 63 | 13 | 0% |

Head width is the union of occupied x positions over the first six occupied rows, matching the prior comparison. The contour statistic counts outer silhouette pixels whose maximum RGB channel is at most 20. Accepted idle uses dark material colors instead of this near-black range. This diagnostic quantifies the observed edge difference; it is not a general visual quality score.

V3's opaque-body intersection-over-union with the corresponding current frame, below y45, is 94.6%, 94.9%, 95.2%, 95.7%, 97.1%, and 89.8%. The source is not pixel-identical outside the head. Recovery5 has the largest change and needs renewed socket review if adopted.

## Viewed findings and decision

- V3 narrows the hair/head in every pose while preserving full trousers, right shoulder armor, left sash and the physical-left-hand strike. There is no new limb swap or changed action order. Preparation, extension/contact and retraction remain distinguishable at native scale.
- V3 preserves the six pose silhouettes closely, with small edge and foot changes; recovery5 changes most. Existing equipment sockets must not be assumed exact for a replacement.
- V3 still has visibly darker outlines and a redder, more textured tunic/leg treatment than accepted idle. Entry and return still change rendering style, despite the head-size improvement. Raised fists also appear immediately at the ready0 boundary.
- V4's brown material edges are visibly closer to idle. At equal body height its head remains 13 pixels wide, so it does not solve the enlarged-head transition. It supplies only one pose and must not replace the six-frame sequence.
- V3 was subsequently adopted after root review; the prior runtime was not a gold standard, and the residual issues do not erase the verified head-size improvement. V4 is rejected for the requested repair. No runtime, importer, renderer or header changes were made in this comparison. No new imagegen call was made.

The next bounded source repair should use V3's narrower head and existing six poses as the edit target, with accepted idle as the separately identified identity/style anchor. The remaining property is the dark contour and material/palette jump; asking again to resize every head would repeat a solved part. Use the corrected published reference arrangement in `PROMPTING.md` (edit target first, opening identity second), without claiming that order alone guarantees success. Before another six-frame pass, verify one repaired ready pose against idle and V3 at the same native body height.

## Review artifacts

- `hero-strike-identity-ready-compare-1x.png` and `-4x.png`
- `hero-strike-identity-v3-transition-1x.png` and `-4x.png`
- `hero-strike-identity-v3-pose-compare-1x.png` and `-4x.png`
- `hero-strike-identity-measurements.json`
- `.ci-artifacts/hero-strike-identity/compare.py` reproduces diagnostics without modifying assets.

## Root adoption

After viewing the V3 transition, root approved adoption as a concrete identity improvement. The six `hero_strike0_se` through `hero_strike5_se` runtime PNGs and production provenance were rebuilt from V3 through the existing importer. Every runtime hash matches its viewed candidate. Canvas80x96, cell7, anchor[40,96], binary alpha, shared32colors, and10–11px heads were verified. Production motion acceptance remains pending; darker outlines, redder material shading and abrupt raised-hand entry are retained as known roughness. Equipment review was notified to remeasure changed hand pixels. `hero-strike-identity-adoption.json` records exact hashes. Prior v1/v2 runtime pixels were copied into the diagnostic directory so the comparison remains reproducible after adoption. No imagegen call was made.
