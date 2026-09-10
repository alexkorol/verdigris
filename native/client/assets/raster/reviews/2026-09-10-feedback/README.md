# Pixel contact, dust and retained bodies — September 10, 2026

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

## Evidence and review scope

- `raster-feedback-ordinary-contact.png`: a real fixed game tick lands an
  ordinary enemy hit and paints owned strike phase3, slash, impact and damage.
- `raster-feedback-death-{0,2,4,6,8}.png`: a real lethal melee event removes the
  live raider and advances its four authored collapse poses. The later samples
  advance the presentation clock without dispatching another attack.
- `raster-feedback-settled-under-player.png` and `-expired.png`: the intentional
  overlap fixture puts the living player over the settled body, then verifies
  expiry without added actors, XP or loot. A160-effect burst also tests local
  eviction policy. This fixture does not claim navigation through corpses.
- `raster-feedback-dash-{0,2,4,6}.png`: a real core dash followed by the four
  registered dust phases; fixed pivots and canvas scale remain unchanged.
- `raster-feedback-session-{before-drain,death}.png`: LocalCoreSession updates
  and paints its model before draining the death event. The saved prior actor
  still supplies the correct body position. Separate real-WebSocket tests
  exercise15 incoming hit cases, including ranged/unknown exclusions.
- `raider-strike-contact.png`: the corrected two-half Sweep flash. The initial
  four-copy composition looked like a flower despite passing tests; its actual
  rejected capture is retained as `sweep-flower-rejected.png`.
- `raider-native-1x.png`, `raider-native-3x.png`, `dust-native-3x.png` and
  `slash-native-3x.png` retain inspected reconstruction reviews. The production
  strip crops actual frame pixels without resampling. These ordered images were
  inspected; no claim is made that root watched a smooth GIF playback.

Each of the three new sources used one built-in image generation call with
accepted references. Dust and death are explicit local adaptations of published
single-subject motion recipes, not claimed creator-proven dust/death prompts.
Raider alpha failed as a painted checker and required measured neutral-only
cleanup; dust used real source alpha. Exact prompts, references, source hashes,
conversion settings and observations remain in the promotion dependency record.
The built-in image tool exposes no selectable model variant.

`retained-evidence.json` records original paths and hashes. Original candidate
acceptance records remain historical. Final active metadata was reimported
without changing any PNG. `root-review.json` records final evidence and scope.

The initial probe expected strike age1 instead of the actual confirmed age3+1;
the first session capture fixture omitted billboard initialization. Those test
failures are retained. Review also caught lost session death snapshots, local
body eviction, stale spawn rings on corpses and contact aging before first paint.
The final suite covers the corrected production paths.

## Remaining limits

The fall clip covers only SW raiders. Other death directions/families retain
dust, and deaths without a known prior snapshot cannot fabricate a body. Slash
trails are generic combat feedback, not measured weapon-specific paths. Native
pursuit, other monster motion, bow/staff actions, cross-clip identity, terrain
repetition and remote wall presentation remain unfinished. WarCry, spawn/loss
effects, semantic boundaries, team rings and shadows still use procedural forms.
The broad pixel-world goal remains active.
