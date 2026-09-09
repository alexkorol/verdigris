# Diablo reference → Verdigris development

Owner-directed study, 2026-09-08. Baseline Verdigris commit: `2b5da07b1`.

This milestone establishes reproducible access to the owner's actual D2R data,
maps verified D1/D2 implementation ideas to Verdigris, and corrects a native
combat pacing defect. It does not claim that Verdigris now looks or feels like
Diablo. The implementation and test results are recorded in `REPORT.md`.

## What was actually inspected

The supplied installation is:

```text
Z:\Games\Diablo-2-Resurrected\Diablo II Resurrected Infernal Edition
```

Its `.build.info` reports `3.2.92777`, and its build-config key is
`3afa9806f33475daee9b317a21d55d09`. These identify the local reference; they do
not independently certify that every installed file is stock Blizzard content.
Infernal Edition is an official contemporary bundle including Reign of the
Warlock, so the edition name alone is not evidence of a fan mod. See
[Blizzard's expansion overview](https://news.blizzard.com/en-us/article/24243863/rain-annihilation-in-reign-of-the-warlock).

An independently built [CascLib](https://github.com/ladislav-zezula/CascLib)
opened the local archives normally. Four gameplay tables and seven UI layouts
were selected for private study. Extracted files and the third-party parser
remain outside the repository at:

```text
C:\Users\Alex\Documents\ChatGPT\diablo-reference-cache
```

The important discovery was that this installation's archive paths use the
`data:data\...` namespace. A name such as `data\global\excel\skills.txt`
alone does not identify the same file to this parser. Hashes and selected
facts are retained with this development record; no Diablo artwork, audio,
fonts, complete tables, or engine code are imported into Verdigris.

D2R was launched, but Windows Computer Use capture failed twice with
`SetIsBorderRequired failed: No such interface supported (0x80004002)`.
Consequently there are **no firsthand D2R gameplay timing, audio, or screenshot
measurements** in this milestone. Data observations below are not disguised
as play observations.

The [live Verdigris baseline](evidence/verdigris-baseline.png) was captured with
the repository's supported capture tool at 3440×1440 and visually inspected.
The full scene
still uses conspicuous flat geometry and weak surface detail. The frame gate
can prove paint cost; it cannot prove atmosphere or visual finish.

## Lessons from the installed data

These are source-layout values, not measured display pixels. Layout inheritance,
locale, scaling, and runtime behavior can change the final presentation.

### HUD stability and resource feedback

`hudpanelhd.json` anchors the main HUD at bottom-center. Its health and mana
vessels share a 348×350 layout size. Health explicitly sets
`smoothDecrease=false`; jump smoothing is 0.25 seconds, maximum smoothing is
1 second, and potion preview transparency is 0.35. Mana's potion preview is
0.5. The idle globe animations use 24 and 20 fps, with a source comment about
avoiding synchronization.

**Verdigris application:** keep vital information spatially stable; show
authoritative damage promptly; distinguish impending recovery from available
life; allow subtle independent ambient vessel motion. Apply this through
`native/client/orb_renderer.hpp`, `orb_hud_layout.hpp`, and `ui_skin.hpp`,
without making presentation smoothing authoritative. These are follow-up
design candidates, not changes made by this patch.

### Restrained panels and a deliberate type hierarchy

`_profilehd.json` defines left/right panel anchors, semantic colors and a
font hierarchy. Selected source sizes include small 22, medium 26, tooltip 36,
large 38, and a prominent play button 56. Tooltips use a dark translucent
background with padding, and `tooltipspanelhd.json` declares zero show delay.
`hudmonsterhealthhd.json` describes a top-center target health panel with its
name and additional properties; its declared width is not a trustworthy final
rendered width because the widget can size dynamically.

**Verdigris application:** give the aimed enemy a stable, legible identity;
reserve the strongest emphasis for the current threat and valuable loot;
retain readable full item names and comparisons. Test the closed HUD, target
hover, character+inventory diptych, and tooltip at the same display size.
Do not translate D2R layout constants directly into Win32 pixels.

### Cursor intent and attack rate are distinct from damage formulas

In `skills.txt`, Attack and Bash select animation `A1` and set
`UseAttackRate=1`. Attack, Bash, Fire Bolt, and Ice Bolt set
`KeepCursorStateOnKill=1`. Blank delay cells do not imply instantaneous
repeat attacks; animation and engine rate rules still matter.

**Verdigris application implemented now:** input may select or update a
target, but must not reset the authoritative attack deadline. The native
remote simulation previously overwrote its 350 ms deadline on every trigger.
That made effective damage cadence depend on click/packet frequency.
`WorldSimulation::start_player_attack` now preserves the outstanding deadline.
The first hit remains immediate; repeats and target switches respect recovery.
The 350 ms value is Verdigris's existing rule, not a copied D2 timing.

### Items communicate through multiple channels

Selected `weapons.txt` records differentiate damage range, raw speed,
inventory footprint, and drop sound family. The sampled dagger occupies 1×2
cells; the sampled hand axe, club, and short sword occupy 1×3. These rows also
carry a drop-sound frame. Raw speed and sound-frame values cannot be read as
milliseconds without the relevant animation rules.

**Verdigris application:** make the equipped item affect reach, cadence,
silhouette and sound, and make its loot silhouette recognizable. Use original
Bronze Age assets and the existing Brands/Bonds identity. Retain the House
and extraction loop; do not copy D2's class, inventory, or town-portal rules
where they conflict with the product constitution.

### Encounter character comes from different behaviors and relative speeds

`monstats.txt` gives the sampled zombie group size 1–2 and velocity 1,
fallen group size 2–3 and velocity 5, and skeleton/quill rat velocity 3.
AI families are separately named. These are base data fields, not effective
encounter measurements; difficulty, level scaling, other tables, animation,
and AI execution also matter.

**Verdigris application:** author slow pressure, quick flankers and ranged
threats as distinct encounters. Measure approach time, hits to kill, damage
pressure, empty travel time and useful-loot interval in the real game before
changing rewards or monster counts. A dark palette alone will not create
dangerous exploration.

## Public source references: what is usable and what is historical

**D1 / DevilutionX.** [Devilution](https://github.com/diasurgical/devilution/blob/master/README.md)
is a community reconstruction, not an official Blizzard source release.
[DevilutionX](https://github.com/diasurgical/DevilutionX) develops that foundation.
Its current [license](https://github.com/diasurgical/DevilutionX/blob/master/LICENSE.md)
is the Sustainable Use License; do not treat it as a drop-in source dependency
for this proprietary game. Reference observations are from current master,
accessed on the study date, and are not all attributed to a released version.

- [`RunGameLoop` / `GameLogic`](https://github.com/diasurgical/DevilutionX/blob/master/Source/diablo.cpp)
  separate input/render iterations from the 50 ms logic cadence and explicitly
  order entity processing. Preserve Verdigris's headless simulation boundary;
  do not adopt a historical tick rate merely to imitate nostalgia.
- [`StartAttack` and queued actions](https://github.com/diasurgical/DevilutionX/blob/master/Source/player.cpp)
  connect action progression to animation. The useful principle is a shared
  contact moment, not copying implementation details.
- [`DoVision` / `ProcessVisionList`](https://github.com/diasurgical/DevilutionX/blob/master/Source/lighting.cpp)
  distinguish lighting, visibility and exploration. For Verdigris, a future
  corridor/doorway visibility study is more useful than uniformly dimming the
  scene. Keep visible floor boundaries and attacks readable.

**D2 / D2MOO.** [D2MOO](https://github.com/ThePhrozenKeep/D2MOO) reconstructs
historical **1.10f**, not the installed D2R engine. Its MIT license file and
README's noncommercial-intent language should be recorded together rather
than silently interpreting this as an approved production dependency.

- [`TASK_ProcessGame`](https://github.com/ThePhrozenKeep/D2MOO/blob/master/source/D2Game/src/GAME/Task.cpp)
  reschedules by 40; [game frame accounting](https://github.com/ThePhrozenKeep/D2MOO/blob/master/source/D2Game/src/GAME/Game.cpp)
  divides frames by 25. This is evidence for historical 25 Hz timing, not a
  measured rate of the installed D2R executable.
- [Animation records](https://github.com/ThePhrozenKeep/D2MOO/blob/master/source/D2Common/include/DataTbls/AnimTbls.h)
  contain frame count, speed and frame flags;
  [`UNITS_SetAnimActionFrame`](https://github.com/ThePhrozenKeep/D2MOO/blob/master/source/D2Common/src/Units/Units.cpp)
  reads action events. [`EVENT_SetEvent`](https://github.com/ThePhrozenKeep/D2MOO/blob/master/source/D2Game/src/GAME/Event.cpp)
  schedules unit events against game frames.
- [Skill table loading](https://github.com/ThePhrozenKeep/D2MOO/blob/master/source/D2Common/src/DataTbls/SkillsTbls.cpp)
  distinguishes server/client functions, missiles, sounds and formulas. Tune
  content through data while keeping authority and presentation separate.

[OpenDiablo2](https://github.com/OpenDiablo2/OpenDiablo2) is archived (2021),
and its [Abyss successor](https://github.com/AbyssEngine/OpenDiablo2) is archived
(2023). [OpenD2](https://github.com/eezstreet/OpenD2) describes itself as mostly
unfinished, without a server implementation. They are historical format and
architecture references, not ready-to-ship engine replacements. Their GPL
source is not imported by this milestone.

Blizzard's own design explanations are useful behavioral evidence:
[miss text and accessibility](https://news.blizzard.com/en-us/article/23700733/how-were-making-diablo-ii-resurrectedtm-more-accessible-to-everyone),
[quick casting and visible bindings](https://news.blizzard.com/en-us/article/23746020/diablo-ii-resurrected-patch-2-3-highlights-coming-soon),
and [blocking/input interruption changes](https://news.blizzard.com/en-gb/article/23788293/diablo-ii-resurrected-patch-2-4-ladder-now-live).
Deliberate commitment should be legible; silently lost inputs and unexplained
whiffs are not necessary ingredients of weighty combat.

## Next implementation experiments, in order

1. **One visible contact per resolved attack.** Local damage currently resolves
   in the same tick as AttackStarted, while the new swing is still labeled
   Windup for its first 28%. The remote path also predicts an arc, then can
   create another when combat:hit confirms contact. Correlate/reconcile the
   visible action before tuning decorative effects. Require recorded frames
   to show contact, damage, sound and hit reaction agreeing; trace labels alone
   do not establish this.
2. **A focused HUD comparison.** Hold camera, resolution and encounter constant;
   compare threat identity, resource loss/recovery, inventory comparison, and
   loot readability against the extracted layout intent. Use original bronze,
   stone, hide and patina artwork through the shared skin.
3. **A five-minute encounter benchmark.** Record goal selection, first threat,
   first contact, first useful drop, first return decision and extraction.
   Separate measured intervals from proposed targets. Compare a slow pursuer,
   fast group and ranged pack. Preserve the constitution's Scion/House loop.
4. **Atmosphere with visibility and audio.** Capture a doorway, corridor and
   crowded room; establish what should be seen before it is heard and what
   should be heard before it is seen. D2R audio mixing and actual controller
   feel remain unmeasured in this study.

This work is a reusable reference and one verified pacing correction. It is
not a full reverse engineering of D2R or a completed visual overhaul.
