# TASK-0134 claim

- task: TASK-0134
- state: REVIEW_REQUESTED
- coordinator: ox-pc-i
- worker: ox-pc-i (isolated PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0134-distribution-signing-boundary-ox-pc-i`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-i`
- route/base SHA: `b3599c80122d09cd0685ae96830990cc5bada5cf` (HEAD; immutable
  task base `cab50d62cb121ab6a88fa513257e645447226959` verified ancestor of HEAD)
- started-at: 2026-08-21 23:04 -07:00 session wall-clock; exact claim commit clock is this commit's author/committer time
- ports: 6780-6799 reserved for this lane (unused by this contract packet; port 6500 untouched)
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: ARCHITECTURE / INDEPENDENT packet

## Experimental-unit configuration provenance

- endpoint: local OpenCode CLI session in `Z:\Code\.worktrees\verdigris\ox-pc-i`
- provider: `openrouter` (harness-visible model id `openrouter/stealth/ox-alpha`)
- model: `stealth/ox-alpha`
- variant/reasoning: not observed in this session; omitted rather than guessed
- harness: OpenCode CLI 1.18.21
- agent persona: ox-alpha

## Routing provenance

- `START_HERE_OX_PC_I.md` launch packet at this worktree routes ox-pc-i to
  TASK-0134 from routed HEAD `b3599c80` with immutable base `cab50d62`
  (`merge-base --is-ancestor cab50d62 HEAD` → true).
- Preflight per AGENTS.md run at claim time: clean tree, origin remote
  `https://github.com/alexkorol/verdigris`, fetch --prune done, branch in
  sync (0/0) with `origin/codex/TASK-0134-distribution-signing-boundary-ox-pc-i`.
- Fresh fetch performed immediately before claim: no competing STATUS.md,
  no RELEASE.md in the task folder, and no other origin branch matching
  TASK-0134 existed. First-STATUS-write-wins is exercised by this file.
- Note: this is a resume after an isolation denial during preflight; no
  files outside this worktree were probed on resume, per launch constraints.

## Scope discipline

- owned_paths only: `orchestration/tasks/TASK-0134-distribution-signing-boundary/**`
- forbidden paths honored: no `native/**`, `server/**`, `src/**`,
  `playtest/**` changes; no credentials, accounts, publication, or any
  claim that signing/notarization occurred.

## Transition log

- CLAIMED: commit `5b952c08` (STATUS.md only), pushed to origin worker
  branch.
- RESUMED: 2026-08-21 (same day, later session). Checked `RELEASE.md` first
  — none present, claim stands. AGENTS.md preflight re-run: tree dirty only
  with this task's three untracked partial-work files; branch in sync `0/0`
  after `git fetch --prune origin`; base `cab50d62` re-verified ancestor of
  HEAD (`merge-base --is-ancestor`, exit 0). Dependencies now installed in
  this worktree (`node_modules/yorkie` present), so the pre-commit hook runs
  normally on subsequent commits.
- IMPLEMENTED: preserved the three interrupted-session files verbatim
  (`distribution-boundary.json`, `fixtures/negative-cases.json`,
  `captures/gate-3-rg-distribution-surface.txt`) and added the missing
  deliverables `VALIDATION.md` + `REPORT.md`. All five SPEC acceptance
  commands pass (exit 0), run twice; both machine gates proven fail-closed
  via mutated-copy negative controls (see VALIDATION.md).
- REVIEW_REQUESTED: this commit; implementation carried by commit
  `8195c6a8`. Awaiting architect review per PROTOCOL.

## Review-requested evidence summary

- Gates: boundary key check PASS; negative-cases case check PASS; rg
  distribution-surface scan exit 0 (589 lines final, captured verbatim in
  `captures/gate-3-rg-distribution-surface.txt`); `git diff --check`
  clean; base..HEAD diff confined to routed orchestration files.
- Negative controls: gate snippets exit 1 against temp copies missing
  `rollback` / `ROLLBACK_UNPROVEN` respectively.
- Scope: writes confined to `orchestration/tasks/TASK-0134-distribution-
  signing-boundary/**`; no credentials acquired, no external service
  contacted, no signing/notarization/publication performed or claimed.
