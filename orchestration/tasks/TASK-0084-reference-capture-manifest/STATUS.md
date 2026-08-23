# TASK-0084 claim

- task: TASK-0084
- title: Reference-capture integrity manifest
- state: CLAIMED
- lane: ox-pc-bb
- model: openrouter/stealth/ox-alpha
- base_commit: d2423873c577d299b3b39c56024d1d840993c72b
- branch: worker/verdigris/pc/ox-pc-bb
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-bb`
- routed HEAD at claim: `d20305e46300003a2bda9b9f4d16a8520c2b86bf` (SPEC
  `base_commit d2423873` verified ancestor of HEAD via
  `git merge-base --is-ancestor`)
- resource capsule honored: read-only image evidence; no binary dependencies;
  port 6500 never touched
- packet: MECHANICAL · topology: INDEPENDENT

## Routing provenance

- `START_HERE_ox-pc-bb.md` launch packet at this worktree routes lane
  ox-pc-bb to TASK-0084 from routed HEAD `d20305e4` (immutable task base
  `d2423873`). SPEC read from
  `orchestration/tasks/TASK-0084-reference-capture-manifest/SPEC.md` and
  matches the launch packet verbatim.
- Preflight verified per AGENTS.md: clean tree, branch
  `worker/verdigris/pc/ox-pc-bb`, HEAD `d20305e4`, fetch --prune run, local
  HEAD identical to `origin/worker/verdigris/pc/ox-pc-bb`.
- Claim-collision check: no STATUS.md or REPORT.md for TASK-0084 on any ref
  (`git log --all` on both paths empty) immediately before this claim.
- Push discipline: this commit and all later commits go to origin
  `worker/verdigris/pc/ox-pc-bb` only, via explicit destination
  `HEAD:refs/heads/worker/verdigris/pc/ox-pc-bb`. The local branch's
  convenience tracking ref points at the program branch
  `origin/codex/native-reconstitution`; no program-branch push, no merge,
  no rebase of other lanes, no force-push.

## Transition log

- CLAIMED: this commit (STATUS.md only), pushed to origin
  worker/verdigris/pc/ox-pc-bb.
