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
