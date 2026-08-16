---
task: TASK-0011
verdict: ACCEPTED
reviewed_commits:
  - 8c68aed
---

## What was reviewed

Core diff at `8c68aed` (pending-action state, elite decision bands,
telegraph event, cancellation paths, 261 test lines) and an independent
`build.ps1 -RunTests` rerun (denylist + all tests green).

## What is correct

- Elites choose skills by deterministic bands (Thrust in the cone at
  thrust-only range, Sweep at melee range when funded, else plain melee) —
  no RNG, and the SAME cone predicate the resolver uses, so telegraph and
  strike can never disagree.
- Resolution re-checks resource/cooldown via `resolve_actor_action` — the
  fizzle semantics the spec required, with zero duplicated combat math.
- Cancellation is thorough: dead attacker, dead player mid-loop, and
  player-death clearing all monsters' windups in `handle_death`.
- Non-elite cadence is the untouched original path.
- `AttackTelegraphed` carries actor, action name, and windup ticks —
  everything a client needs to render honest warnings.

## Problems

None blocking.

## Required corrections

None.

## Architectural effect

Actor symmetry is now behavioral: monsters walk the same action pipeline
as players, and telegraphs are simulation truth. A client
telegraph-rendering task is the natural follow-up. Integration approved.
