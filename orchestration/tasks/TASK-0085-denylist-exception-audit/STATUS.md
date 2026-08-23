# TASK-0085 claim

- task: TASK-0085
- title: Live denylist-exception evidence packet
- state: REVIEW_REQUESTED
- lane: ox-pc-bb
- model: openrouter/stealth/ox-alpha
- base_commit: d2423873c577d299b3b39c56024d1d840993c72b
- branch: worker/verdigris/pc/ox-pc-bb
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-bb`
- routed HEAD at claim: `f9ff44dbe286565c9f6b721e01ef4ce79f4a435c` (SPEC
  `base_commit d2423873` verified ancestor of HEAD via
  `git merge-base --is-ancestor`)
- started-at: 2026-08-23 session wall-clock; exact claim commit clock is this
  commit's author/committer time
- resource capsule honored: read-only evidence gathering; no ports bound or
  probed; port 6500 never touched
- task family: EVIDENCE / INDEPENDENT packet (no disposition chosen here;
  owner-only decisions stay out of scope)

## Routing provenance

- `START_HERE_ox-pc-bb.md` launch packet at this worktree routes lane
  ox-pc-bb to TASK-0085 from routed HEAD `f9ff44db` (immutable task base
  `d2423873`). SPEC read from
  `orchestration/tasks/TASK-0085-denylist-exception-audit/SPEC.md` and matches
  the launch packet verbatim.
- Preflight verified per AGENTS.md: clean tree, branch
  `worker/verdigris/pc/ox-pc-bb`, HEAD `f9ff44db`, fetch --prune run,
  upstream in sync (0/0 vs tracked ref).
- Claim-collision check: no STATUS.md or REPORT.md for TASK-0085 on any ref
  (`git log --all` on both paths empty) immediately before this claim.
- Push discipline: this commit and all later commits go to origin
  `worker/verdigris/pc/ox-pc-bb` only. The local branch's convenience tracking
  ref points at the program branch `origin/codex/native-reconstitution`; every
  push in this task uses the explicit destination
  `HEAD:refs/heads/worker/verdigris/pc/ox-pc-bb`. No program-branch push, no
  merge, no rebase of other lanes, no force-push.

## Transition log

- CLAIMED: commit `224a0b7c` (STATUS.md only), pushed to origin
  worker/verdigris/pc/ox-pc-bb.
- EVIDENCE COMPLETE: FINDINGS.md written (occurrence/contract/visibility tables
  + three-disposition evidence for `legacyRelicId` and `bronze-dagger`, plus
  live gate-mechanics findings); all five SPEC acceptance commands run literally
  with transcripts and exit codes in REPORT.md (all exit 0; supplemental checker
  self-test exit 1 / scan exit 0 documented). Worker writes confined to this
  task folder; no forbidden path touched; no ports bound or probed (6500
  untouched); no disposition chosen, nothing renamed, denylist unchanged.
- REVIEW_REQUESTED: REPORT.md carries literal acceptance transcripts; frozen
  review head `576d325d41bcd8b5522779d23bbf21fb0bbb7c11` (FINDINGS + REPORT +
  this STATUS flip), pushed to `origin/worker/verdigris/pc/ox-pc-bb`. No merge,
  no force-push, no program-branch push.
