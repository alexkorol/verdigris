# TASK-0136 replacement claim (post-RELEASE)

- task: TASK-0136
- state: CLAIMED
- coordinator: ox-pc-h
- worker: ox-pc-h (isolated PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0136-passive-tree-contract-validator-ox-pc-h`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-h`
- route/base SHA: `d2311bc8e1a8bea7f7efb210737847117d235277` (routed HEAD; immutable SPEC base `be6d555688619819084b352660fc0336a90d0ec3` verified ancestor of HEAD via `git merge-base --is-ancestor`)
- started-at: 2026-08-22 01:35 -07:00 (immediately after `RELEASE.md` timestamp; exact claim commit clock is this commit's author/committer time)
- ports: 6760-6779 reserved for this lane (loopback only; port 6500 untouched)
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- clone path: `Z:\Code\.worktrees\verdigris\ox-pc-h`
- task family: IMPLEMENTATION / INDEPENDENT packet (passive-tree contract validator CLI, MECHANICAL)
- dependency: TASK-0112 ACCEPTED and integrated (contract at `orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/passive-tree-contract.json`)

## Experimental-unit configuration provenance

- endpoint: local OpenCode CLI session in `Z:\Code\.worktrees\verdigris\ox-pc-h`
- provider: `openrouter` (harness-visible model id `openrouter/stealth/ox-alpha`)
- model: `stealth/ox-alpha`
- variant/reasoning: not observed in this session; omitted rather than guessed
- harness: OpenCode CLI 1.18.21 (`opencode-ai@1.18.21`, npm global)
- agent persona: ox-alpha

## Replacement provenance

- Prior claims `6ea36f5a`/`7b24e5d3`/`7026892e` were released by
  `RELEASE.md` after a duplicate-dispatch collision in the ox-pc-c worktree.
- Per RELEASE and START_HERE: lane c's dirty worktree was not read from,
  cleaned, or reused; no collided implementation content is copied.
- This is a fresh independent clean worktree at the exact routed current
  program base `d2311bc8`. First post-RELEASE STATUS write wins.

## Routing provenance

- Preflight verified per AGENTS.md (`git status --short`, `git remote -v`,
  `git fetch --prune origin`, `git status -sb`): clean state, branch
  `codex/TASK-0136-passive-tree-contract-validator-ox-pc-h`, HEAD
  `d2311bc8e1a8bea7f7efb210737847117d235277`; branch had no upstream before
  this push (fresh worker branch, pushed with `-u origin`).
- No competing post-RELEASE `STATUS.md` existed in the task folder at claim
  time on this fresh fetch (first-post-RELEASE-write-wins honored).
- Scope honored: work confined to owned paths
  `orchestration/tasks/TASK-0136-passive-tree-contract-validator/**`;
  forbidden paths (`native/**`, `server/**`, `src/**`, `playtest/**`,
  CI/product values) will not be touched. No gameplay/item/campaign/setting/
  House/progression content decisions will be made.
