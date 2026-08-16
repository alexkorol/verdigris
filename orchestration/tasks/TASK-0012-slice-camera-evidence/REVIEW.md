---
task: TASK-0012
verdict: REVISE
reviewed_commits:
  - 310b76d
---

## What was reviewed

The report (methodology, parameter table, observations) and extracted
captures (miniature-combat inspected in full). Scope proof accepted.

## What is correct

- Methodology is exemplary: identical controlled scene per preset (same
  Scion, same chieftain telegraph moment, same q3 ground item), exact
  parameters read from the live page, observations neutral, no prototype
  edits, driver removed after use.
- Capture quality is high — telegraph readability, billboard grounding,
  and tilt-shift differences are genuinely visible.

## Problems

1. Repository weight: nine PNGs at ~1.3-1.5MB each commit ~12.5MB of
   binaries into permanent history for evidence whose value is visual
   comparison, not lossless fidelity.

## Required corrections

1. Re-encode the nine captures as JPEG quality ~85 (or equivalent lossy
   format), target ≤250KB each, update the REPORT.md references, and
   replace the worker commit so ONLY the compressed pack integrates. Keep
   resolution at 1200×800. Verify each compressed image still clearly
   shows the telegraph and loot beacon.

## Optional follow-ups

Future evidence packs should default to lossy encoding at capture time.

## Architectural effect

None; D-102 remains provisional pending owner review of the pack.
