---
task: TASK-0094
title: Native asset provenance manifest audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 8b561963
reviewed_at: 2026-08-23T20:50:00Z
revision: 1
---

# Review — TASK-0094 (native asset provenance manifest audit)

## Verdict: ACCEPTED

Frozen content head `8b561963` (worker branch `worker/verdigris/pc/ox-pc-bc`,
pushed tip `140337cb`; STATUS REVIEW_REQUESTED). Reviewed in detached worktree
`review-task0094-8b561963`.

## Scope

Worker content commits (`5cdfed6e..8b561963`) touch only
`orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/assets.json,
captures/hash-assets.mjs, 4 acceptance transcripts). No asset/license/manifest
changed; only already-present files hashed; no network downloads; WIZARD/
reference assets recorded as candidates, not copied. Read-only capsule honored.
`git diff --check` clean.

Process note: the worker's claim STATUS.md was misrouted to the program branch
(commit `5cdfed6e`, benign coordination doc), while the real TASK-0094 work
lives on `origin/worker/verdigris/pc/ox-pc-bc` at `140337cb`. The worker branch
has been pushed correctly; merging stays clean since the claim is already an
ancestor of the program head.

## Acceptance gates

1. `rg -n "asset|atlas|terrain|splash|orb|png|jpg|bmp" native docs/rebuild docs/product --glob ...`
   → 275 lines, exit 0.
2. `node -e "...assets.json...; console.log('asset manifest: PASS')"` → prints
   `asset manifest: PASS`, exit 0. **179 assets** inventoried.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → owned additions only, exit 0.

## Evidence quality

- FINDINGS.md is excellent and precise: 179 assets across 20 families with
  per-file relative path, type, dimensions, bytes, sha256, provenance evidence,
  license status, build/package use, and KEEP/UNKNOWN/BLOCKED classification
  (150 KEEP / 29 UNKNOWN / 0 BLOCKED — BLOCKED correctly empty since every
  policy-failing asset also lacks license evidence, landing in UNKNOWN).
- **Native boundary verified genuine:** the only asset bytes in the shipped
  native executable are the procedural shapes embedded in `visual_kit.h`
  (deterministic TASK-0141 generator). `rg "fopen|ifstream|CreateFile|LoadImage"
  native/client` → only 3 `ifstream probe` calls, all verified as
  capture-integrity checks of *output* PNGs (comments "Capture integrity: both
  PNGs exist"), not source-asset reads. The native client reads no asset files
  at runtime.
- **Negative control verified genuine:** 29 UNKNOWN assets named explicitly
  (fonts pixelmix/PxPlus/Px437, main_menu.mp3, legacy tiles terrain/objects,
  human.png, wizard orbs, favicon) remain non-shippable with no license
  inferred.
- **Discrepancy verified:** `server/maps/layers/objects.tsx` declares
  objects.png as 288×1024, but the actual PNG IHDR is **288×1056** (33 rows vs
  32) — stale Tiled metadata confirmed.
- Packaging successor (KEEP-only vendoring, content-hashed bundle manifest,
  file-free runtime boundary) is concrete and owner-gated.
- Machine-readable twin `captures/assets.json` parses (179 assets).

## Capsule

Read-only audit respected throughout: no asset/license/manifest changed, no
network downloads, WIZARD/reference assets not copied, only owned task-folder
paths changed.

## Follow-up

Packaging implementation (owner-gated) should take `assets.json` as intake
truth, resolve the 29 UNKNOWN licenses before selection, and reconcile the
objects.tsx/objects.png dimension drift before locking tile gids.
