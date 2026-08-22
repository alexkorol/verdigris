# PC Ox Alpha coordinator re-entry

Shared entry point for the owner-opened PC OpenCode/Ox Alpha lane. This file
is intentionally short; the canonical rules remain in the linked documents.

## Identity and resources

| Name | Primary lane | Ports |
|---|---|---|
| `ox-pc-a` | Windows implementation | 6620-6639 |

Provisioned worktree:
`Z:\Code\.worktrees\verdigris\ox-pc-a`; worker branch:
`codex/TASK-0128-accepted-throughput-normalization-ox-pc-a`; routed task base:
`31d215793f0f799fd365f080ca326ea04e83706c`. The local ignored
`START_HERE_OX_PC_A.md` is the complete launch packet.

Only `ox-pc-a` is currently registered as Verdigris capacity. The stopped
`ox-pc-b` and `ox-pc-c` OpenCode tabs shared one project, made no claim or
write, and are not lanes, stalls, incidents, or capacity. Do not route work to
them unless the owner explicitly registers new independent Verdigris workers.

Use only the provisioned dedicated worktree. The architect checkout at
`Z:\Code\Games\delaford\delaford_game` is forbidden. Never reset, clean, or
reuse a dirty legacy clone. Register the actual clone path in the first
committed task `STATUS.md`.

## Read and run

1. Prove the exact worktree/branch/base from `START_HERE_OX_PC_A.md`, then
   fetch `origin` and reconcile current program routing without mutating the
   architect branch.
2. Read `orchestration/PROTOCOL.md`, `ORCHESTRATION.md`, `RUN_STATUS.md`,
   `ACCEPTANCE.md`, `STANDING-LOOP.md`, and `MODEL_SCORECARD.md`.
3. Follow `STANDING-LOOP.md` with NAME set to the lane name above and the
   assigned port capsule. `RUN_STATUS.md` is the only current routing source.
4. Claim by committing `STATUS.md` with the exact endpoint, provider, model
   alias, harness/version, configuration provenance, machine, task family,
   base SHA, worker branch, clone path, and start time. First commit wins.
5. Work only in `owned_paths`; paste literal acceptance transcripts and exit
   codes into `REPORT.md`; push only the worker branch; request review; loop.

TASK-0081 is ACCEPTED and integrated. The current route is
`ox-pc-a` -> TASK-0128. Route only one Verdigris task at a time until the owner
explicitly adds workers. Re-fetch before claiming because
a newer `RUN_STATUS.md`, claim, REVIEW, RELEASE, or REVISE always wins.

## Activation acknowledgement

Provisioned is not active. When the owner or supervisor requests launch,
`RUN_STATUS.md` records the request time and the expected worktree, branch,
base, task, and packet. The lane becomes live only when the expected worker
branch contains a protocol-valid committed claim. It becomes `ACTIVE` only
after fresh post-claim execution evidence appears.

No valid claim within 10 minutes of a launch request is a P1
`PROVISIONED_UNCLAIMED` alert. No claim after 30 minutes or two sweeps is P0
`ACTIVATION_FAILED`. Any resumed tab or process attached to a different
project, worktree, branch, base, task, or identity is immediate P0
`MISROUTED`. The architect notifies the owner with expected versus observed
evidence; it does not claim or implement the worker task.

## Hard stops

Never touch port 6500, non-loopback binds, peer task files, peer processes,
program/master integration, harness assertions, or owner-only product choices.
Stop on an owned-path collision, unsafe checkout, unprovable driver
precondition, source gap outside scope, or contradictory authority. Record the
blocker without inventing a fix.
