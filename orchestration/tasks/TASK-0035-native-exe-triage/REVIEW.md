---
task: TASK-0035
verdict: REVISE
reviewed_commits:
  - e562ad1e
  - 04055142
---

## What was reviewed

Gates rerun in the worker worktree. Build/denylist/tests green — but the
headless proof loop now ends `trophies stored: 0 | items stored: 0`
(previously 1 | 1), reproduced twice at the worktree tip, while the
REPORT claims "Headless output — unchanged/passed." The rescaled
spawn/range constants broke the scripted demo (its hardcoded command
counts no longer reach enemy/loot/extraction), and validation only
checked the exit code, not the banked outcome.

## Required corrections (revision 1)

1. Adapt the headless demo script to the new D-114 constants so the
   proof loop completes again: kill → drop → pickup → extract →
   `trophies stored: 1 | items stored: 1`. That line IS the proof the
   game rules still connect end-to-end; treat it as an assertion, not
   decoration (make the headless run FAIL non-zero if the banked counts
   are not 1/1, so this class of miss cannot recur).
2. Correct the REPORT's verification section — a claimed-green check
   that I could falsify in one run damages review trust more than a
   disclosed red. State what was actually run and what the validator
   missed.

## What is correct (keep it all)

The D-114 constants table derivation, the F3 debug gating, the clean
default view, and the honest testbed title all look right in the diff —
the play-gate pass happens on revision 1 once the loop proves itself.

## Architectural effect

None until green.
