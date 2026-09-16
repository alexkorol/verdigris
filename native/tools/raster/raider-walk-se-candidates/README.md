# Rejected SE raider walk study

**Do not promote these frames as a walk.** One initial generation and two focused corrections exhausted the bounded attempt. All versions preserve the lower-right view and anatomical-right axe hand, but every contact advances the anatomical-left leg from the screen-right hip. The anatomical-right/axe-side leg remains behind. The second half never becomes the required opposite contact/support.

The native review also shows broader proportions and changed costume/axe shapes. The accepted idle has a56-pixel crown-to-sole body; this common-pitch reconstruction has60–61pixels. Palette snapping uses the actual idle colors but cannot repair that geometry or the missing motion.

## Inspect

- `raider-walk-se-v3-idle-cycle-idle-1x.png`: exact native ordered comparison, original idle at both ends.
- `raider-walk-se-v3-contact-3x.png`: integer3x view of all8 cells.
- `raider-walk-se-v3-loop-1x.gif` and `-3x.gif`: fixed80x96canvas; eight original row-major frames,100ms each. Frame order/count/timing were checked; smooth in-game playback was not observed or accepted.
- `raider-walk-se-review.json`: phase-by-phase verdict, possible isolated reference poses and output hashes.
- `raider-walk-se-cleanup-review.json`: source foreground probes and eight inspected enclosed underarm checker windows.

F0 is potentially useful as an isolated anatomical-left-contact study, and F1 as its compression. They are not approved identity anchors. A future single-frame repair would need the anatomical-right leg/axe-side leg to become the forward planted leg while the current left support lifts and trails. Keep the original SE idle as identity. No further generation was performed after root's stop.

## Reproduce the rejection inspection

From the isolated repository root, use the existing external project:

```powershell
& Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe native/tools/raster/import_assets.py native/tools/raster/raider-walk-se-candidates/raider-walk-se-v3-import.json
& Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe native/tools/raster/raider-walk-se-candidates/review_raider_walk_se.py
```

The manifest is **candidate-only**, not an active runtime import/registration. It uses original source `native/client/assets/raster/source/raider-walk-se-v3.png`, accepted palette `native/client/assets/raster/runtime/raider_se.png`, and actual Pixel Respecter `workspace.reconstruct`, border-connected transparency and `palettes.snap_to_palette`. Source cell384x490, column stride384, row stride490, common source origin[192,462], pitch6, output80x96, integer pivot[40,96]. No per-pose fit, resize or recentering.

All generated sources were RGB with painted checker. Cleanup removed border-connected checker plus exactly8 reviewed enclosed underarm pockets; source mask/fists/axe/sash/boot probes remained opaque. Final native frames have binary0/255alpha and at most32colors. Original source files remain unchanged.

Exact minimal conversion dependencies, versions, engine hashes, sources and prompts are in `raider-walk-se-dependencies.json` and `v3/raider-walk-se-v3.provenance.json`. Current external environment: Python3.12.6, NumPy2.2.2, Pillow10.4.0, OpenCV4.11.0. No installs, runtime/catalog/code edits or git mutations were made.

## Generation evidence

Sources and exact prompts/provenance are versioned as `raider-walk-se-v1`, `v2`, `v3` in the raster source/prompts directories. Initial reference-led sheet structure adapts [Practical_Low29's original combat-sheet prompt](https://www.reddit.com/r/aigamedev/comments/1wbmvnm/gpt_image_25_nailed_a_16_frame_combat_sprite_sheet/), directly opened for this task. That creator describes a combat study, not a validated adult gait. The final correction additionally used the existing external, direction-validated D2SE ordered sheet only for lower-body motion; that is our unsuccessful local reference-role experiment. No external D2 pixels were copied into output or repository. The exact external path/hash is recorded for generation reproduction and is not needed to reconstruct our saved output.

Only the built-in imagegen tool was used for generated art. It exposed no model selector or model identity; no specific variant is claimed.
