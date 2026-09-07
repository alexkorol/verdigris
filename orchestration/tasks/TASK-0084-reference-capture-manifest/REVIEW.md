---
task: TASK-0084
title: Reference-capture integrity manifest
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 3e84a878
reviewed_at: 2026-08-23T23:25:00Z
revision: 1
---

# Review — TASK-0084 (reference-capture integrity manifest)

## Verdict: ACCEPTED

Frozen head `3e84a878` (worker branch `worker/verdigris/pc/ox-pc-bb`), evidence
head `c0f1f34e`, reviewed in detached worktree `review-task0084-3e84a878`.

## Scope

Worker-only delta `30a96556..3e84a878` touches only owned paths:
`orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.{mjs,json}`
and `orchestration/tasks/TASK-0084-reference-capture-manifest/**` (REPORT.md,
STATUS.md). No image evidence rewritten; no binary dependency added; read-only
capsule honored. `git diff --check` clean.

## Acceptance gates (run literally at frozen head)

1. `node --check .../reference-manifest.mjs` → exit 0.
2. `node .../reference-manifest.mjs --write` → regenerates **30 entries** (10
   native, 5 browser, 15 composite), exit 0, **byte-idempotent** (git status
   clean after regeneration).
3. `node .../reference-manifest.mjs --verify` → `verification OK ... 30 entries`,
   exit 0.
4. `git diff --check` → clean, exit 0.
5. Authentic negative (one bad hash on a disposable copied manifest) → exit 1
   with a precise mismatch report (per REPORT.md); disposable copy removed,
   evidence untouched.

## Evidence quality

- `reference-manifest.mjs` is dependency-free (node builtins only) and
  implements the full SPEC contract: five-scene naming matrix, PNG IHDR / JPEG
  SOF dimension parsing without pixel decode, SHA-256 hashing, render-list JSON
  structural validation, and failures for missing/duplicate/zero-byte/
  wrong-resolution/malformed/unmanifested evidence. Default read-only verify;
  `--write` regenerates only the manifest.
- `reference-manifest.json` is well-structured: `{schemaVersion, generator,
  benchmark, entryCount, entries[]}` with 30 entries carrying path, byteLength,
  dimensions, sha256, scene, side, and sourceRevision.
- The worker honestly disclosed and fixed a cwd-relative `--manifest` resolution
  bug (`91630905`) found during the negative test, before final transcripts.

## Capsule

Read-only image-evidence verification respected: no image rewritten, no binary
dependency, no ports, port 6500 untouched, only owned paths changed.

## Follow-up

Wire the verifier into CI (or a task gate) so the frozen reference benchmark is
regression-checked on every change. The 30-entry manifest is ready as the
integrity oracle for presentation regression evidence.
