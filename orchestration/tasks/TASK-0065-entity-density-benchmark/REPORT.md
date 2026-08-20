---
task: TASK-0065
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0065-entity-density-benchmark-cursor
base_commit: 5c41a04821695e38261fdb52f2e86b2dea67f21d
architect_review_required: true
---

# TASK-0065 REPORT — N7 entity-density benchmark

## Executive summary

A opt-in native benchmark (`powershell -File native/build.ps1 -RunDensityBench`)
spawns N ∈ {50, 200, 500, 1000} monsters through `Simulation::spawn_monster`,
runs 1000 fixed-step Melee dispatches, and writes JSON. Three runs per N are
committed. At N=1000 the median is ~182k ticks/s with p99 ≈ 0.0075 ms versus
the 50 ms tick budget. Default unit gates stay green. No `native/src`
behavior changes.

## Approach

- `native/tools/entity_density_bench.cpp` — enter `route:tin:1:0`, spawn until
  alive monster count is N (enter already placed one), then `dispatch(Melee)`
  1000 times with per-tick `steady_clock` samples.
- `native/build.ps1 -RunDensityBench` — compile/link the bench and loop the
  12 runs into the task `captures/` folder. Not part of `-RunTests`.
- JS attach script skipped: the JS server has no spawn seam, and adding one
  would require `server/` edits forbidden by the SPEC's "only if no server
  changes" clause. Shape documented in RESULTS.md.

## Changed files

- `native/tools/entity_density_bench.cpp` (new)
- `native/build.ps1` — opt-in `-RunDensityBench`
- `orchestration/tasks/TASK-0065-entity-density-benchmark/RESULTS.md`
- `orchestration/tasks/TASK-0065-entity-density-benchmark/captures/density-n*-run*.json` (12)

## Verification

1. `powershell -File native/build.ps1 -RunTests` — denylist / core /
   networking / camera2d / session PASS (2026-08-20). Default gates do not
   invoke the bench.
2. `powershell -File native/build.ps1 -RunDensityBench` — 12/12 JSON files.
   Architect rerun target: one N=500 pass
   (`native/build/entity_density_bench.exe --n 500 --run 1 --out ...`).

## Deviations

- Melee does not wipe the pack in 1000 ticks (cooldown + nearest-target), so
  N stays constant — that is the density we wanted to measure.
- Event-log growth is included (no src change to bound `events_`).
- JS comparison is a note, not a number.
