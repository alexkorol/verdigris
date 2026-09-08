# Cartographer expeditions

The production `WorldSimulation` now generates its collision, room graph, encounter anchors and treasure locations with the original WIZARD Cartographer grammar. Native C++ owns gameplay; the image is only a floor material. This is an ARPG-inspired procedural foundation within Verdigris's existing native renderer, not a claim of AAA art or content volume.

## Generation contract

`content/expedition_generator.hpp` uses deterministic C++20 terrain fields,
off-grid landmarks, varied eroded clearings, curved route brushes and smoothing.
The planning graph controls encounter destinations; it does not stamp physical
room boxes. Logical three-cell passage anchors stay traversable. Neighboring
routes can merge naturally in the shared terrain.

Production wilds/grove use woodland, marsh uses tidal wetland, and dungeon/crypt
use eroded limestone interiors. Volcanic badlands and sanctuary are available in
the shared generator and have native render scenarios. Default production maps
are 96 × 78 cells; gauntlets are 114 × 78. Side-destination budgets are 1–3;
there are no repeated internal waystone destinations.

Normal server session creation supplies entropy. Each new instance or floor
visit derives a new seed from its session seed and visit serial, so repeatedly
entering the same road changes its layout. Explicitly seeded command sequences
remain reproducible. Same-socket re-login retains its instance; a new socket
follows the existing town-admission behavior while preserving account progress.
`WebSocketServer` accepts an optional replay seed for regression fixtures; the
production launcher omits it and receives entropy.

The vendored JavaScript matches WIZARD checkpoint `18633475b47140b6bd4480fd6c32e672b3bf4e04`. JavaScript and native are checked across 1,500 combinations
of five biomes, 100 seeds and three extents. Fingerprints cover terrain, graph,
passage anchors and encounters. Update both ports together.

## Runtime and map reading

Normal login and scene-transition messages carry collision rows, terrain IDs and room landmarks. The native client paints this authoritative geometry. Terrain IDs select separate generated grass, damp earth, basalt and limestone materials. Void, water, lava, canopy, outcrops and bridges have distinct treatments. Material loading uses downsampled plates and avoids painting a redundant underlying floor.

Press **M** during an expedition to open the north-up atlas; **M** or **Escape** closes it. The small map and atlas show explored walkable space, using an eight-tile visibility radius with wall occlusion. Landmarks become named on discovery. Unknown guardians and extraction markers stay hidden; changing the map revision resets discovery even when the scene name stays the same. Existing minimap zoom/opacity controls remain available.

Research and source distinctions: [WIZARD research](https://github.com/alexkorol/WIZARD/blob/codex/cartographer-expeditions/tools/cartographer/RESEARCH.md). The generator uses learnable entry direction and encounter pacing with continuous terrain. Asset prompts and origins are in `client/assets/wizard/cartographer/PROVENANCE.md` and `OUTDOOR-PROVENANCE.md`.

## Verification

Build using `./native/build.ps1 -RunTests`. Run `native/build/verdigris_client.exe --scenario all` with a contained `VERDIGRIS_CAPTURE_ROOT`. The `cartography` scenario renders the actual generated terrain and atlas, checks hidden guardian/discovery reset/material loading, and measures the textured frame against 40 ms. Core integration tests exercise all fifteen production theme/layout combinations, reachable loot and nonoverlapping safe monster placement. Session journeys route over the published grid instead of assuming the retired 40x40 maze.

See `docs/rebuild/cartography/VERIFICATION.md` for checkpoint results and live captures.
