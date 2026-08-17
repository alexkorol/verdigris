---
task: TASK-0032
verdict: ACCEPTED
reviewed_commits:
  - 749cc4a6
  - ac3a7216
---

## What was reviewed

The server diff against the 0031 audit's delta table, the report's gate
evidence (757/757 unit, playtest 31/31, focused suites 46), and the one
gate the worker could not run — `smoke:browser` — which I ran myself on
their worktree once the owner's session freed port 6500: **1/1 PASS,
port released**. The worker's refusal to kill the owner's live session
was correct discipline.

## What is correct — every audit delta row closed

- Hard death now transfers ALL equipped + carried items and standalone
  trophies to recovery pools (D-106); socketed trophies stay embedded
  (documented rule); successor starts empty.
- SQLite/JSON store convergence via migration + UUID dedup; old-save
  compatibility tested; long-term single-authority correctly left as the
  owner-facing question per spec.
- Instance retirement with membership checks and one-time requeue —
  mirrors the native TASK-0025 semantics, including underfoot pickup.
- The D-109 hole closed: failed disconnect saves queue a complete atomic
  snapshot BEFORE removal — no silent loss path remains.

## Required corrections

None.

## Architectural effect

The browser game now obeys the owner's death/disconnect rulings —
D-106/D-109 are live at both layers (native core and shippable game).
This also raises the parity bar for wave N5 deliberately. Integration
approved.
