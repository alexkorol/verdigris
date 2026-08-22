---
id: QUESTION-0011
task: TASK-0053
status: RESOLVED
asked_by: kimi (Kimi Code CLI)
---

# TASK-0053 deliverable 3 (clustered accents) — generation lives outside owned paths

## Evidence

TASK-0053 deliverable 3 asks for floor accents/flowers/water to "generate in
coherent clusters (seeded blobs)... deterministic per zone seed". But accent
generation is authoritative server-side, outside the task's owned paths
(`src/core/rendering/**`, `tests/**`; `server/**` forbidden):

- `server/core/map.js:861-874` — `accentPool = theme.floorAccents()`;
  `floorPicker()` picks an accent with a flat 12% per-tile chance (the
  one-cell checkerboard noise the deliverable targets). Runs inside
  `generateInstance`, seeded per zone.
- The client renderer only DRAWS the baked result:
  `src/core/rendering/terrain-renderer.js:250` bakes via
  `map.bakeGroundTexture` (`src/core/map.js`, also outside owned paths), so
  even suppressing the scattered accents at bake time is out of reach.

A renderer-only alternative exists (deterministic decal overlay keyed on
world coordinates) but it would double-source floor visuals against the
authoritative map data and could fight future server-side themes.

## Options

1. Expand TASK-0053 ownership to `server/core/map.js` (floorPicker/accent
   pass only) so clustering is implemented where generation actually lives —
   deterministic per zone seed for free (the existing seeded rng). RECOMMENDED:
   smallest honest change, matches the deliverable's wording.
2. Renderer-side decal overlay in `src/core/rendering/**` (owned as specced),
   accepting the double-sourcing risk above.
3. Drop deliverable 3 from TASK-0053 (verdict NOT-NEEDED-here) and respec it
   as a server task.

Deliverables 1 (exposed-face walls) and 2 (tree-line boundaries) are
implemented renderer-side, tested, and capture-verified independently of
this answer.

## Resolution

The clustered-accent work was split to TASK-0057, accepted, and integrated.
This ownership seam is historical and does not block the surge board.
