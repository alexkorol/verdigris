---
task: TASK-0160
title: Native procedural visual-kit packaging proof
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: be1289b7
reviewed_at: 2026-08-24T00:05:00Z
revision: 1
---

# Review — TASK-0160 (native procedural visual-kit packaging proof)

## Verdict: ACCEPTED

Frozen head `be1289b7` (worker branch `worker/verdigris/pc/ox-pc-bd`),
implementation head `9473009c`, reviewed in detached worktree
`review-task0160-be1289b7`.

## Scope

Worker-only delta `f81a303b..be1289b7` touches only owned paths:
`native/tools/verify_native_visual_kit.py` and
`orchestration/tasks/TASK-0160-native-visual-kit-packaging-proof/**`
(REPORT.md, STATUS.md, run_negative_tests.py, 9 fixture kits). No
runtime/client paint change, no final art, no raster asset, no third-party
dependency. `__pycache__`/`.pyc` correctly not committed. `git diff --check`
clean.

## Acceptance gates (run literally at frozen head)

1. `python native/tools/verify_native_visual_kit.py --check` → exit 0,
   prints `verify_native_visual_kit: OK (kit reproduces byte-for-byte)`.
2. `python .../run_negative_tests.py` → **15 tests: 15 passed, 0 failed**.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → empty (owned additions committed), exit 0.

## Evidence quality

- `verify_native_visual_kit.py` is a clean stdlib-only validator: `--check`
  proves read-only that all 9 manifest entries exist as valid, bounded (64×64),
  safe SVGs (allowlist parsing; DOCTYPE/script/image/URL rejection), map to
  exactly one stable symbol in identical order with matching version metadata,
  and reproduce `manifest.json` + `visual_kit.h` **byte-for-byte** from SVG
  sources alone (integer-only hex math, no float/RNG paths). `--regenerate`
  rewrites the two derived artifacts and refuses on structural errors.
- **Negative suite verified:** 9 fixture kits (missing/duplicate/unknown
  entries, unsafe/malformed SVG, palette drift, stale header, ordering swap,
  positive control) + hand-written golden header oracle + determinism/
  read-only/regen-refusal controls → 15/15 pass.
- The transport-bound/asset contract is reproducible and dependency-free, and
  the runtime fallback remains embedded and asset-neutral. No final art,
  lore, naming, or aesthetic judgment added.
- REPORT.md records literal transcripts + exit codes; STATUS flipped to
  REVIEW_REQUESTED at frozen head `be1289b7`.

## Capsule

Read-only asset verification respected: no asset rewritten, no runtime/client
paint change, no ports/network, port 6500 untouched, only owned paths changed.

## Follow-up

Wire `verify_native_visual_kit.py --check` into CI (or a task gate) so the
committed vector/procedural kit is guaranteed reproducible and present before
every owner launch. This closes the "silently disappearing placeholders" risk
the packet was created to address.
