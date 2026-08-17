---
task: TASK-0025
verdict: ACCEPTED
reviewed_commits:
  - 63df51f
---

## What was reviewed

The core diff (pickup authority boundary, `retire_instance`, pack-clear
guard, 124 test lines) and an independent `build.ps1 -RunTests` rerun
(green).

## What is correct

- Pickup now requires an active instance AND membership in the active
  instance's ID lists — the authority boundary the PR #4 P1 finding
  demanded; stale vectors can no longer leak loot across the
  extraction/route boundary.
- `retire_instance` semantics are exactly the right product reading:
  ordinary floor leftovers are LOST (extraction risk), while surfaced
  relic candidates re-attach to the next instance's floor via a pending
  pool — preserving D-106's "never destroyed" without creating a
  double-registration or an escape hatch, and documented in-code.
- Route clear (and campaign completion) now fires only when no living
  monster remains; per-kill drops unchanged — the P2 finding closed
  without altering reward cadence.

## Required corrections

None.

## Architectural effect

The expedition boundary is now enforced by construction. Integration
approved.
