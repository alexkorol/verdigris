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
