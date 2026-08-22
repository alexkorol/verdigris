# TASK-0120 claim

- task: TASK-0120
- state: CLAIMED
- coordinator: ox-pc-e
- worker: ox-pc-e (isolated PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0120-release-verification-gap-audit-ox-pc-e`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-e`
- route/base SHA: `039dcfa7f12497aa79c3677873a06a96c231a13d` (coordination-only route refresh on top of immutable code base `42718fbc4340589e606fff94a6eaa3dfbd03ad1c`, verified ancestor of HEAD)
- started-at: 2026-08-21 21:47 -07:00 session wall-clock; exact claim commit clock is this commit's author/committer time
- ports: 6700-6719 reserved for this lane (resource capsule read-only expected; port 6500 untouched)
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: MECHANICAL / INDEPENDENT packet

## Experimental-unit configuration provenance

- endpoint: local OpenCode CLI session in `Z:\Code\.worktrees\verdigris\ox-pc-e`
- provider: `openrouter` (harness-visible model id `openrouter/stealth/ox-alpha`)
- model: `stealth/ox-alpha`
- variant/reasoning: not observed in this session; omitted rather than guessed
- harness: OpenCode CLI 1.18.21
- agent persona: ox-alpha

## Routing provenance

- `START_HERE_OX_PC_E.md` launch packet at this worktree routes ox-pc-e to
  TASK-0120 from base `039dcfa7` (immutable base `42718fbc`) per RUN_STATUS.md
  effective READY table row "TASK-0120 release verification audit".
- Fresh fetch performed immediately before claim: no competing STATUS.md,
  no RELEASE.md in the task folder, and no origin branch matching TASK-0120
  or ox-pc-e existed.
