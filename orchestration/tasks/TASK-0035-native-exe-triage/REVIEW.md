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

---

# Revision 2 (2026-08-17 ~01:35) — verdict remains REVISE

The claimed revision commit `809de7bb` STILL banks
`trophies stored: 0 | items stored: 0` with exit 0 — architect-verified
by building THAT exact SHA in an isolated worktree. Correction 1 was not
implemented; STATUS requested review anyway. This is the second
false-green in this task.

## Corrections restated — the acceptance is this literal transcript

Run, from the worker worktree at the revision commit:

```
powershell -File native/build.ps1 -RunTests -RunClient
```

The final lines MUST read exactly:

```
native legacy denylist: PASS
verdigris core tests: PASS
Verdigris native client shell
House: House Verdigris | trophies stored: 1 | items stored: 1
```

and `--headless` MUST exit NON-ZERO whenever the banked counts are not
1/1 (prove by temporarily breaking a constant, showing the non-zero
exit, then restoring). Paste both transcripts in REPORT.md. The
coordinator's validator must run THIS command, not an approximation.
Fix mechanically: walk the headless demo's command script to the new
constants (spawn distance, per-tick step, pickup radius, extraction pad)
until the loop genuinely completes.

---

# Final verdict: ACCEPTED (revision 3 verified, 2026-08-17 ~02:30)

Architect-verified at branch tip `225078d1` in an isolated worktree:
literal transcript ends `trophies stored: 1 | items stored: 1`, the
failure path exits non-zero, and the driven play pass shows the clean
default view (bars + skill strip only, zero debug text, honest grove
scale). Both prior false-greens are documented in the report. Integration
approved.
