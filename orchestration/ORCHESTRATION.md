# Orchestration constitution

Short, curated, architect-owned. The full protocol mechanics stay in
PROTOCOL.md; decisions in DECISIONS.md; current truth in RUN_STATUS.md.
Adopted 2026-08-18 from the owner's field-guide integration; local
verified evidence outranks external advice.

## Prime directive

Optimize owner-visible, accepted progress per unit of wall time and
cost — not agent utilization, tokens, commits, or coordination volume.
The immutable global objective is D-116 (C++ version at web parity or
better) under D-110 (playable-first). No local task outranks these.

## Topology-first dispatch

Label every task before routing: INDEPENDENT / PIPELINED / COUPLED /
EXPLORATORY.

- COUPLED or sequential shared-state work stays with one strong agent
  (usually the architect scaffolds the seam first, per D-120).
- INDEPENDENT bounded work may fan out (disjoint owned_paths).
- PIPELINED work releases only when its prerequisite is ACCEPTED.
- EXPLORATORY fan-out assigns deliberately different hypotheses or
  regions — identical prompts are correlated, not independent.

Spawn gate — all YES before a task goes READY: discrete deliverable?
owner + forbidden scope explicit? interfaces frozen (or exactly one
task owns freezing them)? base commit recorded? resources isolated or
serialized? exact acceptance path stated? review capacity exists?
parallelism actually shortens the critical path?

WIP budget: architect + 3 workers max at current review bandwidth.

## Authority (narrower than capability)

- Coordinators: implement on OWN worker branches in OWN clones; write
  ONLY their task folders' STATUS/REPORT/captures. Never merge to the
  program branch or master; never edit peer evidence, ARCHITECT_STATE,
  DECISIONS, SPECs, REVIEWs, or coordination truth; never kill peer
  processes or touch shared credentials/permissions.
- Architect: specs, reviews, scaffolding (D-120: interfaces, risky
  math, failing tests — pre-wave), integration, merge to master, this
  file. During a wave the architect coordinates; any architect
  implementation beyond scaffolding becomes an explicit tracked task.
- Owner only: seasons, magic, economy, naming, lore, assets, balance
  retunes, irreversible/account actions, GitHub settings.
- Contradictory directives or unexpected competing writes = STOP,
  preserve state, escalate in the task folder. Never defeat a peer.

## Delegation contract (required for READY)

Every SPEC carries: outcome + owner-visible contribution, non-goals,
owned/forbidden paths, base commit, frozen interfaces/invariants,
scaffolding when the packet is MECHANICAL, exact acceptance commands
on the DEFAULT path, required evidence format, resource capsule, and
STOP/escalate conditions. Packets are typed by JOB, not by model ego:
MECHANICAL (exact steps + scaffold), BOUNDED-DESIGN (pinned interface,
local freedom), ARCHITECTURE (architect-only).

## Resource capsules (ports)

- kimi-work: 6510–6529 · deepseek: 6540–6559 · kimi: 9880–9899 ·
  architect: 6560–6579 · cursor (Grok 4.6): 6580–6599 ·
  ox-pc-a: 6620–6639 ·
  owner live server: 6500 (never touch) ·
  playtest default: 6510 (serialized — one full suite at a time per
  machine). All binds 127.0.0.1 (enforced in server/index.js default).

## Validation ladder (acceptance = all rungs that apply)

G0 driver preconditions proven (target actually reached) ·
G1 targeted deterministic check, raw output + exit code ·
G2 negative control where practical ·
G3 subsystem/integration on a base merged with CURRENT tip ·
G4 default owner path + owner-visible artifact (screenshots/playthrough) ·
G5 architect reruns the exact gate personally (testimony ≠ evidence) ·
G6 post-merge revalidation of affected checks.

G5 peer-rerun option (token-scarcity adaptation, 2026-08-20): for
non-critical BROWSER tasks, G5 may be satisfied by a PEER RERUN — a
coordinator other than the implementer reruns the exact acceptance
gates in its own clone (fresh merge with current tip) and commits
literal transcripts to the task folder as REVIEW-PEER-<name>.md. The
architect then reads the diff plus both evidence sets and issues the
verdict, rerunning personally only on conflict or suspicion. Personal
architect rerun stays MANDATORY for: native waves, any lane's first
task (calibration), anything modifying tests/playtest assertions.

Greens are revision- and environment-bound: stale after relevant code,
base, evaluator, or environment changes. PARTIAL is failure unless the
contract defined a partial deliverable. Modified tests are listed and
reviewed — a test the implementer can rewrite is not an oracle.

