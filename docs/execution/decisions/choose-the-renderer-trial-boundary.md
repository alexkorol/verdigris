# VG-GOV-005 — Choose the renderer trial boundary

Draft 2026-09-06. Extends TASK-0114 (INTEGRATED evaluation, no ADR).
Does not mint TASK numbers. Does not authorize an engine migration.

## Trial that is in bounds

The approved limited GPU proof on this lease is the **software sample**
already wired as `--scenario gpu-sample` / `gpu-packets` / `gpu-reference`
(`native/renderer/gpu/**`). Platform target for that proof: Win32 client
with a software present path. macOS/D3D device-loss remain out of this
trial.

TASK-0114 recorded sokol_gfx and SDL2 as research candidates. Those
citations are not a dependency approval and not a port order.

## Stop criteria

- A green triangle, quad, or packet snapshot does **not** authorize
  replacing the production GDI/GDI+ present path.
- `backend_handle` stays 0 on snapshots.
- No full-engine migration, no runtime `.hlsl` path, no new third-party
  GPU SDK in the client from this decision.

## Negative control

`gpu-sample` passing cannot be filed as “renderer chosen / engine port
started.” Actor-representation comparison stays the existing billboard
+ vector kit vs the software sample materials, not a second mesh engine.

## Status

**Drafted** on the Cursor GPU lease. Owner may still pick sokol/SDL later;
this packet only freezes the current trial boundary.
