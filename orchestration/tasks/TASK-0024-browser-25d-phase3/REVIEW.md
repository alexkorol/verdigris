---
task: TASK-0024
verdict: REVISE
reviewed_commits:
  - 6f45c2e
---

## What was reviewed

The knob-by-knob report (excellent provenance — including the archaeology
showing the original port matched the reference and later commits diluted
it), the committed `capture.mjs` harness, and the full capture set
side-by-side (before/after ARPG midday, open-field edges, night,
reference).

## What is correct

- Process quality is the best of any browser task so far: scripted,
  reproducible captures including the open-field horizon shots the 0023
  review demanded; every knob change carries old→new and a reference
  citation; day-length owner comment respected.
- Night reading with emitter lights (after-night) works.

## Problems

1. **The midday frame got materially DARKER and less readable than
   before.** Compare `before-arpg.jpg` vs `after-arpg.jpg` at the same
   scene/time: the after loses luminance across the playfield and the
   0.45 vignette visibly crushes the corners. Root cause: the reference's
   ABSOLUTE grade values were authored over its pastel-bright art;
   applying them verbatim over Verdigris's darker tile albedo
   double-darkens. Conformance means the reference's RELATIVE grade
   shape, normalized to Verdigris's art — the owner's complaint was low
   contrast and mud; the fix cannot be gloom.

## Required corrections (revision 1)

1. Renormalize the ambient keyframes so the midday window (t≈0.0–0.35)
   multiplies at or near neutral over Verdigris art — the after-arpg
   midday capture must have average luminance ≥ the before capture's
   (state both numbers in REPORT.md; a 10-line pixel-average script over
   the two JPEGs is fine). Keep the reference's relative day/night CURVE
   (deeper nights are good) anchored to that neutral midday.
2. Vignette: cap `VIGNETTE_EDGE_ALPHA` at ≤0.30 over current art, or
   scale it with scene luminance. Corners must not read as crushed at
   midday.
3. Re-capture the same set with the same harness; keep everything else
   from this revision (mist, clouds, night emitters) unless the
   renormalization interacts.

## Architectural effect

None until green. The capture harness should be reused by Phase 4/5.
