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

## Verified live PC checkpoint — 2026-08-22 04:03 PDT

State: `PRIMARY_ACTIVE`; this is a succession checkpoint, not a retirement
request.

- Program checkout: `Z:\Code\Games\delaford\delaford_game`, branch
  `codex/native-reconstitution`, clean local/remote head `141082f6` before this
  checkpoint refresh commit. `origin/master` is not the worker base; workers use the exact
  routed program heads recorded in `RUN_STATUS.md`.
- Standalone broadcast checkout:
  `Z:\Code\.worktrees\orchestration\pc-overnight-game-wave`, clean branch
  `codex/pc-overnight-game-wave`, local/remote head `835de51`; observed
  standalone `origin/main` is `59a70b6`. PC has no shared-main authority.
- Active implementation/revision evidence:
  - ox-pc-i, ports 6780-6799, TASK-0145 claim `226e5149`, dirty/fresh process
    PID 1560;
  - ox-pc-j, ports 6800-6819, TASK-0149 frozen head `96f4ccbd`, REVISE for the
    post-spawn readiness-failure orphan path, dirty/fresh revision PID 12216;
  - ox-pc-l, ports 6840-6859, TASK-0146 claim `78a0c4a0`, clean/fresh process
    PID 4108;
  - ox-pc-m, ports 6860-6879, TASK-0147 claim `7d092a74`; original process
    stopped dirty and its one permitted exact-session recovery is fresh at PID
    21864. A second stop before handoff is quarantine/release;
  - ox-pc-o, ports 6900-6919, TASK-0148 claim `71a73de8`; original process
    stopped clean after claim and its one permitted exact-session recovery is
    fresh at PID 25572. A second stop before handoff is quarantine/release.
- Preserved/non-capacity: ox-pc-d/e/g are dirty P0 quarantines after exhausting
  one recovery; ox-pc-n is a clean activation failure after its launch and one
  recovery both stopped before claim; a/b/c/h are preserved historical or
  exhausted routes as detailed in `RUN_STATUS.md`. Never infer capacity from
  their old logs or pushed claim heads.
- Queue/factory: board sentinel healthy, 27 effective READY, one REVISE, 17
  sequenced DRAFT successors, zero owned-path collisions; deterministic factory
  verified at 2,000 nodes / 500 packets. Runway remains honestly `UNKNOWN`.
- Local supervision: human dashboard `http://127.0.0.1:4737/` health 200,
  Node PID 2028; visible PowerShell monitor PID 6824. Same-task automation
  `verdigris-surge-supervisor` is ACTIVE every five minutes and derives lanes
  dynamically from current routing/config. Chat-spawning cron
  `pc-fleet-emergency-monitor` remains PAUSED.
- Exact next sweep: verify all five PIDs/log ages and pushed heads; freeze any
  clean `REVIEW_REQUESTED` head; run every literal SPEC gate independently;
  integrate only ACCEPTED commits; then run the complete combined native
  build/client scenario suite and real visible owner launch. Visual/encounter/
  Chronicles activity does not count as morning success until integrated and
  freshly launched.
- Owner action required now: none. On credit-driven replacement, the successor
  must fetch both repositories, reproduce this checkpoint against live evidence,
  acknowledge the current alerts/reviews/automations, and only then assume the
  loop.
