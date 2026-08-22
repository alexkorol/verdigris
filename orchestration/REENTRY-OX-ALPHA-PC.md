# PC Ox Alpha coordinator re-entry

Shared entry point for the owner-opened PC OpenCode/Ox Alpha lane. This file
is intentionally short; the canonical rules remain in the linked documents.

## Identity and resources

| Name | Primary lane | Ports |
|---|---|---|
| `ox-pc-a` | Windows implementation | 6620-6639 |
| `ox-pc-b` | Windows implementation | 6640-6659 |
| `ox-pc-c` | Windows implementation | 6660-6679 |
| `ox-pc-d` | Windows implementation | 6680-6699 |
| `ox-pc-e` | Windows implementation | 6700-6719 |

Provisioned worktree:
`Z:\Code\.worktrees\verdigris\ox-pc-a`; worker branch:
`codex/TASK-0128-accepted-throughput-normalization-ox-pc-a`; routed task base:
`31d215793f0f799fd365f080ca326ea04e83706c`. The local ignored
`START_HERE_OX_PC_A.md` is the complete launch packet.

The owner has now registered five isolated CLI-addressable Verdigris lanes.
The new `ox-pc-b` and `ox-pc-c` worktrees are not the historical stopped tabs:
those old tabs shared one project, made no claim/write, and remain non-incidents.
Every registered lane below has its own Z: worktree and branch.

| Lane | Worktree | Branch | Route |
|---|---|---|---|
| ox-pc-a | `Z:\Code\.worktrees\verdigris\ox-pc-a` | `codex/TASK-0128-accepted-throughput-normalization-ox-pc-a` | TASK-0128 |
| ox-pc-b | `Z:\Code\.worktrees\verdigris\ox-pc-b` | `codex/TASK-0080-board-sentinel-ox-pc-b` | TASK-0080 |
| ox-pc-c | `Z:\Code\.worktrees\verdigris\ox-pc-c` | `codex/TASK-0086-gate-c-contract-audit-ox-pc-c` | TASK-0086 |
| ox-pc-d | `Z:\Code\.worktrees\verdigris\ox-pc-d` | `codex/TASK-0083-server-lifecycle-soak-ox-pc-d` | TASK-0083 |
| ox-pc-e | `Z:\Code\.worktrees\verdigris\ox-pc-e` | `codex/TASK-0120-release-verification-gap-audit-ox-pc-e` | TASK-0120 |

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

TASK-0081, TASK-0086, TASK-0105, and TASK-0120 are ACCEPTED and integrated.
TASK-0080 is in a narrow architect-directed revision on its existing lane;
ox-pc-d is released to TASK-0083 at program base
`1455c536caeb02b88a7355e75a6efe71f0358667`. Each lane works only one task at a time. Re-fetch before claiming because
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
