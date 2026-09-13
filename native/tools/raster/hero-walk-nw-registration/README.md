# NW walk ground registration

Baseline: `f3318b059`, reviewed production capture
`.ci-artifacts/raster-directional-verified/raster-motion-nw-019.png`.
Its trace selects `hero_walk6_nw` at 1200 ms.

The actor renderer places the complete 80 by 96 canvas at
`top = base.y - height`. A native pixel edge `y` therefore maps to
`base.y + (y - 96) * height / 96`. The ground ellipse uses the same base.
There is no visible-bounds scaling on this path. Walk6 ended at native edge
90, leaving six empty pixel rows above that base; the captured separation
was asset registration, rather than a different renderer scale.

The sheet has two rows of poses with different vertical packing. A global
downward shift sufficient for the second row would clip the first row.
Planted/crossing phases 1–3 reach source sole edges 479–480; return contact 7
reaches second-cell sole edge 454, nearest reconstruction grid edge 456.
The fixed row roots are now local `[192,480]` and `[192,456]`, or absolute
source Y 480 and 968. Their 488-pixel authored spacing differs from the
512-pixel cell spacing. These are shared row translations, not individual
pose fitting.

Actual Pixel Respecter reconstruction at the existing six-pixel grid pitch
and shared 32-color palette produces exactly the old RGBA pixels translated:

| Frames | Native Y translation | Old sole gaps | New sole gaps |
|---|---:|---|---|
| 0–3 | +1 | 3, 1, 1, 1 | 2, 0, 0, 0 |
| 4–7 | +5 | 7, 7, 6, 5 | 2, 2, 1, 0 |

All eight images retain their 80 by 96 canvas, every visible pixel, binary
alpha and original colors. No clipping, resizing, per-pose alignment, source
editing or palette reduction beyond the existing importer occurred. The
retained two-pixel sole travel still distinguishes lifted and crossing feet.

Viewed `idle-cycle-idle-before-after-1x.png` and
`idle-cycle-idle-after-contact-3x.png`. The shared ellipse and canvas expose
the corrected contact and the unchanged identity jump: the walk body remains
65–69 pixels tall versus idle 63, with a wider stance and redder tunic.
The paired GIF is an 80 ms per-pose review loop, with 400 ms idle holds.
This is asset-preview verification; new equipped production captures are
the remaining integration check.

Validation: `review_registration.py` checks all eight pixel arrays against
their exact integer translations, including visible-pixel counts and binary
alpha. The seven existing importer tests pass. Runtime hashes match the
viewed candidate hashes in `registration-review.json`. NW hand sockets must
receive the same Y translations; the equipment owner was notified after
the PNGs were frozen. Shared catalog and freeze files were left to root.

`baseline/` retains the previous PNGs, manifest and report. `candidate/`
retains the actual reconstructed result. The active manifest and provenance
preserve the earlier production review as a baseline observation and mark
the new registration as awaiting production review.

## Production review, September 10

Root subsequently viewed the corrected production capture at
`.ci-artifacts/ground-hud-motion/raster-motion-nw-019.png`. The feet now sit
at the ellipse. All four input-driven hero cycles passed first movement
paint, all-phase and settle checks, and the full NW equipment probe passed.
`registration-review.json` records the capture, trace and equipment-review
hashes. The correction is accepted for this iteration; the identity and
body-size limitations above remain. This review update changed no PNG bytes.
