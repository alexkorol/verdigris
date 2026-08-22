# OpenCode Ox Alpha CLI sub-fleet

This is the reusable PC launch contract for independently routed Verdigris
workers. It supplements `REENTRY-OX-ALPHA-PC.md`; the route table and each task
SPEC remain authoritative.

## Dispatcher prompt

> You are the PC Ox Alpha sub-fleet dispatcher for Verdigris. Operate only as a
> control-plane supervisor; never implement a worker task. Read AGENTS.md,
> orchestration/ORCHESTRATION.md, LEADER_POLICY.md, PROTOCOL.md, RUN_STATUS.md,
> REENTRY-OX-ALPHA-PC.md, STANDING-LOOP.md, and every selected task SPEC. For
> each routed worker, prove that its task is READY and independent, its owned
> paths do not collide with another live claim, and its base is valid. Give it
> a unique Z:\Code\.worktrees\verdigris\<lane> worktree, exact worker branch,
> port capsule, ignored START_HERE packet, JSONL log, and an explicit
> openrouter/stealth/ox-alpha OpenCode CLI launch. Never share a worktree,
> branch, task, ports, or log. Do not use --auto. Do not count a process,
> window, worktree, or log stream as capacity. Require a committed and pushed
> protocol-valid STATUS claim within 10 minutes; alert P1
> PROVISIONED_UNCLAIMED on the first observation after that boundary, escalate
> to P0 ACTIVATION_FAILED at 30 minutes or two sweeps, and classify any
> root/repository/branch/base/task/identity mismatch as P0 MISROUTED
> immediately. After claim, require fresh branch or heartbeat evidence before
> ACTIVE. Restart once only for a proved centralized launch/auth/provider
> failure. Workers push only their worker branches and never merge. Escalate to
> the leader only for Tier C, conflicting evidence, protected surfaces, owner
> authority, or repeated unexplained gate failure.

## Worker launch shape

OpenCode CLI 1.18.21 is installed at
`C:\Users\Alex\AppData\Roaming\npm\node_modules\opencode-ai\bin\opencode.exe`.
New shells also resolve `opencode` through the user PATH. Authentication comes
from `OPENROUTER_API_KEY`; never copy the value into prompts, files, command
history, or logs.

Each route gets a local ignored `START_HERE_<LANE>.md` containing the exact
root, branch, route head, immutable task base, task, ports, model, allowed
paths, gates, evidence, and STOP conditions. Launch it headlessly with the
equivalent of:

```powershell
& opencode run `
  --dir 'Z:\Code\.worktrees\verdigris\<lane>' `
  --model 'openrouter/stealth/ox-alpha' `
  --variant max `
  --agent build `
  --format json `
  --title 'Verdigris-<lane>' `
  'Read START_HERE_<LANE>.md completely and execute it now.'
```

The supervisor normally starts one hidden process per lane and redirects stdout
to `Z:\Code\.fleet\logs\<lane>.jsonl` and stderr to
`Z:\Code\.fleet\logs\<lane>.stderr.log`. A zero-byte stderr log is healthy.
JSONL `sessionID` and OpenRouter metadata prove the harness/model handshake;
Git and task STATUS remain the authority for claim, activity, and completion.

## Capacity states

1. `PROVISIONED_PARKED`: resources exist; no launch requested; not capacity.
2. `LAUNCH_REQUESTED`: record time, PID/session when known, root, branch, base,
   task, ports, launch packet, and logs; still not capacity.
3. `CLAIMED`: the exact branch contains a committed and pushed valid task
   STATUS claim; first point counted as live capacity.
4. `ACTIVE`: fresh post-claim branch or heartbeat evidence proves execution.
5. `REVIEW_REQUESTED`: freeze the reviewed head and route through the task's
   Tier A/B/C acceptance path.

The old stopped GUI tabs formerly labelled `ox-pc-b` and `ox-pc-c` shared a
different OpenCode project and never claimed or wrote work. They remain
historical non-capacity and non-incidents. Current lanes with those names are
the distinct isolated CLI worktrees recorded in `RUN_STATUS.md`.
