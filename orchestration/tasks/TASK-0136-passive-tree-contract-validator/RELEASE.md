---
task: TASK-0136
released_claims: [6ea36f5a, 7b24e5d3, 7026892e]
released_at: 2026-08-22 01:35 -07:00
reason: duplicate-dispatch collision in ox-pc-c worktree
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
