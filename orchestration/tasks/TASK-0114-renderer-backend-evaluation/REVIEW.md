---
task: TASK-0114
title: Stage-2 renderer backend evaluation matrix
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 7b8374ea
reviewed_at: 2026-08-23T22:25:00Z
revision: 1
---

# Review — TASK-0114 (Stage-2 renderer backend evaluation matrix)

## Verdict: ACCEPTED

Frozen head `7b8374ea` (worker branch `worker/verdigris/pc/ox-pc-bb`), content
head `adf17d7b`, reviewed in detached worktree `review-task0114-7b8374ea`.

## Scope

Worker-only delta `84ede1a0..7b8374ea` touches only
`orchestration/tasks/TASK-0114-renderer-backend-evaluation/**` (EVALUATION.md,
REPORT.md, STATUS.md, captures/source-index.json). Read-only research capsule
honored (no downloads, builds, dependencies, or ports; port 6500 untouched).
`git diff --check` clean.

## Acceptance gates

1. `node -e "...source-index.json...; console.log('source index: PASS')"` →
   prints `source index: PASS`, exit 0. **20 primary sources** with URLs +
   access dates + upstream pins.
2. `rg -n "Windows|macOS|sprite|atlas|shader|text|offscreen|license|CMake|GDI" EVALUATION.md`
   → 84 lines, exit 0 (all criteria present).
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → owned additions only, exit 0.

## Evidence quality

- EVALUATION.md is excellent and rigorous: all 5 candidates (D3D11, GL 3.3
  core, SDL2+batcher, sokol_gfx @ `7cee0ba1`, optimized GDI null) × every SPEC
  criterion, each with primary-source citation, a full comparison matrix, risk
  table R1-R11, a render-list-preserving migration sketch, and explicit
  unknowns U1-U6.
- **Negative control verified genuine (U1):** Apple's macOS OpenGL deprecation
  statement could not be retrieved from any live or archived primary source
  (the expected `developer.apple.com/library/archive` URL returns 404 and no
  Wayback snapshot exists) — the deprecation claim is **not asserted**
  anywhere; macOS-GL viability rests only on vendor support. Correctly
  preserved as UNKNOWN rather than inferred.
- Additional honest unknowns: sokol's missing public GPU→CPU readback API was
  verified by full-text search of the pinned header (U3/R7), SDL2 uncompressed
  DLL size unmeasured (U2), sokol binary weight unmeasured (U5), osmesa not
  researched (U6).
- **Recommendations (sokol_gfx, SDL2) clearly labeled as recommendations, not
  decisions**; the ADR is explicitly delegated to architect + owner. D3D11/raw
  GL/GDI are rejected with reasons on record.
- Machine-readable twin `captures/source-index.json` parses (20 sources).

## Capsule

Read-only research respected: no downloads/builds/deps/ports, port 6500
untouched, only owned task-folder paths changed.

## Follow-up

The owner/architect should take the two recommended candidates (sokol_gfx,
SDL2) into the Stage-2 ADR, resolving U1 (Apple GL policy) and prototyping the
sokol capture path (R7/U3) as the first milestone. No dependency is added here.
