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

## Verified live PC checkpoint — 2026-08-22 04:44 PDT

State: `PRIMARY_ACTIVE`; this is a succession checkpoint, not a retirement
request.

- Program checkout: `Z:\Code\Games\delaford\delaford_game`, branch
  `codex/native-reconstitution`, clean local/remote coordination head
  `491cff3c` before this checkpoint refresh commit; accepted product content is
  integrated through `a81642ae`. `origin/master` is not the worker base;
  workers use the exact routed program heads recorded in `RUN_STATUS.md`.
- Standalone broadcast checkout:
  `Z:\Code\.worktrees\orchestration\pc-overnight-game-wave`, clean branch
  `codex/pc-overnight-game-wave`, local/remote head `8b55376`; observed
  standalone `origin/main` is `59a70b6`. PC has no shared-main authority.
- Active implementation/revision evidence:
  - TASK-0145 head `78dcac60` and TASK-0149 revision head `a88d307d` are
    independently ACCEPTED and integrated at `2df5eac5` and through
    `8677f021`; combined program binaries passed all native suites, all eight
    client scenarios, and the real-window lifecycle selftest;
  - ox-pc-l, ports 6840-6859, TASK-0146 frozen head `a72b6317` reviewed REVISE
    because it never placed multiple living threats on the floor; exact branch
    revision process PID 8108 is active on the simultaneous elite/flanker fix;
  - ox-pc-p, ports 6920-6939, TASK-0147 claim `3ee9f928`, process PID 20868;
  - ox-pc-q, ports 6940-6959, TASK-0148 claim `815a359b`, recovery PID 6104.
    The first process was stopped clean before writes after proposing forbidden
    `dev:teleport` setup; this lane has consumed its one recovery and must prove
    the complete journey using normal accepted envelopes only.
- Preserved/non-capacity: ox-pc-d/e/g/m are dirty P0 quarantines after exhausting
  one recovery; ox-pc-n is a clean activation failure after its launch and one
  recovery both stopped before claim; a/b/c/h are preserved historical or
  exhausted routes as detailed in `RUN_STATUS.md`; ox-pc-o is clean but
  exhausted after its claim plus one stopped recovery. Never infer capacity
  from their old logs or pushed claim heads.
- Queue/factory: board sentinel healthy, 27 effective READY, one REVISE, 17
  sequenced DRAFT successors, zero owned-path collisions; deterministic factory
  verified at 2,000 nodes / 500 packets. Runway remains honestly `UNKNOWN`.
- Local supervision: human dashboard `http://127.0.0.1:4737/` health 200,
  Node PID 12400; visible PowerShell monitor PID 7772. The monitor's alert set
  is restricted to current completed/active routes `i/j/k/l/p/q`, so preserved
  historical/quarantined lanes cannot produce false toasts. Same-task automation
  `verdigris-surge-supervisor` is ACTIVE every five minutes and derives lanes
  dynamically from current routing/config. Chat-spawning cron
  `pc-fleet-emergency-monitor` remains PAUSED.
- Shared native session tests use the 6580-6599 capsule across worktrees. One
  combined run overlapped ox-pc-l's test process and failed different timing
  checks; after its listener exited, the exact integrated binaries passed
  core/networking/camera/session, all eight client scenarios, and lifecycle.
  Do not classify a concurrent-capsule failure without listener/parent-process
  evidence; route a deterministic isolation hardening packet.
- Exact next sweep: verify l/p/q PIDs, logs, git changes, and pushed heads;
  require l to close the numbered simultaneous-pack revision; freeze any clean
  REVIEW_REQUESTED head; run every literal SPEC gate independently and
  visually inspect TASK-0147 captures; reject any TASK-0148 journey containing
  `dev:*` setup or direct state mutation; integrate only ACCEPTED commits; rerun
  combined native gates and the owner launch after each product integration.
- Owner action required now: none. On credit-driven replacement, the successor
  must fetch both repositories, reproduce this checkpoint against live evidence,
  acknowledge the current alerts/reviews/automations, and only then assume the
  loop.
