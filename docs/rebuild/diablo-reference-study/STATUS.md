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

Final native build/all 64 scenarios pass (18.7 ms fullscreen, 9.4 ms dense), native
suites pass, browser final 32/32 passes, importer 7/7 and 54-pose equipment checks
pass. The initial SW walk capture failure was corrected by a clear corridor
and full 15 ms painting without changing collision. One initial browser final-
death timeout did not recur in the focused or final full run; bounded failure
diagnostics were added, and the intermittent failure remains unexplained.
Retained evidence: `native/client/assets/raster/reviews/2026-09-10/README.md`.

At that checkpoint, NW registration, NE attacks, enemy weapon/motion continuity,
ground paths and HUD coherence remained unfinished. Original D2 walking and
A1/A2 attack references remain external and private; none of their pixels are
added to runtime or committed evidence.

September 10 ground/HUD continuation: 127 active assets now include quiet earth
and eight axe-preserving SW raider walk poses. NW walking feet and equipment
receive exact +1/+5px registration corrections; walking begins on the first
actual movement update. World-aligned paths follow the existing town/village
landmarks, using a bounded ground cache. Common HUD chrome, measured label
placement, equipment row spacing and XP placement pass production captures at
3440x1440, 1366x768 and 960x600, including both panes with muted audio.

Final supported build/all 66 scenarios pass: 23.258 ms stationary and 24.715 ms
moving average (36.620 ms peak), twenty 3440x1440 frames per condition with the
unchanged 40 ms average gates. Native suites, browser 32/32, importer 7/7 and
equipment/ground/HUD checks pass. Root inspected the final live window and
small dual-pane capture, then closed the local game cleanly. Evidence and
initial failures are retained in
`native/client/assets/raster/reviews/2026-09-10-ground-hud/README.md`.

Raider motion verification is explicitly scripted through production drawing;
native enemy AI remains stationary. The NE contact candidate improves limb
continuity, but its next follow-through switched arms and its matte cleanup
damaged foreground. It is rejected outside runtime. The real published
reference recipe and both local failure stages are recorded in PROMPTING.md.
The visual goal remains active: NE actions, native enemy locomotion, cross-clip
identity, other monster directions, terrain repetition and further composition
are unfinished. No push or merge.

## 2026-09-10 — Reference-led attacks and readable warnings

The catalog has 139 active pixel assets: NE hero strikes and SW raider strikes
add six poses each. Actual Pixel Respecter palette snapping pins NE colors to
the accepted idle, whose ready pose stays pixel-identical. Measured equipment
sockets pass 60 poses×5 weapons×3 scales. Twelve dropped-item families now select
appropriate existing art; compatible storehut art replaces an existing village
dwelling and Mara's existing town stall without changing objects or collision.

Real elite Sweep events now drive preparation during the warning, phase 3 on
the first confirmed damage frame, and follow-through/recovery afterward. Root
caught and removed an opaque warning fill that hid both actors. Supported final
build/all 68 native scenarios, native suites, browser 32/32, importer 10/10 and
equipment/sampling pass. Fullscreen static average 22.550 ms; moving average
23.993 ms and peak 33.926 ms across 20 frames, unchanged 40 ms gates. Root viewed the
live 3440×1440 window and closed it with exit 0/no orphan.

[Captures, final logs, failures and precise review scope](../../../native/client/assets/raster/reviews/2026-09-10-actions/README.md)
are retained. NE body width/reset, bow/staff-specific actions, other monster
directions and locomotion, pixel effects/deaths and terrain repetition remain.
The elite contact proof does not establish ordinary native melee animation,
whose existing damage event lacks attacker identity. This milestone preserves
the active visual objective. Committed locally; no push or merge.

September10 feedback/body continuation:

Nine new pixel sprites replace geometric dust/swing feedback and add a four-pose SW
raider collapse. The catalog now contains 148 assets across 27 manifests and 52
sources. Each new output exactly matches its reviewed Pixel Respecter candidate;
all 139 previous runtime PNGs remain unchanged.

Ordinary native melee now identifies its attacker before damage. Identified
remote melee/thrust/sweep does the same; ranged and unknown attacks retain their
existing damage feedback. Live ticks age old effects before ingesting contact,
so the first paint keeps the contact pose. Session deaths use a prior actor
snapshot when polling/painting has already removed the living enemy.

Falls last 160 ticks (8 seconds), with four poses over 8 ticks and a final
20-tick fade. Settled bodies draw beneath standing actors. Up to 32 bodies share
the 128-effect cap; transient bursts preserve them. Death never creates a live
actor or a second reward, and scene/loss transitions clear retained bodies.

Final supported build/all 69 scenarios pass. Native core/networking/session/
presentation/audio suites, browser 32/32, importer 10/10 and equipment 60x5x3 plus
sampling pass. Twenty 3440x1440 stationary frames average 25.426 ms; moving
frames average 25.467 ms with 35.561 ms peak. Both average gates remain
40 ms. Root viewed the final supported live window and closed it with exit0.

The fall clip covers only SW raiders. Other death directions/families retain
dust, and deaths without a known prior snapshot cannot fabricate a body. Slash
trails are generic combat feedback, not measured weapon-specific paths. Native
pursuit, other monster motion, bow/staff actions, cross-clip identity, terrain
repetition and remote wall presentation remain unfinished. WarCry, spawn/loss
effects, semantic boundaries, team rings and shadows still use procedural forms.
The broad pixel-world goal remains active.

Evidence: `native/client/assets/raster/reviews/2026-09-10-feedback/README.md`. Goal remains IN_PROGRESS.
