# Real pursuit and directional pixel walks

This milestone adds six northeast and eight northwest raider walk frames,
converted through the actual external Pixel Respecter workspace API. All 148
previous runtime PNGs remain unchanged. The library has 162 sprites, 29 active
manifests and 55 unique sources. Existing southwest walking has eight frames.
Southeast walking remains unavailable: its initial sheet and two repairs kept
the same anatomical leg forward instead of producing opposite support.

The new native pursuit scenario uses actual fixed-tick movement, collision,
contact and damage. Its isolated direction studies place actors only during
setup; later positions come from the core. The tree-detour study retains the
production tree and proves every travelled segment clears its collider.
The older `raider-motion` scenario remains an explicitly scripted presentation
fixture; it is not evidence of AI movement.

## Observed art and source limits

Root viewed the native idle/walk strips and all fourteen unscaled production
walk crops, plus full-frame walk, contact and tree-detour captures. The poses
have readable leg progression at game scale. Narrower bodies, smaller axe
silhouettes and minor crown/phase differences remain visible. This is
provisional integration, not a claim of perfect cross-clip identity or smooth
video playback. The stored frame traces prove timing and progression; static
images alone do not prove temporal quality.

`walk-production-crops-1x.png` contains unscaled crops from actual game paints.
The accompanying crop method uses the recorded world positions and production
camera transform, without resizing or touching runtime artwork. Its original
path was `.ci-artifacts/pursuit-production-crops-1x.png`; the adoption record
retains that original hash and context.

The source-first methods are documented in [RESEARCH.md](../../RESEARCH.md)
and [PROMPTING.md](../../PROMPTING.md). The borrowed short sheet recipe is a
combat study, not a published validated isometric walk. Our ordered Diablo
motion references were private generation references; they are excluded from
runtime and Git. Built-in imagegen exposed no model or variant selector.

## Integration changes

- Native enemies pursue around shared solid circles. Both player movement and
  pursuit use the same swept collision predicate. Geometry and path search are
  bounded; crowd separation and arbitrary-maze navigation are not implemented.
- Input commands resolve in order within one 50 ms authority tick. Mouse or
  polling bursts cannot accelerate the world. Successful scene transitions
  discard remaining commands for the old scene.
- Walk direction follows actual travel. Warnings, strikes, death poses and
  feedback retain their event-time position/facing across later commands.
  Authored strikes use the common feet pivot without a second procedural lunge.
- Local sessions preserve world coordinates, actor identity and diagonal
  facing. Entry, extraction and re-entry install or retire floor collision.
  Ordinary drops use the defeated actor's event position. Previously recovered
  items without an authored location retain their legacy placement.
- Content keeps entry, extraction and owed spawn anchors clear. The initial
  collision-only Tin2 repair was rejected after its production capture still
  hid the player and stairs behind a tree; that image is retained here.
  The final capture shows the hero and exit after moving that one tree from
  `(-31,87)` to `(-87,-67)`, with the other props unchanged. The visibility
  guard uses actual sprite alpha bounds and renderer scaling only during
  construction; it does not change normal painter order or travel occlusion.

## Validation record

Final validation and source hashes are recorded in `root-review.json`.
Original paths in the evidence manifest describe capture-time sources;
portable evidence is the retained file and its hash. Harness scratch outputs
can be overwritten by later runs without changing this retained record.
The final supported build passes all 71 scenarios. The preceding supported
run passes every native suite; its remaining two failures were the targetless
main.cpp commitment fixture, corrected in the final run. Browser 32/32,
importer 10/10 and equipment/sampling checks also pass. Twenty 3440x1440
stationary frames average 23.283 ms; moving frames average 25.402 ms with a
34.124 ms peak. The average limits stay at 40 ms.

Root launched `play-native.ps1 -Local`, viewed original-resolution live
captures, opened/closed the gear pane through Sky and closed both test
sessions with exit 0 and no orphan game process. Both local launches lost the
idle Scion during tool observation. R/E attempts were too late to establish
successful combat; no completed manual fight or live extraction is claimed.
This exposes an opening-pace/inspection limitation of the hostile local
testbed. The actual fight/loot/extraction proof is the production scenario
pipeline. Sky screenshot capture reported an interface error, so native
captures used the repository's supported capture tool; UI input stayed on Sky.

Initial failures are retained: older fixed-distance fight fixtures walked past
pursuers; one buff fixture queued its cast after a scene-changing entry in the
same batch; the first strike-commitment probe had no target. The corrected fight
fixtures retain real kills, loot, equipment and extraction assertions and use
ordinary commands with unchanged stats.

The broader pixel-world goal remains active. Other monster motion, southeast
raider walking, opening pace, bow/staff actions, cross-clip identity, terrain repetition and
remote wall presentation still need work. This milestone does not close them.
