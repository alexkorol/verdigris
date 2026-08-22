# TASK-0152 evidence contract

`native/tools/entity_density_bench.cpp` (tool_version 2.0.0) emits
`verdigris-density-evidence/2` JSON. It is measurement-only: no gameplay,
simulation-rule, or presentation behavior changed.

## Run mode

```powershell
native\build\entity_density_bench.exe --n N --run R [--ticks T] [--seed S] --out path.json
native\build\entity_density_bench.exe --validate path.json
```

Every invocation executes the seeded scenario TWICE in fresh `Simulation`
instances and requires both to agree on counts and the FNV-1a64 state
checksum; disagreement exits 1. The fixed scenario id is
`density-melee-contact` (enter `route:tin:1:0`, pack N monsters at melee
contact via `Simulation::spawn_monster`, drive T scripted `Melee` frames).

## Evidence sections

- `scenario` — fixed id, route, action, n, ticks, seed, run.
- `provenance` — generated_at_utc, tool_version, git_ref, compiler,
  cxx_standard, build_config, os, arch, cpu, cpu_cores, timer,
  timer_resolution_ns, host. All fields mandatory; `--validate` rejects
  missing/empty provenance.
- `determinism` — runs=2, reproducible, counts_match, checksum_match,
  state_checksum, state_checksum_repeat, counts, counts_repeat.
- `timings` — update_ms (dispatch only) and frame_ms (dispatch + read-only
  presentation state pass feeding the rolling checksum), each with
  p50/p90/p99/max/mean in milliseconds using nearest-rank percentiles.
- `thresholds` — the check list below plus `all_pass`.

## Threshold contract `verdigris-density-threshold/1`

| check id | op | bound | meaning |
|---|---|---|---|
| monsters_spawned | min | n | spawn seam delivered the requested density |
| samples_complete | min | ticks | no dropped update/frame samples |
| reproducible | min | 1 | double-run counts+checksum agreement |
| frame_p99_within_budget | max | 50 ms | one fixed-step frame budget |
| frame_mean_within_budget | max | 50 ms | mean frame within the budget |
| update_p99_within_budget | max | 50 ms | simulation step p99 within budget |
| ticks_per_sec_floor | min | 20 | real-time simulation floor |

## Exit codes

- 0 — evidence complete, reproducible, thresholds passing (validate: valid)
- 1 — malformed, incomplete, irreproducible, threshold-failing, or unwritable
- 2 — usage error (unknown scenario id, bad flags)

## Captures

- `captures/density-n{50,500,1000}-seed*-run{A,B}.json` — two identical seeded
  process invocations per density; A/B agree on counts and checksums.
- `captures/invalid/*.json` — negative fixtures; every one exits 1 under
  `--validate` (truncated JSON, garbage, missing provenance field, flipped
  reproducibility/threshold, incomplete percentiles/samples).
