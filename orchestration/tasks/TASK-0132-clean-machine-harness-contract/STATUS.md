# TASK-0132 claim

- task: TASK-0132
- state: CLAIMED
- coordinator: ox-pc-g
- worker: ox-pc-g (isolated PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0132-clean-machine-harness-contract-ox-pc-g`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-g`
- route/base SHA: `b3599c80122d09cd0685ae96830990cc5bada5cf` (routed HEAD; immutable SPEC base `cab50d62cb121ab6a88fa513257e645447226959` verified ancestor of HEAD)
- started-at: 2026-08-21 22:52 -07:00 session wall-clock; exact claim commit clock is this commit's author/committer time
- ports: 6740-6759 reserved for this lane (loopback only; port 6500 untouched)
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: ARCHITECTURE / INDEPENDENT packet

## Experimental-unit configuration provenance

- endpoint: local OpenCode CLI session in `Z:\Code\.worktrees\verdigris\ox-pc-g`
- provider: `openrouter` (harness-visible model id `openrouter/stealth/ox-alpha`)
- model: `stealth/ox-alpha`
- variant/reasoning: not observed in this session; omitted rather than guessed
- harness: OpenCode CLI 1.18.21
- agent persona: ox-alpha

## Routing provenance

- `START_HERE_OX_PC_G.md` launch packet at this worktree routes ox-pc-g to
  TASK-0132 from routed HEAD `b3599c80` (immutable base `cab50d62`) per the
  RUN_STATUS.md effective READY table row "TASK-0132 clean-machine harness
  contract".
- Preflight verified: clean state, branch
  `codex/TASK-0132-clean-machine-harness-contract-ox-pc-g`, HEAD `b3599c80`,
  upstream in sync (0/0), SPEC base an ancestor of HEAD.
- Fresh fetch performed immediately before claim: no competing STATUS.md, no
  RELEASE.md in the task folder, and origin's
  `codex/TASK-0132-clean-machine-harness-contract-ox-pc-g` sits at the same
  route HEAD with no claim commit.

## Transition log

- CLAIMED: this commit (STATUS.md only), pushed to origin.
