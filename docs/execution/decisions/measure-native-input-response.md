# Measure native input response (VG-MOVE-008)

Draft protocol on the Cursor client lease. Not a VG-GOV-002 ruling.

## Method

1. On key-down or mouse button-down, store `QueryPerformanceCounter` (`note_input`).
   Do not stamp `WM_MOUSEMOVE`.
2. At the end of `paint_scene` (the production present path used by live
   WM_PAINT and headless `--scenario`), store present QPC (`note_present`).
3. Each paired sample is input-to-present milliseconds.
4. Report p50 and p95 over a cap of 128 samples.

## Machine

Record `SM_CXSCREEN`×`SM_CYSCREEN`, logical CPU count, Win32, GDI present.
Same fields as VG-PERF-001 `frame-budget`.

## Uncertainty

- One client frame (timer ~15 ms) plus QPC granularity.
- No display vsync, no GPU readback, no photodiode.
- VG-MOVE-007 action buffering is not on this path.

## Forbidden label

Headless `Simulation::dispatch` elapsed time is command time. It must not
be published as `input-latency:photon` or "input-to-photon".
