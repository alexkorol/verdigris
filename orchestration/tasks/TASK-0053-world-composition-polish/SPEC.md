---
task: TASK-0053
title: World composition polish — wall faces, tree-line boundaries, clustered accents
state: READY
priority: medium (browser visual polish; salvaged from superseded PR #3)
owned_paths:
  - src/core/rendering/**
  - tests/**
  - orchestration/tasks/TASK-0053-world-composition-polish/**
forbidden_paths:
  - server/**, native/**, playtest/** (assertions; new scenarios fine)
base: current program tip
architect_review_required: true
---

## Provenance

Salvaged from closed PR #3 (`codex/recover-merge-refinements`,
pre-orchestration recovery, superseded at 565 commits behind). The
diff is dead; these three composition ideas may still be valuable.
FIRST verify each against current master — any already-present item
becomes NOT-NEEDED in the REPORT with evidence (screenshot), and you
skip it. Reference implementation sketches exist in that branch's
`de6b1ae4`/`00cdd5aa` commits (read-only inspiration; do NOT merge or
cherry-pick them — reimplement on the current renderer).

## Deliverables (each independently owner-visible)

1. **Exposed-face-only indoor walls.** In dungeon interiors, render
   wall faces only where a wall cell borders walkable floor; interior
   wall mass stays dark room-mass. Kills the "repeated brick
   wallpaper" read. Check first: the 25d overhaul may already do this.
2. **Tree-line collision boundaries.** In Grove/Wilds, blocked cells
   at the edge of walkable space render as dense tree-line/vegetation
   matching the ground palette — never masonry carpet or black void.
   Collision behavior unchanged (presentation only).
3. **Clustered accents.** Floor accents, flowers, and water generate
   in coherent clusters (seeded blobs) rather than one-cell
   checkerboard noise. Deterministic per zone seed.

## Constraints

- D-108: the webchat-Fable demo (docs/reference/25d-overhaul/) remains
  the look/feel authority — match its readability, not PR #3's exact
  pixels.
- No perf regressions: state before/after frame-cost numbers using the
  existing perf capture method from 25d phase 5.

## Acceptance evidence (0038 standard)

1. Hard-fail Playwright captures: Old Barrow interior (wall faces +
   room mass), a Grove boundary (tree-line), a village accent cluster.
2. `npm run test:unit` + `npm run smoke:browser` + full
   `npm run playtest` transcripts, default path.
3. Per-deliverable verdict: IMPLEMENTED / ALREADY-PRESENT (+evidence)
   / NOT-NEEDED (+reason).

Architect inspects captures and reruns gates personally (G5).
