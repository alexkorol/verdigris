---
task: TASK-0012
state: REVIEW_REQUESTED
branch: codex/TASK-0012-slice-camera-evidence
base_commit: e6d3f48
---

# Camera-preset evidence pack

## Executive summary

Captured the three founding-slice camera presets in the same Hearthstead
village scene, with the same controlled chieftain melee telegraph and the same
controlled q3 masterwork on the ground. The result is nine lossy JPEGs at
1200×800 (therefore ≤1280px wide and ≤250KB each), plus the exact camera
values read from the live page.
This report records observations only; it does not rank the presets or make a
projection decision. D-102 remains provisional.

## Approach

The existing `window.__V` surface and Laboratory debug panel were driven
read-only through Playwright. Each preset ran in a fresh browser context:

1. Start a new Dexterity Scion in `hearthstead` and pick up the intro spear
   through the real `E` interaction.
2. Restore the common player origin `(0, 520)` and apply one preset through its
   Camera lab button.
3. Capture the quiet village scene.
4. Hold a level-1 Ashmark Chieftain at `(110, 520)` until its arc telegraph is
   visible, then capture the mid-combat state.
5. Remove the actor, restore player position/life/breath, use the existing
   `give masterwork` debug action, and capture one q3 item at player `x + 60`.
6. Re-encode each PNG to progressive JPEG at quality 85, verify dimensions and
   file sizes, and visually inspect the telegraph and loot beacon after
   compression.

The temporary Playwright driver was removed after capture; no prototype or
shared configuration file was edited. The final driver run completed three
contexts, nine screenshots, and zero page/console errors.

## Exact preset parameters read from the page

The `displayed` column is the value shown by the Camera lab label and matches
the live `__V.cam` object. `input` is the browser range element's value.

| Preset | pitch input/displayed | zoom input/displayed | perspective input/displayed | player anchor | fog | tilt-shift |
|---|---:|---:|---:|---:|---:|---:|
| Miniature | 52 / 52 | 1.1 / 1.08 | 0.0013 / 0.0013 | 0.58 | 0.6 | 1 |
| ARPG | 62 / 62 | 0.85 / 0.85 | 0.0006 / 0.0006 | 0.52 | 0.4 | 0 |
| High Table | 74 / 74 | 0.75 / 0.75 | 0.0003 / 0.00025 | 0.5 | 0.25 | 0 |

The exact live camera values are therefore:

```text
Miniature:  zoom 1.08, pitch 52, perspective 0.0013, anchor 0.58, fog 0.6, tilt 1
ARPG:       zoom 0.85, pitch 62, perspective 0.0006, anchor 0.52, fog 0.4, tilt 0
High Table: zoom 0.75, pitch 74, perspective 0.00025, anchor 0.5, fog 0.25, tilt 0
```

## Controlled state evidence

All nine captures are in `state=instance`, node `hearthstead` / Hearthstead,
with the player restored to `(0, 520)`. The combat captures contain one level-1
`chieftain` at approximately `(110, 520)`, with `telegraph=arc` and duration
`0.45s`; `effectsVisible=true`. The loot captures contain no live entities and
one q3 masterwork at player `x + 60`.

The q3 item was generated under the same temporary deterministic random
instrumentation in each loot capture, so its visible name is the same:
`Stone Short Blade of the Ox of the Hare`. The intro spear's optional affix can
vary in the HUD between fresh contexts, but the world setup, actor position,
combat actor, and ground-loot state are held constant.

## Capture index

| Preset | Village scene | Mid-combat arc telegraph | Loot on ground |
|---|---|---|---|
| Miniature | [miniature-village.jpg](captures/miniature-village.jpg) | [miniature-combat.jpg](captures/miniature-combat.jpg) | [miniature-loot.jpg](captures/miniature-loot.jpg) |
| ARPG | [arpg-village.jpg](captures/arpg-village.jpg) | [arpg-combat.jpg](captures/arpg-combat.jpg) | [arpg-loot.jpg](captures/arpg-loot.jpg) |
| High Table | [high-table-village.jpg](captures/high-table-village.jpg) | [high-table-combat.jpg](captures/high-table-combat.jpg) | [high-table-loot.jpg](captures/high-table-loot.jpg) |

All nine files were checked after re-encoding. Each is progressive JPEG,
1200×800, quality 85, and below the 250KB target:

```text
arpg-combat.jpg        1200x800  161964 bytes
arpg-loot.jpg          1200x800  161574 bytes
arpg-village.jpg       1200x800  159937 bytes
high-table-combat.jpg  1200x800  172136 bytes
high-table-loot.jpg    1200x800  172110 bytes
high-table-village.jpg 1200x800  170694 bytes
miniature-combat.jpg   1200x800  133997 bytes
miniature-loot.jpg     1200x800  133553 bytes
miniature-village.jpg  1200x800  131912 bytes
```

## Neutral observations

