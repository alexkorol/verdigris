# Wight SW walk — frozen native candidate

Root approved these exact eight cleaned v1 poses for production import after
viewing the native1x and3x idle/cycle strips. Final acceptance requires the
parent's production-game motion review. The asset lane did not alter runtime,
catalogs, equipment or game code. The renderer owns byte-identical promotion.

The skeleton retains its skull, exposed ribs, curled empty hands and torn brown
cloth. The torso is narrower and the legs more exposed than the accepted idle.
Cloth still partly obscures hip-to-knee ownership; this package does not certify
perfect anatomical alternation. A forward foot appearing lower-left in both
contacts does not itself prove the same leg, because the character faces SW.

All eight original row-major poses are retained. No isolated v2/v3 repair is
spliced in. Phases0/1 have a forward lower-left bone foot and lifted rear foot;
2 narrows the stance;3 opens it;4/5 carry a wide stance with shifted upper body;
6 narrows again;7 returns toward the opening pose. The pairs0/1 and4/5 are close.
The1x/3x browser playback retains the fixed canvas and makes the narrower idle
entry visible. Real-game cadence and translation may expose additional issues.

## Exact reconstruction

Run from `C:/Users/Alex/Documents/ChatGPT/verdigris-diablo-study`:

```powershell
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' 'native/tools/raster/import_assets.py' 'native/tools/raster/wight-walk-sw-candidates/selected/import.json'
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' 'native/tools/raster/wight-walk-sw-candidates/selected/review_walk.py'
```

The external Pixel Respecter project is called unchanged. Exact external source
hashes, its dirty state, Python/packages, importer and input/output hashes live
in `import.provenance.json`. `dependencies.json` records the minimal direct
reconstruction inputs, original reference inputs and selected review evidence.

- Canvas80×96; world pivot40,96; one source cell size6 for all frames.
- Source1619×971; four columns at405px pitch; row centers at450px pitch.
- Source world origin210,486 in row0, with the same world offset in row1.
- Body heights59–63px versus61px idle; genuine per-frame lift is preserved.
- 30 visible colors, all from the accepted idle's palette; binary alpha.
- Preview100ms/frame; idle holds320ms. Preview timing is not runtime policy.

## Alpha result and rejected attempts

All three built-in calls requested real alpha but returned RGB painted checkers.
The first broad connected fill removed part of phase1's pale hand. It remains
rejected in `../v1/`. The selected recipe uses the same actual engine API with
a blue-biased distance center and538 exact neutral row-run windows in20
visually reviewed enclosed gaps. The windows remove4,097 neutral pixels and no
chromatic foreground;12 source matte-edge pixels are removed by border fill.
Retained source RGB is unchanged. The source originals are unchanged.

View `alpha-source-comparison.png`, `hand-alpha-comparison.png` and
`chromatic-edge-review.png` for the inspected source/alpha evidence. Native
phase1 has its full pale skeletal hand, and the white arm gaps are transparent.

The v2 isolated edit did not resolve the leg-ownership ambiguity. The v3
geometry-first edit exposed the femur more clearly but changed torso/cloth
identity. Root selected v1 consistency. Exact prompts, references and source
hashes are in the three `prompts/wight-walk-sw-v*.provenance.json` records.
Those local outcomes are separate from the linked creators' original reports.

`staging-selected.txt` lists selected new files. `staging-experiments.txt`
separately lists rejected source/prompt/alpha evidence without requiring whole
candidate folders. Once runtime byte parity is recorded, duplicate candidate
PNGs can remain unstaged if the root chooses. Existing runtime reference PNGs
and importer are dependencies, not new asset-lane changes.
