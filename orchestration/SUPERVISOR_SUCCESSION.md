# Supervisor succession and temporary retirement

The primary desktop Verdigris supervisor is the current PC Codex architect task
while the owner keeps it active. Supervisor capacity is not assumed permanent;
the owner may temporarily retire or replace it when Codex credit pressure
requires. The supervisor cannot read the owner's account quota reliably, so an
owner warning or switch directive is the authoritative budget trigger.

Leader-token austerity is active under `LEADER_POLICY.md`. Continuous
reconciliation transfers to the portable `orchd` controller after its vertical
slice is accepted. Before then, any emergency monitor is short, read-only, and
at most hourly; the leader does not remain in a 15-minute reasoning loop.

## Retirement states

- `PRIMARY_ACTIVE`: normal architect/orchestrator loop.
- `RETIREMENT_WARNED`: owner signals likely credit-driven switch; avoid opening
  optional long investigations, finish the current atomic coordination action,
  and prepare the handoff ledger.
- `HANDOFF_READY`: all coordination changes are committed/pushed as authorized,
  current evidence is freshly fetched, and the ledger below is complete.
- `SUCCESSOR_ACKNOWLEDGED`: replacement proves authority read, exact checkout,
  program/remote tips, current claims/reviews/alerts, and automation ownership.
- `TEMP_RETIRED`: predecessor stops only after acknowledgement, unless the owner
  explicitly orders an immediate stop.

## Required handoff ledger

Record in `RUN_STATUS.md` or a dated handoff packet:

1. canonical repo/worktree paths, remotes, branches, local and remote SHAs, and
   dirty/divergence status;
2. active/parked/requested/claimed/active workers with task, base, branch,
   ports, last durable evidence, and alert deadlines;
3. reviews, revisions, integrations, CI, owner gates, incidents, and open
   activation/queue/host-sync alerts;
4. autonomous runway hours and confidence, trailing accepted throughput,
   READY/AUTO_RELEASE/reserve/graph counts and composition;
5. exact next sweep, critical path, automatic promotions, and owner actions;
6. automation names, cadence, destination, notification caveats, and the owner
   or successor action required to transfer them;
7. standalone `alexkorol/orchestration` PC/Mac sync proof and bootstrap state.

## Successor preflight

The successor reads `AGENTS.md`, `ONBOARDING-SOL-ORCHESTRATOR.md`, PROTOCOL,
ORCHESTRATION, ACCEPTANCE, DECISIONS, INCIDENTS, RUN_STATUS,
BACKLOG_FACTORY, CONTENT_SCALE_MATRIX, and this file; fetches before acting;
reconciles repository evidence; posts a durable acknowledgement; then resumes
scan/review/integrate/release/decompose/restock. It does not restart completed
work or implement worker tasks.

A failed handoff or missing acknowledgement is an owner-notification event. It
does not authorize the outgoing supervisor to burn remaining credits by taking
over implementation.

## Verified live PC checkpoint — 2026-08-22 05:41 PDT

State: `PRIMARY_ACTIVE`; this is a succession checkpoint, not a retirement
request.

- Program checkout: `Z:\Code\Games\delaford\delaford_game`, branch
  `codex/native-reconstitution`, clean local/remote program head
  `657ab514827398e0aca734a8ff11a4e0454cbe2f` before this checkpoint refresh
  commit. `origin/master` is not the worker base;
  workers use the exact routed program heads recorded in `RUN_STATUS.md`.
- Standalone broadcast checkout:
  `Z:\Code\.worktrees\orchestration\pc-overnight-game-wave`, clean branch
  `codex/pc-overnight-game-wave`, local/remote head `ff49efc`; observed
  standalone `origin/main` is `59a70b6`. PC has no shared-main authority.
