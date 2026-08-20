# TASK-0078 — Native surface density (presentation delta #3)

**Packet type:** MECHANICAL-VISUAL (native client presentation only)
**Ports:** per-lane capsule. **Base:** master (after PR #45).

## Why

D-122 axis 3. Server/rules parity is complete (32/32 attach, PR #45);
the remaining visible gap in the side-by-sides
(`orchestration/benchmarks/side-by-side-2026-08-20/sxs3-*.jpg`, delta
list in BENCHMARK.md) is surface density: the browser town reads as a
village (walls, paths, fountain plaza, NPC sprites); the native
surface is sparse.

## Scope

In `native/client/` presentation ONLY (no simulation, no networking,
no protocol changes):

1. Town surface: render the village layout the server already ships —
   road/path tiles between the four road gates, plaza ring at the
   wagon pitches (kWagonPitches), fountain marker at the fountain
   tile, and the four town NPCs (Aldwyn 34,116; Mara 49,103;
   Ludovicus 19,113; Rhea 31,121) with name plates.
2. Wall/boundary treatment around the village bounds (browser uses
   brick rows; any consistent tile treatment from the existing
   terrain1/terrain4 atlases is acceptable — NO new art assets).
3. Density pass on instance surfaces: ground variation from the
   existing atlases so floors don't read as a flat grid.

## Non-goals

Expedition/quest panels and typography (delta #4, separate task).
No gameplay change; render list only.

## Verification (all required)

- `powershell -File native/build.ps1 -RunTests -RunClientScenarios`
  green (render-list scenarios must still pass — extend expected op
  counts if the checks pin them).
- `native/build/verdigris_client.exe --reference-scene all` — two-run
  JSON identical (determinism preserved).
- Regenerate side-by-side: copy fresh captures into
  `orchestration/benchmarks/side-by-side-2026-08-20/native-after/`,
  run the composite (see BENCHMARK.md tail), attach sxs4-01 and
  sxs4-02 to REPORT.md.
- Screenshot evidence in REPORT.md; architect review REQUIRED before
  merge (visual-quality bar per cursor calibration note).
