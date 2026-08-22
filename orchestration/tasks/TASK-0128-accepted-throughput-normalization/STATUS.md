# TASK-0128 claim

- task: TASK-0128
- state: REVIEW_REQUESTED
- transitioned-at (commit clock): recorded in this revision's commit metadata (wall-clock note: ~22:07 PDT 2026-08-21; approximate, not used for durations)
- coordinator: ox-pc-a
- worker: ox-pc-a (only registered PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0128-accepted-throughput-normalization-ox-pc-a`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-a` (same provisioned isolated worktree, re-registered for this task)
- base SHA: `31d215793f0f799fd365f080ca326ea04e83706c` (merge integrating accepted TASK-0081)
- started-at (commit clock target): 2026-08-21 21:38 PDT (-07:00) session wall-clock; exact claim commit clock is this commit's author/committer time
- ports: none required (read-only Git/task evidence capsule; Node.js 22); port 6500 untouched
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: MECHANICAL / INDEPENDENT / IMPLEMENTATION packet

## Experimental-unit configuration provenance (normalized)

- endpoint: local OpenCode TUI session in `Z:\Code\.worktrees\verdigris\ox-pc-a`
- provider (harness-visible): `opencode`; upstream provider remains unknown
- model id: `x-preview-f-free`; variant: `max` (per saved OpenCode session metadata)
- agent alias: `ox-alpha` persona on that model/variant
- NOT OpenRouter: recorded as OpenRouter only if current harness/session evidence ever proves that selection
- harness: OpenCode CLI/TUI; version not exposed by `opencode --version` on this PATH (recorded unknown, never guessed)

## Routing provenance

- pushed RUN_STATUS (`af3b61d8`, refreshed by tip `42718fbc`) explicitly routes
  ox-pc-a to TASK-0128 from base `31d21579` after the accepted+integrated
  TASK-0081 rev3 `dff39173` (merge `31d21579`). Claimed per STANDING-LOOP with a
  fresh fetch immediately before branching.

## REVISE round 1 (review of bb67c566)

- review verdict received: REVISE at reviewed head `bb67c566`
  (origin/codex/native-reconstitution REVIEW.md, 2026-08-22 01:35 -07:00);
  collector design, UNKNOWN/null discipline, aggregation key, path containment,
  and test battery explicitly remain accepted foundations
- release-blocking defect: captures bound `repo_revision` to the commit lineage
  containing their own bytes, so every capture commit invalidated its own
  evidence; the architect gate reproduced this by write-mode regenerating both
  capture files from `0d1898bd` to `bb67c566` (those two dirty files were
  preserved untouched until this revision replaced them under the corrected
  scheme)
- this revision: explicit deterministic evidence/source revision (write mode
  binds the head at write time, i.e. the implementation parent of the capture
  commit); `--check` now verifies that revision resolves, is an ancestor of
  HEAD, and that no relevant input evidence changed in between before
  byte-comparing recomputed output; schema bumped to v2; regression cases 11-13
  added; all literal SPEC gates and the tamper negative control rerun
- runway semantics unchanged and still honest: `hours:null` / confidence
  `UNKNOWN` for lane ox-pc-a; no rate guessed
- state remains REVIEW_REQUESTED, now for this revision
