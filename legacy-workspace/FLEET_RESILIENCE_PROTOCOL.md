# Verdigris fleet resilience protocol

This protocol makes the fleet tolerant of worker, harness, model, and
supervisor failure. It complements `FLEET_HANDOFF.md` and the canonical
control-plane contracts in `Z:\Code\orchestration`.

## Core rule

There may be one **active** execution coordinator, but there must never be one
irreplaceable coordinator process. Authority and progress live in durable state,
not in a chat session's memory.

The durable sources are:

- the Verdigris Git history and isolated worker branches/worktrees;
- the project's `.orch` append-only event ledger;
- versioned claims, task packets, decisions, reviews, and evidence;
- the current integration lease and coordinator lease;
- the generated human-readable status projection.

If a claim, decision, result, or dependency change exists only in chat, it did
not happen.

## Roles

### Deterministic sentinel

An OS-scheduled, non-LLM watcher runs every 15 minutes. It detects missing
heartbeats, stale claims, dark lanes, path collisions, stale bases, review
pressure, and insufficient runnable runway. It records state changes and alerts
the owner. It does not implement code or make product decisions.

### Active execution coordinator

Exactly one coordinator holds a renewable coordinator lease. It releases tasks,
integrates evidence, replenishes the queue, and prepares succession state. Its
lease is short enough to recover overnight and long enough to tolerate a slow
model response.

Recommended defaults for this surge:

- coordinator heartbeat: every 10 minutes or after every material transition;
- coordinator lease: 30 minutes;
- worker heartbeat: every 15 minutes or after every commit/evidence event;
- stale worker warning: 25 minutes;
- automatic worker reclaim eligibility: 40 minutes without valid progress;
- retry limit for one task/approach: 2 attempts before mandatory split or
  alternative implementation;
- minimum ready queue: 10 claimable tasks across at least 3 collision-free
  lanes;
- reserve queue: at least 30 detailed successor tasks.

### Standby coordinator

Scheduled Codex is a standby auditor, not a simultaneous leader. At each wake:

1. Read the durable ledger and active coordinator lease.
2. If the lease and heartbeat are healthy, audit and leave corrective tasks.
3. If the lease expired, verify that no newer durable heartbeat exists.
4. Publish a takeover acknowledgement against the exact integrated head.
5. Become the active coordinator only after that acknowledgement is durable.
6. Reconstruct the queue from tasks, dependencies, claims, commits, reviews,
   and evidence; never rely on the failed coordinator's chat transcript.

When the original coordinator returns, it becomes a worker/auditor until a new
explicit handoff occurs. It must not resume leadership from stale memory.

## Worker task contract

Every released task must contain:

- stable task id and parent outcome;
- exact assigned harness/lane and resource capsule;
- owned paths and prohibited shared hotspots;
- base/integrated head observed when released;
- dependencies and release predicate;
- concrete player-visible result;
- build, test, playtest, and visual evidence requirements;
- heartbeat promise and lease expiry;
- bounded retry count;
- fallback task that is safe in the same lane;
- successor-generation rule;
- result state: accepted, revise, rejected, blocked, superseded, or abandoned.

A worker is capacity only after a durable claim. A visible window, running
process, or conversational acknowledgement is not proof of activation.

## Automatic failure handling

| Failure | Required reaction |
|---|---|
| Worker never claims | Mark activation failed; release its packet to another capable lane. |
| Worker heartbeat becomes stale | Preserve its branch/worktree, revoke write authority after lease expiry, and reissue from the last durable commit. |
| Worker returns late | Treat output as a candidate against its recorded base; never let it overwrite the replacement's work. |
| Same approach fails twice | Split the task, change approach/model, or release a prerequisite. Do not blindly retry. |
| Harness/provider unavailable | Route compatible tasks to another harness; release low-collision fallback work immediately. |
| Reviewer stalls | Promote a different independent reviewer after lease expiry. |
| Coordinator stalls | Standby publishes takeover acknowledgement and reconstructs from durable state. |
| Integrator stalls | Transfer the separate integration lease against an exact verified head; never run two integrators. |
| Queue falls below minimum | Backlog factory expands the highest-impact unblocked epics and validates paths/dependencies before release. |
| Build breaks | Freeze risky integration, open repair tasks, and keep unaffected lanes producing isolated candidates. |
| Owner decision blocks one branch | Continue fallback tasks and other branches; batch the decision for morning. |

## Queue rules

The ready queue must not be one linear chain. Maintain work in several
independent domains:

- renderer/integration;
- actors/combat feedback;
- UI/Framekit/orbs/inventory;
- town/zones/gates/instances;
- progression/items/House/Chronicles;
- playtest/visual comparison/release repair.

No single failed dependency may empty the queue. Each critical task should have
at least one fallback that produces reusable evidence, content, assets, tests,
or a lower-risk vertical slice without touching the same hotspot.

Queue health is measured as runnable time by compatible lanes, not raw task
count. Reject filler audits created only to satisfy a numeric threshold.

## Integration safety

Coordinator authority and integration authority are separate leases. Exactly
one integrator may modify the integrated program branch. Before each integration
operation it verifies the exact expected head and rejects stale candidates.
Workers never force-push, overwrite another worktree, or merge themselves into
the program branch.

Late results are normal. Review them as candidates, rebase or replay only when
safe, and record rejection when their value has been superseded.

## Minimum durable checkpoint

After every accepted integration, and at least every 30 minutes, record:

- integrated head and dirty/ahead/behind state;
- active coordinator and integration leases;
- live, stale, and reclaimable lanes;
- claimed, ready, blocked, review, accepted, and rejected tasks;
- playtest state and evidence locations;
- queue runway by compatible lane;
- next automatic releases;
- unresolved owner gates;
- exact recovery action if the active coordinator disappears now.

## Tonight's bootstrap gates

The fleet is not resilient until all are true:

1. Verdigris has an initialized `.orch` ledger.
2. Every harness receives a stable lane identity and task/heartbeat contract.
3. The deterministic Windows sentinel is installed and verified.
4. A coordinator lease and separate integration lease are durably recorded.
5. At least 10 tasks are ready across 3 independent lanes, with 30 validated
   reserve successors.
6. Scheduled Codex knows how to audit and take over from durable state.
7. A simulated dead-worker test proves task reclaim without deleting its work.
8. A simulated expired-coordinator test proves standby takeover without dual
   integration.

Until these gates pass, call the run a supervised experiment rather than an
autonomous overnight fleet.
