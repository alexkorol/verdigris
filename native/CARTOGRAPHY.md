# Cartographer expeditions

The production `WorldSimulation` now generates its collision, room graph, encounter anchors and treasure locations with the original WIZARD Cartographer grammar. Native C++ owns gameplay; the image is only a floor material. This is an ARPG-inspired procedural foundation within Verdigris's existing native renderer, not a claim of AAA art or content volume.

## Generation contract

`content/expedition_generator.hpp` is deterministic C++20 with independent random streams for topology, room variants and encounters. Each run builds a required entrance-to-guardian route, optional treasure branches, bounded loops, and reciprocal three-tile sockets. Grid realization, safe encounter population, navigation distance tiers and the exit all derive from that graph. Pillars preserve central lanes. Room rotation is retained as an authoring field; the current symmetric templates do not require transformed geometry.

Dungeon/crypt use limestone rooms, marsh/grove use islands and bridges, wilds use a volcanic quarry. Clearings, warrens and gauntlets select different branch/loop budgets. Guardian loot, item pools, progression and actor rules remain owned by the existing simulation. The fourth sanctuary recipe is available in the shared generator for future content selection.

The vendored `content/wizard/` JavaScript files match WIZARD commit `d383b00d208f62c5922d1be669f23e4219c468fa`. `tools/check_cartography_parity.cjs` checks 1,200 seed/size/recipe combinations against C++ fingerprints covering tiles, topology, sockets, variants, tiers and encounter population. Update both ports together; a browser screenshot is not a parity test.

## Runtime and map reading

Normal login and scene-transition messages carry collision rows, terrain IDs and room landmarks. The native client paints this authoritative geometry. Dungeon floors use the generated limestone material; void, water, lava and bridges have distinct treatments. Room dressing stays clear of central lanes.

Press **M** during an expedition to open the north-up atlas; **M** or **Escape** closes it. The small map and atlas show explored walkable space, using an eight-tile visibility radius with wall occlusion. Landmarks become named on discovery. Unknown guardians and extraction markers stay hidden; changing the map revision resets discovery even when the scene name stays the same. Existing minimap zoom/opacity controls remain available.

Research and source distinctions: [WIZARD research](https://github.com/alexkorol/WIZARD/blob/codex/cartographer-expeditions/tools/cartographer/RESEARCH.md). The generator applies authored-room/socket and learnable-entry principles, rather than copying Diablo II or Path of Exile layouts or assets. Asset prompt and origin are in `client/assets/wizard/cartographer/PROVENANCE.md`.

## Verification

Build using `./native/build.ps1 -RunTests`. Run `native/build/verdigris_client.exe --scenario all` with a contained `VERDIGRIS_CAPTURE_ROOT`. The `cartography` scenario renders the actual generated terrain and atlas, checks hidden guardian/discovery reset/material loading, and measures the textured frame against 40 ms. Core integration tests exercise all fifteen production theme/layout combinations, reachable loot and nonoverlapping safe monster placement. Session journeys route over the published grid instead of assuming the retired 40x40 maze.

See `docs/rebuild/cartography/VERIFICATION.md` for checkpoint results and live captures.
