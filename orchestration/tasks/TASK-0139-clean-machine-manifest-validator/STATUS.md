# TASK-0139 claim

- task: TASK-0139
- state: CLAIMED
- coordinator: ox-pc-g
- worker: ox-pc-g (isolated PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0139-clean-machine-manifest-validator-ox-pc-g`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-g`
- route/base SHA: `a631cb2e74e2b7463a9f9b3706684be8988b3c09` (routed HEAD; immutable SPEC base `be6d555688619819084b352660fc0336a90d0ec3` verified ancestor of HEAD)
- started-at: 2026-08-21 23:32 -07:00 session wall-clock; exact claim commit clock is this commit's author/committer time
- ports: 6740-6759 reserved for this lane (loopback only; port 6500 untouched)
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: MECHANICAL / INDEPENDENT packet

## Experimental-unit configuration provenance

- endpoint: local OpenCode CLI session in `Z:\Code\.worktrees\verdigris\ox-pc-g`
- provider: `openrouter` (harness-visible model id `openrouter/stealth/ox-alpha`)
- model: `stealth/ox-alpha`
- variant/reasoning: not observed in this session; omitted rather than guessed
- harness: OpenCode CLI 1.18.21
- agent persona: ox-alpha

## Routing provenance

- `START_HERE_OX_PC_G.md` launch packet at this worktree routes ox-pc-g to
  TASK-0139 from routed HEAD `a631cb2e` (immutable SPEC base `be6d5556`)
  per the RUN_STATUS.md effective READY table row for the clean-machine
  manifest validator.
- Preflight verified (`git fetch --prune origin` immediately before claim):
  clean state, branch
  `codex/TASK-0139-clean-machine-manifest-validator-ox-pc-g`, HEAD `a631cb2e`,
  upstream in sync (0/0), SPEC base an ancestor of HEAD, dependency contract
  TASK-0132 ACCEPTED and present at
  `orchestration/tasks/TASK-0132-clean-machine-harness-contract/clean-machine-contract.json`.
- No competing `STATUS.md` and no `RELEASE.md` existed in the task folder at
  claim time (first-STATUS-write-wins honored).

## Transition log

- CLAIMED: this commit (STATUS.md only), pushed to origin.
