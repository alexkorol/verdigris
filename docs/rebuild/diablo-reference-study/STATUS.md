# Diablo reference study

- coordinator: codex
- worker: /root, with d1_architecture, d2_architecture, d2_local_tables, verdigris_combat_gap
- state: IMPLEMENTED (Diablo study); IN_PROGRESS (owner-directed pixel world continuation)
- authorization: owner request, 2026-09-08, to reverse engineer the installed D2R as a Verdigris development reference and use subagents
- base: 2b5da07b1
- branch: codex/diablo-reference-study-20260908
- isolated checkout: C:/Users/Alex/Documents/ChatGPT/verdigris-diablo-study
- scope: read-only Diablo reference inspection; actual offline D2R play; independent native cadence and contact-presentation corrections; regression tests; source/evidence record
- original installation: no patches or archive writes; new offline Barbarian verdigris created and played in a 1920x1080 window
- existing user checkouts: preserved

This is an owner-directed study and implementation milestone, not a claim on an existing orchestration task. No architect-owned specifications or product documents are edited.

The owner extended the work to pixel-art world assets, actual Pixel Respecter
conversion, reference-led image generation, and research into firsthand Image
2.5 game/animation workflows. That continuation is active. The first raster
integration milestone is documented in `native/client/assets/raster/README.md`;
it does not claim final animation or visual acceptance.

The next verified milestone adds consistent hero idles, quieter ground and
physical exit stairs, measured equipment attachment, a stopped-motion fix,
accurate capture colors, and color-correct scenario surfaces. All 64 native
scenarios pass (18.7 ms fullscreen; 8.6 ms dense effects), alongside browser
32/32 and the asset/equipment checks. The retained September 9 asset review
includes the viewed live window and actual motion trace. Full animation and
scene composition remain active work; this milestone does not close the goal.

The owner's research correction is now reflected in a source-first recipe
selection table: choose an actual published prompt and record the adaptation
before generation. Original posts and critical comments were rechecked, with
Kiki's separate combat recipe, additional 36/99-frame creator reports and a
complete pose-sheet-to-H3 prompt recorded separately from sprite evidence.
No verified seamless-terrain or
complete adult isometric-walk recipe was found. The browser harness rerun is
32/32; current native motion edits and six new strike poses remain pending
production integration and are not covered by the prior native pass.

The next research audit directly reopened Kiki's complete original prompt,
viewed the combat sheet, and checked a reader's posted GIF attempt and the
palette/engine critique. The guide now preserves the chosen recipe's level of
detail rather than preferring short prompts universally. Flixly's actual
reference arrangement is explicit: previous frame first as edit target,
opening frame additionally as identity anchor. New generation was paused
while these source-backed corrections were applied; existing implementation
and candidate WIP remain preserved and unaccepted where noted.

The firsthand feedback audit also records a game-room developer's improved
edit stability and persistent surface microtexture, with the original comment
and evidence limits. The guide now explicitly checks quiet walls/ground at
gameplay scale. This research checkpoint makes no new gameplay acceptance claim.

September 10 implementation checkpoint: 118 registered pixel assets now include
four hero walk directions (SE 4, SW 8, NW 8, NE 8), three six-pose strike directions,
and a compact contact spark. Measured equipment, actual-target silhouette tints,
damage-label placement, tree dressing and textured raster orb composition are
integrated. Exact prompts and published-recipe adaptations accompany the art.
Root and workers inspected native sprites, equipment composites, production
contact/motion captures and the final live 3440x1440 window.

Final native build/all 64 scenarios pass (18.7ms fullscreen, 9.4ms dense), native
suites pass, browser final 32/32 passes, importer 7/7 and 54-pose equipment checks
pass. The initial SW walk capture failure was corrected by a clear corridor
and full 15 ms painting without changing collision. One initial browser final-
death timeout did not recur in the focused or final full run; bounded failure
diagnostics were added, and the intermittent failure remains unexplained.
Retained evidence: `native/client/assets/raster/reviews/2026-09-10/README.md`.

The visual goal remains active. NW foot registration, clip identity/palette
changes, NE attacks, enemy weapon continuity/motion, quieter ground, paths and
HUD coherence are unfinished. Current NE/raider candidates remain unaccepted
outside runtime. Original D2 walking and A1/A2 attack references remain external
and private; no original D2 pixels are added to runtime or this commit.
