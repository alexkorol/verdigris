---
task: TASK-0146
verdict: REVISE
reviewed_head: a72b6317a0a57a31c2e50e91f1bd3844a5283ef8
implementation_head: e0ca05f6fa8e5482a1586967c11200d4185bc2e3
reviewed_at: 2026-08-22 04:37 -07:00
---

# TASK-0146 review — REVISE

The pushed head is clean, scoped to the owned native core/test paths plus the
task STATUS/REPORT, and its deterministic roster, instance-retirement, reward,
and exactly-once phase mechanics are otherwise coherent. It does not yet meet
the owner-visible encounter outcome, however.

## 1. Materialize a real multi-threat pack

At the reviewed head, `materialize_wave()` moves only `pending_wave_.front()`
into `actors_` and clears the timer. The next reserve Warden is not scheduled
until that newly materialized Warden dies. Consequently there is never more
than one living Warden in the first expedition. The player sees three serial
single-target fights, not the required small pack with simultaneously legible
normal/elite composition and spatial separation.

Revise the existing kill-scheduled reinforcement design so the elite and
normal flanker materialize together after the entry Warden's telegraph window.
Preserve their existing deterministic anchors and shared constants. Clearing
either reinforcement must leave `SlayWardens` active while the other remains;
clearing the last living Warden must advance to `ExtractCarriedValue` exactly
once. Do not weaken or reorder any existing client scenario to make this pass.

Add or strengthen focused assertions proving all of the following:

- immediately before the reinforcement deadline, no reserve Warden is alive;
- at the deadline, both reserve Wardens are alive concurrently at their
  deterministic elite/flanker anchors;
- killing one leaves the other alive and does not clear the route or advance
  the phase;
- killing the last one clears/advances exactly once;
- replay equality and death/recovery retirement remain deterministic.

Rerun every literal SPEC acceptance command and publish a revision REPORT with
the exact new implementation head. Preserve the frozen reviewed head for audit.
