# TASK-0167 — Framekit raster slice pack — REPORT

Lane: ox-alpha-pc-w1 · Worktree: `Z:\Code\.worktrees\verdigris\worker-t0167` · Branch: `ox/TASK-0167`
State: REVIEW_REQUESTED · Implementation commit: `2ee53fe03cc31b21eb30da643f5f2626f62e8970`

## Executive summary

Adopted the real, deterministically generated WIZARD gui_framekit art (FK-107) as a native
raster slice pack under `native/client/assets/wizard/framekit/`: 5 verbatim textures
(nine-slice panel, nine-slice slot, 3 circular orb sprites) plus 9 derived nine-slice
pieces (4 corners, 4 edge strips, 1 fill) cropped from the authentic 48x48 panel texture
with alpha preserved. Metadata ships as `nine_slice.json` (per-slice source file, pixel
margins, anchors, stretch axis, DPI scale 4) and `adoption_manifest.json` (per-artifact
sourcePath + sha256 + bytes + provenance hash). A deterministic verifier
(`native/tools/verify_framekit_assets.py`) enforces existence, RGBA alpha, minimum
dimensions, hash/size match, verbatim provenance equality, slice-rect bounds, and the
nine_slice.json schema, with a `--corrupt` negative control that mutates a temp copy and
must fail. A Pillow contact sheet tiles every artifact with labels and includes a
nine-slice reconstruction proof (220x160 panel rebuilt from the slices).

No generic CSS-style substitutes, no procedural stand-ins: every pixel descends from the
WIZARD source art.

## References inspected (read-only, unmodified)

- `Z:\Code\WIZARD\tools\gui_framekit\assets\` — textures/panel.png (+.json), textures/slot.png (+.json), sprites/orb-*.png, evidence/fk-107-assets-demo.png
- `Z:\Code\WIZARD\tools\gui_framekit\tools\generate_assets.py` — slice geometry (SLICE=12), palette provenance
- `Z:\Code\WIZARD\tools\gui_framekit\INTERFACES.md` + assets README — sidecar format `{slice:[t,r,b,l],width,height}`
- `Z:\Code\WIZARD\tools\gui_framekit\tokens\tokens.css` — DPI/palette token hints (4px base, brass/verdigris)
- `Z:\Code\.worktrees\verdigris\owner-demo-runway\native\client\assets\wizard\source_manifest.json` — framekit family sha256 provenance (all 5 adopted sources hash-verified at build time)
- `Z:\Code\.worktrees\verdigris\owner-demo-runway\native\tools\verify_wizard_source_manifest.py` — verifier style precedent

## Changed files (all within owned_paths)

- `native/client/assets/wizard/framekit/panel_frame.png` — verbatim WIZARD panel.png (48x48 RGBA, 1457 B)
- `native/client/assets/wizard/framekit/slot_frame.png` — verbatim WIZARD slot.png (32x32 RGBA, 639 B)
- `native/client/assets/wizard/framekit/orb_vitality.png`, `orb_mana.png`, `orb_essence.png` — verbatim 16x16 circular sprites
- `native/client/assets/wizard/framekit/panel_corner_{tl,tr,bl,br}.png` — derived 12x12 corner slices
- `native/client/assets/wizard/framekit/panel_edge_{top,bottom,left,right}.png` — derived edge strips (24x12 / 12x24)
- `native/client/assets/wizard/framekit/panel_fill.png` — derived 24x24 center fill
- `native/client/assets/wizard/framekit/nine_slice.json` — nine-slice metadata (margins 12/12/12/12, anchors, stretch, dpiScale 4)
- `native/client/assets/wizard/framekit/adoption_manifest.json` — 14 artifacts, sourcePath + sha256 + bytes + sourceSha256
- `native/tools/verify_framekit_assets.py` — deterministic verifier + `--corrupt` negative control
- `orchestration/tasks/TASK-0167-framekit-raster-pack/build_slice_pack.py` — reproducible adoption builder (provenance-gated)
- `orchestration/tasks/TASK-0167-framekit-raster-pack/contact_sheet.png` — visual evidence
- `orchestration/tasks/TASK-0167-framekit-raster-pack/STATUS.md` — claim + review state

## Commands and exit codes

| Command | Exit | Result |
|---|---|---|
| `python native/tools/verify_framekit_assets.py` | 0 | VERIFY OK: 14 artifacts, hashes/alpha/dimensions/nine-slice schema valid |
| `python native/tools/verify_framekit_assets.py --corrupt` | 1 (expected) | negative control: mutated temp copy detected via sha256+size mismatch |
| `python native/tools/check_legacy_denylist.py` | 0 | PASS |
| `git diff --check` | 0 | clean |
| `git status --short` (after final commit) | 0 | clean |

Commits: `3b2ff30a` claim · `2ee53fe0` implementation · final `implement(TASK-0167): framekit raster slice pack`.

## Evidence paths

- Contact sheet: `orchestration/tasks/TASK-0167-framekit-raster-pack/contact_sheet.png` (all 14 artifacts, checkerboard alpha backdrop, derived-vs-verbatim color-coded labels, nine-slice reconstruction proof)
- Source art evidence: `Z:\Code\WIZARD\tools\gui_framekit\assets\evidence\fk-107-assets-demo.png`
- Verifier: `native/tools/verify_framekit_assets.py` (re-run anytime; `--pack-dir` override supported)

## Residual gaps

- WIZARD gui_framekit provides **no raster** divider/button art (buttons, tabs, dividers are CSS-only in FK-103); per the "if the source provides them" clause none were adopted. If native buttons/dividers are needed, a successor must generate raster equivalents in WIZARD first (do not synthesize here).
- Panel/slot textures are fully opaque RGBA (alpha channel present, all 0xFF) — that is the authentic WIZARD output; true transparency exists only in the orb sprites. The Framekit render adapter should composite the panel over the scene backdrop rather than expect see-through frames.
- `nine_slice.json` slot entry carries empty `slices` (slot is consumed as a whole nine-slice texture); per-piece slot slices can be derived identically if TASK-0180 prefers pieces.

## Successor notes (TASK-0180 Framekit render adapter)

- Load `panel_frame.png` with margins `{12,12,12,12}` at dpiScale 4 (texture is 4x the 1px engraved-border design; draw at 0.25x for logical 12px frame, or keep 1:1 for chunky look — tokens.css 4px spacing base suggests 0.25x).
- Corner pieces (`panel_corner_*.png`) are provided for atlas baking / hardware-clipped rendering; edges stretch along their `stretch` axis only, fill stretches both (`nine_slice.json` `slices` map has rect + anchor + stretch per piece).
- `slot_frame.png` is a self-contained 32x32 nine-slice (margins 12) for inventory cells; orbs are 16x16 center-anchored circular sprites for HUD resources.
- Verify integration changes with `python native/tools/verify_framekit_assets.py`; any re-adoption must go through `build_slice_pack.py` so provenance hashes are re-validated against the runway source manifest.
