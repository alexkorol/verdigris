---
task: TASK-0136
released_claims: [6ea36f5a, 7b24e5d3, 7026892e, ddd00857]
released_at: 2026-08-22 02:16 -07:00
reason: duplicate-dispatch collision on ox-pc-c, then repeated post-claim worker exit on ox-pc-h
---

# TASK-0136 claim release

The ox-pc-c claim is released. Two OpenCode writers mutated the same worktree
and task folder, after which the surviving worker correctly stopped at
`7026892e`. No worker process is now active, but three untracked implementation
paths remain in `Z:\Code\.worktrees\verdigris\ox-pc-c`.

Preserve that branch and dirty worktree for forensic comparison. Do not clean,
reset, resume, or count ox-pc-c as capacity. A replacement worker may claim the
immutable READY spec from a fresh current program base and an independent clean
worktree; first post-RELEASE STATUS write wins.

## Replacement lane release

The clean ox-pc-h replacement validly claimed at `ddd00857` and preserved a
partially implemented validator plus synthetic fixtures. Its first process
exited without a handoff; the supervisor applied the single allowed exact-
session recovery. That recovery also exited with uncommitted work and no
REVIEW_REQUESTED head. This is now a P0 post-claim activation failure, not a
dark active lane. Preserve `Z:\Code\.worktrees\verdigris\ox-pc-h` exactly as
evidence; do not clean, reset, resume a second time, or count it as capacity.

TASK-0136 returns to READY but is not re-routed during the owner-visible
overnight wave. A future replacement must use a fresh independent worktree and
must not copy the quarantined implementation blindly.
