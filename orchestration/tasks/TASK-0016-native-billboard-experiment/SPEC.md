---
id: TASK-0016
title: Billboard-sprite experiment in the native client
state: READY
track: native
priority: high
base_commit: current program tip (Codex records the actual SHA in STATUS.md on claim)
dependencies: [TASK-0013]
parallel_safe: true
owned_paths:
  - native/client/**
  - native/renderer/**
forbidden_paths:
  - native/src/**
  - native/include/**
  - native/CMakeLists.txt
  - native/build.ps1
  - native/tools/**
  - src/**
  - server/**
  - prototypes/founding-slice/slice.html
  - prototypes/founding-slice/index.html
  - prototypes/founding-slice/build.mjs
  - prototypes/founding-slice/assets/**
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests -RunClient
---

## Goal

The native Win32 client can render actor billboards from the
already-vendored magenta-keyed plates (READ-ONLY from
`prototypes/founding-slice/assets/`) instead of placeholder capsules,
with magenta keyed to transparency at load — the first native step of the
Milestone E visual pass and the slice→native bridge.

## Why this task exists

The slice proved the 2.5D billboard look works (D-102 evidence pack); the
native client still draws capsules. The plates are committed derivatives
already approved for prototype use; loading them read-only keeps D-O2
(full asset pipeline) untouched while making the native lab visually
real.

## Product and architectural invariants

- Simulation untouched; headless output byte-identical.
- Assets are loaded AT RUNTIME from the existing repo path (resolve
  relative to the executable/repo root; degrade gracefully to capsules
  with an on-screen note if files are missing — the build must not embed
  or copy them).
- GDI/GDI+ only (no new external dependencies). Magenta keying (min(r,b)
  − g threshold, as the slice does) happens once at load into an
  alpha-capable bitmap.
- Depth sorting, contact shadows, and the camera lab keep working; the
  billboard anchors at the sprite's lowest opaque row (foot anchoring,
  like the slice loader).

## Scope

1. A small asset loader in the client (or `native/renderer/` as a
   client-linked file ONLY if no build-file change is needed — build
   files are forbidden, so in practice keep it in `native/client/`):
   load `scion_str` (player), `raider` (monster), `boss` (elite) plates,
   key magenta, compute foot anchor.
2. Replace capsule rendering for player/monsters with the billboards
   (AlphaBlend or manual per-pixel pre-multiplied draw — pick the
   simplest correct GDI approach), horizontal mirror by facing sign,
   preserving hit-flash (brighten) and death fade if cheaply possible.
3. Fallback path renders the old capsules when assets are absent
   (verified by renaming the folder in a manual check).
4. Debug overlay notes billboard mode on/off.

## Non-goals

Scenery billboards, animation frames, asset pipeline/packaging (D-O2),
terrain texturing, performance work.

## Deliverables

Client changes, one coherent commit.

## Acceptance criteria

- `build.ps1 -RunTests -RunClient` green; headless unchanged.
- Driven-input pass with PrintWindow captures showing: billboarded player
  and raider in a fight, mirror flip when aiming left vs right, and the
  fallback capsules when the asset folder is renamed away. Captures in
  REPORT.md.

## Required verification

The acceptance command plus the driven pass above.

## File ownership

`native/client/**` (and `native/renderer/**` if usable without build
changes). Sole in-flight client task.

## Dependencies

TASK-0013 integrated.

## Parallel-safety assessment

Disjoint from TASK-0015 (core). No other client task in flight.

## Review focus

Keying quality at edges (despill), anchor correctness on the ground
plane, fallback robustness, and zero simulation/build coupling.

## Stop conditions

Any need for build-file changes (e.g., GDI+ link flags beyond what
`user32/gdi32` linking allows — note msimg32 AlphaBlend needs no build
change via LoadLibrary, but if a .lib addition is unavoidable) → stop and
file a question with the exact linker need.