- Active implementation/revision evidence:
  - TASK-0145 head `78dcac60` and TASK-0149 revision head `a88d307d` are
    independently ACCEPTED and integrated at `2df5eac5` and through
    `8677f021`; combined program binaries passed all native suites, all eight
    client scenarios, and the real-window lifecycle selftest;
  - ox-pc-l, ports 6840-6859, TASK-0146 revision head `086ac07b` independently
    ACCEPTED and integrated as the four-commit program chain through `5324f13e`;
    combined program gates passed and pushed head is `08e8776e`;
  - ox-pc-p, ports 6920-6939, TASK-0147 claim `3ee9f928`; first process exited
    with owned-path edits preserved after its real GDI capture exposed broken
    scenery geometry. The supervisor independently rejected the worker's fresh
    05:40 stale probes; the actually regenerated 05:41 motif set is coherent.
    Its 06:00 native capture then used a stale 04:54 executable predating the
    05:39 header, so the exact session is resumed at PID 9048 and must rebuild,
    prove executable freshness, and recapture before any handoff;
  - ox-pc-q, ports 6940-6959, TASK-0148 claim `815a359b` released and clean
    preserved after its recovery proposed `mutate/select` for the first Scion,
    bypassing the required accepted `create/set-out` chain. It is exhausted,
    not capacity;
  - ox-pc-r, ports 6960-6979, TASK-0148 fresh r5 route from release head
    `c1acd4ec`, independent claim `837a412f` pushed inside the activation SLA,
    clean pre-write exit resumed in the exact session at PID 7324. Its start
    contract freezes the accepted first-Scion chain and bans `dev:*`,
    `player:chronicles:mutate`, and direct state shortcuts.
- Preserved/non-capacity: ox-pc-d/e/g/m are dirty P0 quarantines after exhausting
  one recovery; ox-pc-n is a clean activation failure after its launch and one
  recovery both stopped before claim; a/b/c/h are preserved historical or
  exhausted routes as detailed in `RUN_STATUS.md`; ox-pc-o is clean but
  exhausted after its claim plus one stopped recovery. Never infer capacity
  from their old logs or pushed claim heads.
- Queue/factory: board sentinel healthy, 27 effective READY, zero REVISE, 17
  sequenced DRAFT successors, zero owned-path collisions; deterministic factory
  verified at 2,000 nodes / 500 packets. Runway remains honestly `UNKNOWN`.
- Local supervision: human dashboard `http://127.0.0.1:4737/` health 200,
  Node PID 12400; visible PowerShell monitor PID 18912. The monitor's alert set
  is restricted to current completed/active routes `i/j/k/l/p/r`, so preserved
  historical/quarantined lanes cannot produce false toasts; an unattended
  REVISE route now raises a distinct P1 alert. Same-task automation
  `verdigris-surge-supervisor` is ACTIVE every five minutes and derives lanes
  dynamically from current routing/config. Chat-spawning cron
  `pc-fleet-emergency-monitor` remains PAUSED.
- Shared native session tests use the 6580-6599 capsule across worktrees. One
  combined run overlapped ox-pc-l's test process and failed different timing
  checks; after its listener exited, the exact integrated binaries passed
  core/networking/camera/session, all eight client scenarios, and lifecycle.
  Do not classify a concurrent-capsule failure without listener/parent-process
  evidence; route a deterministic isolation hardening packet.
- A stale local TASK-0141 cherry-pick tool chain was found trying to write the
  program worktree during TASK-0146 integration. App thread inventory proved
  no second live coordinator existed and TASK-0141 was already integrated;
  the exact orphan process tree was terminated before it altered the index.
  Never permit two coordinator commands to share this checkout.
- Exact next sweep: verify p/r PIDs, logs, git changes, and pushed heads;
  freeze any clean REVIEW_REQUESTED head; run every literal SPEC gate independently and
  visually inspect TASK-0147 captures; reject any TASK-0148 journey containing
  `dev:*` setup or direct state mutation; integrate only ACCEPTED commits; rerun
  combined native gates and the owner launch after each product integration.
- Owner action required now: none. On credit-driven replacement, the successor
  must fetch both repositories, reproduce this checkpoint against live evidence,
  acknowledge the current alerts/reviews/automations, and only then assume the
  loop.