## Rule lifecycle

Learnings carry status: OBSERVATION → HYPOTHESIS → EXPERIMENT → RULE →
RETIRED. A RULE names its enforcement (hook/script/CI/gate/permission);
prose-only rules are hypotheses. Incidents append to INCIDENTS.md;
calibration lives in MODEL_SCORECARD.md; current truth in
RUN_STATUS.md (rewritten, not appended). ORCHESTRATION-LEARNINGS.md is
frozen source history plus new observations awaiting promotion.

## Enforcement backlog (deterministic controls to add)

1. GitHub branch protection on master (OWNER action — settings).
2. Playtest-port lease note in RUN_STATUS before any full-suite run.
3. CI stale-base check: PR diff vs current tip on rendering/protocol
   surfaces (candidate GitHub Action; task when justified).
4. Capture scripts: hard-fail pattern is already the standard (0038).

## Continuous-loop contract (promoted from INC-011, 2026-08-18)

Every coordinator runs the canonical standing loop in
STANDING-LOOP.md - claim semantics (committed CLAIMED STATUS.md is
the ONLY claim form), notes discipline (NOTES-<name>.md, never task
STATUS files), empty-board backoff (real sleep, doubling to 3600s),
REVISE-first priority. Enforcement: STANDING-LOOP.md is the single
goal source (briefs reference it; per-coordinator goal texts must not
drift), and the architect sweep runs the stuck-loop heuristic (fresh
clone FETCH_HEAD + no active claim + READY tasks on board =>
intervene via spec annotation, never by editing coordinator state).

## Worker activation and liveness contract (D-127, 2026-08-21)

Provisioning is not activation. A worktree, branch, port capsule, launch packet,
or visible application window does not count as fleet capacity. Every routed
lane moves through explicit evidence states:

1. `PROVISIONED_PARKED` -- resources exist, but no launch has been requested;
   not capacity and not an alert.
2. `LAUNCH_REQUESTED` -- the owner or supervisor has asked a worker to start;
   record the timestamp, exact worktree, branch, base, task, and launch packet.
3. `CLAIMED` -- the expected branch contains the protocol-valid committed
   `STATUS.md` claim. This is the first point at which the lane counts as live.
4. `ACTIVE` -- post-claim branch evidence or a fresh task heartbeat proves the
   worker started execution. A claim-only lane remains separately visible.

At every sweep, compare requested activations with repository evidence before
counting capacity. A requested local worker that has not committed a valid
claim within 10 minutes is `P1 PROVISIONED_UNCLAIMED`: notify the owner on the
first observing sweep. If it remains unclaimed for 30 minutes or two sweeps,
escalate to `P0 ACTIVATION_FAILED`. A project/repository, worktree, branch,
base, task, or identity mismatch is `P0 MISROUTED` immediately. Alerts include
expected versus observed values and the smallest owner action that can restore
the lane. They deduplicate by lane + expected task + state and re-notify only
on escalation or material evidence change.

Do not rewrite a stopped or never-launched lane as dark capacity, and do not
open an incident against an unrelated historical tab. Activation failure is a
control-plane alert. The architect may re-provision or re-brief the lane, but
must never claim or implement its task.

## Durable queue runway (D-125, 2026-08-20)

Queue health is measured from the effective READY table in `RUN_STATUS.md`,
not from immutable historical SPEC headers. Integrated, claimed, held,
superseded, dependent, or owned-path-colliding packets do not count. Before a
three-lane restart, hold at least 8 effective READY packets plus 4 sequenced
successors; thereafter keep the greater of 8 or two per live coordinator. A
claim triggers same-sweep restocking. Useful runway outranks filler: when the
floor cannot be met without crossing owner authority, stage lane revival and
report the real deficit.

D-128 supersedes count-only queue sufficiency. Follow `BACKLOG_FACTORY.md`:
measure 72-hour autonomous runway, maintain the 2,000-node terminal graph and
500-packet detailed reserve, release validated successors automatically, and
report domain/composition/owner-blocked coverage. The 24+12 surge target remains
only the emergency absolute floor.

## Supervisor succession

Follow `SUPERVISOR_SUCCESSION.md`. Owner-reported credit pressure is a planned
retirement trigger. The outgoing supervisor publishes exact repo, worker,
claim, review, alert, runway, automation, and next-sweep state; a replacement
acknowledges that evidence before the predecessor retires unless the owner
orders an immediate stop.
