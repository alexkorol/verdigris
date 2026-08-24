# Broadcast — nine-hour day wave (PC, 2026-08-24 ~07:15 PDT)

Dispatch for all lanes, all harnesses. This is the current dispatch until a
newer file lands in broadcasts/. The owner is away ~9 hours; everything
below is designed to run without them.

## Standing changes, effective now

1. **orchestration/BUS.md is binding** for every harness. Enrollment,
   push-is-the-lock claims, own-worktree isolation, 45-minute commit
   cadence, QUESTION-and-move-on. Read it before claiming.
2. **A deterministic sentinel now watches this fleet** (Task Scheduler,
   every 30 min, no LLM): stalls, launcher failures, hijacked checkouts,
   and pending owner decisions page the owner directly. Silence is no
   longer invisible; don't rely on it.
3. **D-129 ruled** (see DECISIONS.md): ranged windup rides the projectile
   convention. `monster:telegraph` stays slam-only everywhere.

## Board — claimable READY packets

Fresh scaffolded packets, integrated this morning from the cursor prep
wave (each has an interface header in native/client/, tests, and a
run-tests.ps1 in its task folder):

- TASK-0189 instance-gate-bridge
- TASK-0190 town-runtime-layout
- TASK-0192 native-multizone-runtime
- TASK-0194 skill-tree-layout
- TASK-0196 spell-lattice-integration
- TASK-0201 house-first-investment-integration

Continuing: ox-pc-bh on TASK-0163 (mid-implementation, not claimable).

**Not claimable:** TASK-0108 — ruled D-129, awaiting SPEC rev 3 from the
architect (lands today; both prior heads superseded on the wire question).

## Housekeeping rulings

- Branches worker/verdigris/pc/ox-pc-bf and ox-pc-bg are superseded stale
  duplicates (their tasks integrated under other lanes on 2026-08-23). Do
  not review them; coordinator-of-day may prune after verifying the
  integrate commits (850c07d2, c3f13ec9).
- The ox-pc-hm-1 enrollment commit (99fbb33f) is welcome; its method was
  not — it switched the architect checkout. BUS.md rule 3 now forbids
  this. Hermes lanes: re-enroll per BUS.md from your own worktree.

## Coordinator-of-day

The seat is open. Claim it per BUS.md (pushed RUN_STATUS.md heartbeat
naming yourself). The seat integrates accepted REVIEW_REQUESTED heads,
keeps the READY floor visible, and prunes superseded branches. Any
enrolled lane except an author may validate reviews regardless of who
holds the seat. Never force-push; never touch master.

## Definition of a good nine hours

Every READY packet above either DONE-and-integrated, REVIEW_REQUESTED
with a frozen head, or parked on a specific pushed QUESTION. Zero silent
lanes. The owner should come home to a board that explains itself from
RUN_STATUS.md and this file alone.
