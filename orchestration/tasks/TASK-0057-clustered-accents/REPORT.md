---
task: TASK-0057
state: REVIEW_REQUESTED
coordinator: deepseek
worker_branch: codex/TASK-0057-clustered-accents-deepseek
base_commit: 79723db35c4d2873d3c5af3fda13a4503337236d
architect_review_required: true
---

# TASK-0057 REPORT — Clustered floor accents (seeded generation-side clusters)

## Executive summary

Procedural instance floors now paint their themed accents as **coherent
seeded blobs** (blob growth from deterministic seed cells) instead of the
old one-cell `rng() < 0.12` checkerboard. Density stays at the 12% budget
(exactly `floorCount * 0.12`, so within the spec's ±10%).

The accent pass runs on its **own rng stream** derived from the floor seed,
and `floorPicker` still consumes the old accent-decision call (discarded),
so the **main rng stream — decor, water, monster, and item placement — is
byte-identical** to the pre-clustering generator. Revisiting a floor by seed
produces the same monsters/items/stairs as before; only the accent tiles move
from noise to patches.

## Changed files

- `server/core/map.js` — `floorPicker` no longer emits checkerboard accents;
  new `Map.paintAccentClusters({...})` static method (blob growth, capped at
  20 cells/blob, growth 0.55 below percolation threshold); called after carve
  on a seed-derived stream.
- `tests/unit/map-accent-clusters.spec.js` — 4 new unit tests (clustering,
  determinism, checkerboard contrast, full-instance determinism).
- `orchestration/tasks/TASK-0057-clustered-accents/captures/` — hard-fail
  Playwright capture + 2 PNGs + evidence JSON.

No `playtest/**` changes; no `src/**` changes; no `native/**` changes.

## Design note (why the main stream is preserved)

The old `floorPicker` consumed exactly two rng calls per carved cell when the
theme had an accent pool (the `rng() < 0.12` decision + the pool pick). The
new `floorPicker` keeps consuming both calls — the decision call is now
discarded and the pick always comes from the floor pool. `paintAccentClusters`
is fed `Map.createSeededGenerator((seed ^ 0x9e3779b9) >>> 0)`, a distinct
stream, so nothing downstream shifts. Accents are walkable floor variants, so
walkability/connectivity and the flood-fill guarantee are untouched.

## Test commands and outcomes

`npm run test:unit`:

```
 Test Files  131 passed (131)
      Tests  830 passed (830)
```

`npm run playtest` (PLAYTEST_PORT=6540, loopback):

```
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":32.19,"maxEventLoopLagMs":111.02}
```

`npx playwright test tests/e2e/browser-critical-loop.spec.mjs`
(server on 127.0.0.1:6542):

```
  1 passed (18.6s)
```

Capture (server on 127.0.0.1:6542):

```
CAPTURES OK {"villageRendered":true,"zoneRendered":true,"zoneEntered":true}
```

## Authentic negative

Set `growth` to `0` in `paintAccentClusters` (no blob growth → every accent
an isolated singleton) and reran the unit test:

```
 FAIL  ... > paints accents as contiguous blobs at the density budget
AssertionError: expected 0.4267241379310345 to be greater than 0.7
 FAIL  ... > clusters contrast sharply with per-cell checkerboard noise
AssertionError: expected 0.4267241379310345 to be greater than 0.5387931034482758
```

Restored `growth = 0.55`; test green again (4/4). The 0.427 "growth=0"
adjacency matches the old checkerboard's expected ~0.4, confirming the test
measures clustering, not noise.

## Scope boundary (flagged for the architect)

The spec names "floor accents, flowers, and water". In `map.js`:

- **Floor accents** (the `floorPicker` checkerboard) — implemented as clusters
  (this is the title deliverable).
- **Water** — already generated as a coherent 2×2 pool (not one-cell noise),
  left as-is.
- **Flowers** in instances — already the decor pass scatter (deliberate cover
  distribution, not checkerboard), left as-is.

The literal "one-cell checkerboard" flowers live in
`server/core/world-layout.js` (`addFlowers`/`addGrove` use
`hash(x,y,salt) % N < density`), which is **not in this task's owned_paths**
(owned paths are `server/core/map.js`, `server/core/generation/**`,
`src/core/rendering/**`, `tests/**`, task folder). The `01-village-overworld.png`
capture therefore shows the overworld unchanged. If the architect wants the
overworld village flowers clustered too, that needs either world-layout.js
added to owned_paths here or a follow-up task.

## Commits

- `32fdcdf8` — claim (STATUS.md)
- `23749d26` — implementation (clustered accents + tests + captures)
