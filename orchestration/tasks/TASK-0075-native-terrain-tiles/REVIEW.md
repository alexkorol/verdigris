# TASK-0075 review - REVISE (mechanics accepted, visuals miss)

Architect rerun 2026-08-20 ~10:35 (merged with 0076 + N5 candidate):
all gates green, render-list Floor ops asserted, deterministic capture
identity holds, both modes render, fallback intact - the MECHANICS are
right. Play-pass capture shows the miss: the floor reads as a noisy
high-contrast teal/brown checkerboard with hard grid lines - louder
than the browser reference and arguably less readable than the old
dark floor. Spec required the floor to aid readability, not dominate.

Numbered corrections:
1. SCALE: tile the texture finer (sub-tile the world tile, e.g. 2x2 or
   4x4 texture repeats per world tile, or downscale sampling) so the
   grain reads like the browser floor, not big saturated panels.
2. TONE: mute it - blend each tile toward the scene's dark base
   (e.g. 55-70% black overlay) so actors/FX/loot pop; the floor is
   context, not content.
3. GRID: remove the hard per-tile grid lines; if an edge treatment
   stays, it must be subtler than the telegraph/shadow layers.
4. VARIATION: soften the checkerboard alternation - hash-bias toward
   ONE dominant tile with occasional variant, per theme.
Re-request review with an updated play capture + the same determinism
evidence. Everything else stands.
