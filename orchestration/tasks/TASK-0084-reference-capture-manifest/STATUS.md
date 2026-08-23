# TASK-0084 claim

- task: TASK-0084
- title: Reference-capture integrity manifest
- state: REVIEW_REQUESTED
- reviewed_commit: c0f1f34ee6ae91424bb23d26eae8bec04e04f552
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

- CLAIMED: commit `30a96556` (STATUS.md only), pushed to origin
  worker/verdigris/pc/ox-pc-bb.
- EVIDENCE COMPLETE: `reference-manifest.mjs` (dependency-free verifier:
  five-scene naming matrix, PNG IHDR / JPEG SOF header dimension parsing
  without pixel decode, SHA-256 hashing, render-list JSON structural
  validation, missing/duplicate/zero-byte/wrong-resolution/malformed/
  unmanifested failure modes, read-only verify by default, `--write`
  regenerates only `reference-manifest.json`) plus the frozen 30-entry
  manifest committed as `2481abb0`. A first negative run exposed a
  cwd-relative `--manifest` resolution bug with an unclean crash; fixed in
  `91630905`. All four SPEC acceptance commands then re-run literally against
  final code (all exit 0) and an authentic one-bad-hash negative on a
  disposable copied manifest exited 1 and was removed without touching any
  evidence. Transcripts in REPORT.md.
- REVIEW_REQUESTED: REPORT.md carries literal acceptance transcripts; frozen
  review head `c0f1f34ee6ae91424bb23d26eae8bec04e04f552` (verifier + manifest
  at `91630905` + REPORT + this STATUS flip), pushed to origin
  worker/verdigris/pc/ox-pc-bb. No merge, no force-push, no program-branch
  push.
