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
| `ox-pc-f` | Windows implementation | 6720-6739 |
| `ox-pc-g` | Windows implementation | 6740-6759 |
| `ox-pc-h` | Windows implementation | 6760-6779 |
| `ox-pc-i` | Windows implementation | 6780-6799 |
| `ox-pc-j` | Windows implementation | 6800-6819 |
| `ox-pc-k` | Windows implementation | 6820-6839 |
| `ox-pc-l` | Windows implementation | 6840-6859 |
| `ox-pc-m` | Windows implementation | 6860-6879 |
| `ox-pc-n` | Windows implementation | 6880-6899 |
| `ox-pc-o` | Windows implementation | 6900-6919 |
| `ox-pc-p` | Windows implementation | 6920-6939 |
| `ox-pc-q` | Windows implementation | 6940-6959 |

Provisioned worktree:
`Z:\Code\.worktrees\verdigris\ox-pc-a`; worker branch:
`codex/TASK-0128-accepted-throughput-normalization-ox-pc-a`; routed task base:
`31d215793f0f799fd365f080ca326ea04e83706c`. The local ignored
`START_HERE_OX_PC_A.md` is the complete launch packet.

The owner has now registered eight isolated OpenRouter CLI-addressable
Verdigris lanes (`ox-pc-b` through `ox-pc-i`). `ox-pc-a` remains a separate
desktop-session experimental unit on its originally observed provider alias.
The new `ox-pc-b` and `ox-pc-c` worktrees are not the historical stopped tabs:
those old tabs shared one project, made no claim/write, and remain non-incidents.
Every registered lane below has its own Z: worktree and branch.

The overnight owner-visible wave may use up to eight simultaneous Ox Alpha
workers. Lanes `ox-pc-j` and `ox-pc-k` are clean expansion lanes added for
launch resilience and clean-build convergence; they do not make the preserved
or quarantined a/b/c/h worktrees available again.

| Lane | Worktree | Branch | Route |
|---|---|---|---|
| ox-pc-a | `Z:\Code\.worktrees\verdigris\ox-pc-a` | `codex/TASK-0128-accepted-throughput-normalization-ox-pc-a` | TASK-0128 |
| ox-pc-b | `Z:\Code\.worktrees\verdigris\ox-pc-b` | `codex/TASK-0080-board-sentinel-ox-pc-b` | TASK-0080 |
| ox-pc-c | `Z:\Code\.worktrees\verdigris\ox-pc-c` | `codex/TASK-0112-passive-tree-authority-schema-ox-pc-c` | TASK-0112 |
| ox-pc-d | `Z:\Code\.worktrees\verdigris\ox-pc-d` | `codex/TASK-0129-server-lifecycle-soak-ox-pc-d` | TASK-0129 |
| ox-pc-e | `Z:\Code\.worktrees\verdigris\ox-pc-e` | `codex/TASK-0130-gate-c-decision-envelope-ox-pc-e` | TASK-0130 |
| ox-pc-f | `Z:\Code\.worktrees\verdigris\ox-pc-f` | `codex/TASK-0131-release-proof-manifest-ox-pc-f` | TASK-0131 |
| ox-pc-g | `Z:\Code\.worktrees\verdigris\ox-pc-g` | `codex/TASK-0132-clean-machine-harness-contract-ox-pc-g` | TASK-0132 |
| ox-pc-h | `Z:\Code\.worktrees\verdigris\ox-pc-h` | `codex/TASK-0133-save-migration-rollback-contract-ox-pc-h` | TASK-0133 |
| ox-pc-i | `Z:\Code\.worktrees\verdigris\ox-pc-i` | `codex/TASK-0134-distribution-signing-boundary-ox-pc-i` | TASK-0134 |
| ox-pc-j | `Z:\Code\.worktrees\verdigris\ox-pc-j` | `codex/TASK-0149-native-owner-launch-resilience-ox-pc-j` | TASK-0149 |
| ox-pc-k | `Z:\Code\.worktrees\verdigris\ox-pc-k` | `codex/TASK-0150-native-clean-build-convergence-ox-pc-k` | TASK-0150 |
| ox-pc-l | `Z:\Code\.worktrees\verdigris\ox-pc-l` | `codex/TASK-0146-native-first-expedition-encounter-wave-ox-pc-l-r2` | TASK-0146 replacement |
| ox-pc-m | `Z:\Code\.worktrees\verdigris\ox-pc-m` | `codex/TASK-0147-procedural-native-visual-polish-wave-ox-pc-m-r2` | TASK-0147 replacement |
| ox-pc-n | `Z:\Code\.worktrees\verdigris\ox-pc-n` | `codex/TASK-0148-native-chronicles-reconnect-runtime-ox-pc-n-r2` | TASK-0148 replacement |
| ox-pc-o | `Z:\Code\.worktrees\verdigris\ox-pc-o` | `codex/TASK-0148-native-chronicles-reconnect-runtime-ox-pc-o-r3` | TASK-0148 replacement |
| ox-pc-p | `Z:\Code\.worktrees\verdigris\ox-pc-p` | `codex/TASK-0147-procedural-native-visual-polish-wave-ox-pc-p-r3` | TASK-0147 independent replacement |
| ox-pc-q | `Z:\Code\.worktrees\verdigris\ox-pc-q` | `codex/TASK-0148-native-chronicles-reconnect-runtime-ox-pc-q-r4` | TASK-0148 independent replacement |

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
TASK-0080 is in a narrow architect-directed revision on its existing lane.
TASK-0083 was rejected before claim because its immutable base is absent from
origin; replacement TASK-0129 is released to ox-pc-d at exact base
`88d9210bf2b27ab3a776974be23f54c6174c3fff`. The expansion routes c/e/f/g/h/i
are task-folder-only, pairwise disjoint architecture packets promoted at base
`cab50d62cb121ab6a88fa513257e645447226959`. Each lane works only one task at a time. Re-fetch before claiming because
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