### Miniature

- The chieftain's red ground arc is visible as a broad, readable telegraph
  between the two billboards.
- Character and enemy billboards have distinct circular contact shadows, which
  visually grounds them against the village plane.
- The stronger perspective value and tilt-shift produce a pronounced depth
  change across the ground grid and scenery; distant props reduce in scale.
- The outer trees and upper scenery reach the frame edges, and the tilt-shift
  treatment softens the top and bottom bands. The capture does not show a
  gameplay obstruction in the central combat area.

### ARPG

- The same arc telegraph remains visible around the combatants, with its shape
  readable against the ground texture.
- Contact shadows continue to anchor both billboards to the ground plane.
- The lower perspective value presents the village plane with a milder depth
  change than Miniature; the grid reads more evenly from foreground to back.
- The side scenery remains within a comparatively stable frame; there is no
  tilt-shift blur band in this preset, and edge geometry is less visibly
  softened.

### High Table

- The same red arc remains visible, though its projected footprint is more
  compact around the actors at the higher pitch.
- Contact shadows remain visible under both billboards and preserve their
  ground relationship.
- The high pitch exposes more of the village plane and reduces the visible
  depth scaling; the scene reads closer to a top-down table surface.
- A dark upper band is visible where the pitched ground meets the backdrop,
  and the large trees at the lateral edges approach or cross the frame edge.
  This is recorded as an observation, not a recommendation.

## Camera-lab defects and limitations

1. The Camera lab's range element cannot represent every preset value at its
   declared step. Miniature reports an HTML input value of `1.1` for zoom while
   the displayed label and live camera are `1.08`; High Table reports an input
   value of `0.0003` for perspective while the displayed label and live camera
   are `0.00025`. The preset button still applies the exact object values, so
   this is a control/readback mismatch rather than a capture failure.
2. High Table's pitch exposes a dark upper backdrop band and edge-adjacent
   trees in the village view. No source change was made; the screenshots are
   the evidence of the current camera lab behavior.
3. Fresh contexts can generate different optional affixes on the intro spear,
   which changes the HUD life total/name without changing the village layout,
   combat actor, or loot setup. The capture harness kept the scene geometry and
   controlled loot deterministic; this remaining HUD variance is noted for
   interpretation.

## Verification

Independent validator `/root/validate_task_0012` — ACCEPT. It confirmed the
task-folder-only diff, nine valid 1200x800 captures, neutral observations,
parameter/defect evidence, and a clean worktree.

Original scripted Playwright capture command:

```text
node captures/drive.mjs
```

Result: three fresh contexts completed; all nine screenshots written; all
camera parameter reads returned; `errors: []` for every context. The temporary
driver was removed after the run, leaving the nine JPEG artifacts and this
report in the task folder.

JPEG conversion command (quality 85, optimized progressive output):

```text
python -  (Pillow Image.convert('RGB').save(..., format='JPEG', quality=85,
         optimize=True, progressive=True))
```

JPEG dimension/size check: passed for all nine files (`1200x800`, each
131,912–172,136 bytes, all below 250KB).

Manual post-compression visual check: passed. The red chieftain arc remains
clearly visible in all three `*-combat.jpg` captures, and the gold diamond
loot beacon plus its item label remain clearly visible in all three
`*-loot.jpg` captures. JPEG compression did not change the neutral
observations above.

Reference slice harness (run from the repository checkout with the existing
Playwright installation):

```text
node prototypes/founding-slice/run-checks.mjs
4/4 checks passed in 18.1s
```

Repository preflight and scope proof before report creation:

```text
git status --short
?? orchestration/tasks/TASK-0012-slice-camera-evidence/captures/
```

After adding this report, the only changed paths remain inside the task
folder:

```text
git status --short
?? orchestration/tasks/TASK-0012-slice-camera-evidence/REPORT.md
?? orchestration/tasks/TASK-0012-slice-camera-evidence/captures/
```

No acceptance command was specified by the task (`acceptance_commands: []`).
No prototype, native, shared configuration, or browser-game source file was
modified.

## Deviations, risks, and follow-ups

- Deviation: the original PNG captures were re-encoded to lossy progressive
  JPEG at quality 85 per review; the source PNGs are no longer retained in the
  task folder.
- Risk: JPEGs are point-in-time visual evidence, not a visual-regression gate;
  animation and asset rendering can vary slightly across browser versions,
  and lossy compression can soften very fine texture detail.
- Follow-up: if the owner wants to resolve the range readback mismatch, the
  step/min/max contract should be revised in the camera lab as a separate task.
- No owner decision or question is required by this evidence task. D-102 stays
  provisional.

## Commit

Original capture commit: `310b76d` — `docs: capture camera preset evidence pack`.

Revision conversion commit: `f813a2e` — `docs: compress camera evidence captures`.

Final report commit: `4b27757` — `docs: record camera evidence compression revision`.
