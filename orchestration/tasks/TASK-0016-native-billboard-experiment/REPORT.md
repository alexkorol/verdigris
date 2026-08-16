---
task: TASK-0016
state: REVIEW_REQUESTED
branch: codex/native-reconstitution
commits:
  - 6d1b7d6
base_commit: b1ef7c2
---

# TASK-0016 report — native billboard experiment

## Executive summary

The native Win32 client now loads the existing `scion_str.png`, `raider.png`,
and `boss.png` plates at runtime. The client applies the slice-compatible
magenta key/despill pass once at load, computes a lowest-opaque-row foot anchor,
and draws premultiplied-alpha billboards with a mirrored source for left-facing
actors. If the plates or image support are unavailable, the existing capsule
renderer remains active and the debug line states the fallback reason.

## Changed files

- `native/client/main.cpp` — dynamic GDI+/AlphaBlend loader, keyed DIBs,
  foot-anchored/mirrored actor drawing, runtime path probing, fallback status.
- `orchestration/tasks/TASK-0016-native-billboard-experiment/captures/` —
  driven PrintWindow evidence for the asset and fallback paths.

No simulation, build-system, renderer-library, or prototype asset files were
changed.

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests -RunClient` — PASS;
  `native legacy denylist: PASS`, `verdigris core tests: PASS`, and the native
  client shell completed its headless summary.
- `git diff --check` — PASS.
- Asset pass: the driven client loaded all three plates and displayed
  `billboards: on (scion_str / raider / boss; magenta keyed)` in the overlay.
- Mirror pass: `billboard-aim-right.png` and `billboard-aim-left.png` capture
  the same fight after aiming to opposite sides, exercising the facing-sign
  mirror branch.
- Fallback pass: the existing assets directory was temporarily renamed to
  `assets.disabled-task0016`, the client was driven again, and the overlay
  displayed `billboards: off (fallback capsules; asset plates missing)`.
  The directory was restored unchanged.

## Evidence

- [Asset initial](captures/billboard-initial.png)
- [Asset aim right](captures/billboard-aim-right.png)
- [Asset aim left](captures/billboard-aim-left.png)
- [Fallback initial](captures/billboard-fallback-initial.png)
- [Fallback aim right](captures/billboard-fallback-aim-right.png)
- [Fallback aim left](captures/billboard-fallback-aim-left.png)

## Interface and runtime notes

The loader uses only Win32/GDI+ entry points resolved at runtime; no linker or
CMake changes are required. It probes the repository-relative asset path and
paths derived from the executable directory, without embedding or copying
plates. Premultiplied-alpha DIBs are retained for the client lifetime and
released with RAII. The simulation and its authoritative facing values remain
untouched.

## Deviations and risks

None. The window title is not used by the evidence harness; the stable native
window class remains available for PrintWindow capture. Architect review is
required for acceptance.
