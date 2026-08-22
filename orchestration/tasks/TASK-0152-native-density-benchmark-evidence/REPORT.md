# REPORT — TASK-0152 native density benchmark evidence

Worker: ox-pc-ac (coordinator ox-alpha, openrouter/stealth/ox-alpha,
OpenCode CLI 1.18.21 variant max). Branch:
`codex/TASK-0152-native-density-benchmark-evidence-ox-pc-ac`. Base at claim:
`c2b814488278f4f093e754cf695ea9ed749d81fb` (spec base
`060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2` verified ancestor).

## Executive summary

The TASK-0065-era density bench is now a reproducible evidence producer:
fixed scenario id (`density-melee-contact`), complete hardware/build
provenance, p50/p90/p99/max/mean percentile frame and update timings, a
built-in double-run seeded determinism gate (counts + FNV-1a64 state
checksum), the documented `verdigris-density-threshold/1` contract enforced
on every run, and a strict `--validate` mode that fails malformed or
incomplete evidence with exit 1. Two identical seeded process runs agree on
counts/checksums at N=50/500/1000; five negative fixtures all fail as
required. No gameplay, simulation, or presentation code changed.

## Approach

Single owned file rewritten (`native/tools/entity_density_bench.cpp`),
dependency-free C++20, /W4-clean under MSVC 2019 (v16.11). The CLI stays
backward compatible with `build.ps1 -RunDensityBench`'s invocation shape
(`--n/--run/--out`, exit-code contract). Per frame: update timing wraps
`dispatch(Command::action_use(Melee))`; frame timing additionally covers the
read-only presentation state pass that feeds a rolling FNV-1a64 checksum over
tick, instance state, every actor's identity/stats/position/effects, scion,
and event/ground/legend counters — so the presentation-read cost owner-visible
waves pay is part of the evidence, and the checksum falls out of the timed
region honestly. Provenance comes from compiler macros (_MSC_FULL_VER etc.),
CPUID brand string, environment, and an auto-resolved git ref (flag > env
`VERDIGRIS_GIT_REF` > `git rev-parse HEAD`). `--validate` uses a strict RFC
8259-subset parser (depth-capped, surrogate-checked) and enforces schema id,
task id, scenario set, provenance completeness, determinism agreement,
percentile sanity (positivity + monotonic ordering), samples==ticks, and the
exact documented check-id set.

## Changed files

- `native/tools/entity_density_bench.cpp` (rewritten; owned path)
- `orchestration/tasks/TASK-0152-native-density-benchmark-evidence/STATUS.md`
  (claim → REVIEW_REQUESTED)
- `orchestration/tasks/TASK-0152-native-density-benchmark-evidence/README.md`
  (evidence + threshold contract documentation)
- `orchestration/tasks/TASK-0152-native-density-benchmark-evidence/captures/**`
  (6 positive captures + 5 invalid fixtures)
- `orchestration/tasks/TASK-0152-native-density-benchmark-evidence/REPORT.md`

No other path touched (verified via `git status`; forbidden paths
native/client|src|include|tests, play-native.ps1, server/, src/, CI untouched;
build artifacts stayed in gitignored `native/build/`). build.ps1 itself was
not modified (read-only for this packet); the bench was compiled with
build.ps1's exact cl flags and linked against build.ps1-produced core.obj +
seasonal.obj.

## Public interfaces added/changed

Changed (tool CLI/evidence only):
- Evidence schema `verdigris-density-evidence/2` replaces the TASK-0065 flat
  JSON shape.
- New flags: `--scenario <id>` (fixed set), `--git-ref <ref>`,
  `--validate <file>`.
- Exit codes documented: 0 pass/valid, 1 evidence failure, 2 usage.
No simulation/core/networking/client interface changed.

## Test commands + outcomes (literal transcripts)

1. Documented unit gate (`AGENTS.md`/`native/README.md`):

```
PS> powershell -NoProfile -ExecutionPolicy Bypass -File native\build.ps1 -RunTests
PASS phase-a: local seam starts for the negative control
PASS phase-a: applying every drained beat leaves the model untouched
presentation events tests: PASS
EXIT=0
```

(Full transcript green: core, networking, camera2d, session, presentation
events tests + `native legacy denylist: PASS`.)

2. Bench compile with build.ps1's exact flags (vcvars64.bat MSVC 2019 x64):

```
cl /nologo /std:c++20 /EHsc /W4 /I<root>\native\include /c <root>\native\tools\entity_density_bench.cpp /Fo<...>\entity_density_bench.obj
cl /nologo entity_density_bench.obj core.obj seasonal.obj /Fe<...>\entity_density_bench.exe
COMPILE_EXIT=0        (no warnings)
```

3. Two identical seeded runs per density (six process invocations):

```
PS> & exe --n 50   --run 1 --seed 424242    --out ...\captures\density-n50-seed424242-runA.json ; EXIT=0  (runB EXIT=0)
PS> & exe --n 500  --run 1 --seed 777       --out ...\captures\density-n500-seed777-runA.json  ; EXIT=0  (runB EXIT=0)
PS> & exe --n 1000 --run 1 --seed 42424242  --out ...\captures\density-n1000-...-runA.json     ; EXIT=0  (runB EXIT=0)

Cross-process comparison (PowerShell ConvertFrom-Json):
n=50:   checksumA=fnv1a64:3798bf673ec63b90 checksumB=same countsEqual=True
n=500:  checksumA=fnv1a64:84cf19822434d14f checksumB=same countsEqual=True
n=1000: checksumA=fnv1a64:9f2964a4df5ac069 checksumB=same countsEqual=True
reproducibleA=True reproducibleB=True for all three densities
```

Summary table (update = dispatch only; frame = dispatch + read-only
presentation state pass; nearest-rank percentiles, ms):

| n | seed | ticks/s | upd p99 | frame p50 | frame p99 |
|---|---|---|---|---|---|
| 50 | 424242 | ~25,000 | 0.0009 | 0.0384 | 0.0627 |
| 500 | 777 | ~2,400 | 0.0041–0.0062 | 0.373 | 1.12 |
| 1000 | 42424242 | ~1,230 | 0.0098–0.0101 | 0.747 | 2.12–2.24 |

All seven threshold checks pass in every capture (`all_pass: true`).

4. `--validate` positive:

```
PS> & exe --validate <each of the six capture files>
VALIDATE_EXIT=0 for all six
```

5. Malformed/incomplete evidence fails (fixtures committed under
   `captures/invalid/`):

```
garbage.json               VALIDATE_EXIT=1   ("malformed JSON (offset 1: expected object key)")
truncated.json             VALIDATE_EXIT=1
missing-provenance.json    VALIDATE_EXIT=1   (removed provenance.cpu)
repro-fail.json            VALIDATE_EXIT=1   (flipped reproducible/checksum_match/all_pass/reproducible check)
incomplete-percentiles.json VALIDATE_EXIT=1  (frame_ms.mean removed; samples 999 != ticks 1000)
```

6. Usage errors:

```
& exe --n 10 --scenario bogus --out x.json  -> EXIT=2
& exe --n 0 --out x.json                    -> EXIT=2
& exe                                       -> EXIT=2
```

7. Whitespace gate:

```
PS> git diff --check      (run against staged diff; clean, no output, exit 0)
```

## Manual verification

- Read every emitted JSON by hand (schema fields, provenance completeness,
  checksum equality, monotonic percentiles).
- Confirmed `git status --short` shows only owned paths modified/added.
- Confirmed no listener was opened; port 6500 never touched; no other
  worktree/process touched.

## Commit SHAs

- Claim commit: `5156c33e826481f6e427498b0b755f35245c3ae2`
- Revision-0 implementation commit (REVIEW input): `86f72c1cb04f21062eae16e299f3e05e8e88a70d`
- Revision-1 commit: branch tip at push (validator re-enforcement + fixtures +
  this REPORT/STATUS update).

## Deviations

- `build.ps1 -RunDensityBench` writes into the TASK-0065 captures folder;
  running it would have overwritten another task's committed evidence files,
  which this packet forbids. Instead the bench was compiled with build.ps1's
  exact compile/link commands against build.ps1's own core/seasonal objects.
  If the architect wants `-RunDensityBench` retargeted to per-task folders, a
  follow-up task owning native/build.ps1 should do it.
- Percentile "max" values are reported but intentionally not bound by the
  threshold contract (only p99 and mean are bounded), so one OS-scheduler
  spike cannot invalidate otherwise healthy evidence; the validator mirrors
  exactly this contract.

## Unresolved questions

- None blocking. (Question-worthy observation only: frame cost at N=1000 is
  dominated by the full-state read pass (~0.75 ms median), which suggests a
  future presentation wave may want incremental dirty-state reads. Out of
  scope here.)

## Risks

- Threshold bounds are hardware-independent (they derive from the simulation's
  own 50 ms fixed-step cadence), but extremely slow CI machines could fail
  them; provenance records enough to triage any such failure.

## Follow-ups

- Retarget `-RunDensityBench` output to per-task capture folders (owner of
  native/build.ps1).
- Optional JS-side parity once the browser server gains a spawn verb
  (inherited TASK-0065 note, unchanged).

## Revision 1 (REVIEW correction 1)

Reviewed commit `86f72c1cb04f21062eae16e299f3e05e8e88a70d` returned REVISE
with one correction: `--validate` had to independently re-enforce
`verdigris-density-threshold/1` from the evidence fields instead of trusting
seven recognized ids plus stored `pass: true` booleans, and bind the fixed
scenario route/action values.

Implementation (single owned source file,
`native/tools/entity_density_bench.cpp`):

- `scenario.route` must equal `route:tin:1:0` and `scenario.action` must
  equal `Melee` exactly (new `kScenarioAction` constant).
- A per-id contract row table (`kContractRows`) pins each check's operator.
- Each check's bound must equal the documented row: `scenario.n` for
  `monsters_spawned`, `scenario.ticks` for `samples_complete`, 1 for
  `reproducible`, 50 ms for the three budget checks, 20 for
  `ticks_per_sec_floor`.
- Each check's `value` must equal its source field recomputed from the
  evidence (`determinism.counts.monsters_start`, `timings.samples`,
  `determinism.reproducible`, `timings.frame_ms.p99`,
  `timings.frame_ms.mean`, `timings.update_ms.p99`,
  `timings.ticks_per_sec`), and each stored `pass` must equal the operator
  reapplied to the recomputed value/bound. Fabricated passing checks,
  loosened bounds, flipped operators, or lying pass flags now fail with a
  targeted diagnostic naming the contradicted source field.

New focused negative fixtures (all derived from the committed positive
capture `density-n500-seed777-runA.json` by `make-invalid-fixtures.ps1`,
which is committed alongside for reproducibility):

| fixture | tamper | validator diagnostic |
|---|---|---|
| tampered-check-value.json | check value 0.0011 vs source 0.0041, pass kept true | value disagrees with timings.update_ms.p99 |
| tampered-check-bound.json | ticks_per_sec_floor bound 20→10 | bound contradicts contract bound |
| tampered-check-op.json | frame_p99 op max→min | op contradicts contract op |
| tampered-check-pass.json | reproducible pass flipped to false | stored pass contradicts recomputed result |
| tampered-threshold-fail.json | frame p99 raised to 61.12 ms consistently (field + check value, percentiles monotonic) with pass kept true | stored pass contradicts recomputed result — the exact fabrication the correction forbids |
| tampered-route.json | route → route:tin:9:9 | scenario.route must be the fixed route |
| tampered-action.json | action → Sweep | scenario.action must be Melee |

Revision-1 gate reruns (all literal):

1. `powershell -NoProfile -ExecutionPolicy Bypass -File native\build.ps1
   -RunTests` → GATE_EXIT=0 (core, networking, camera2d, session, journey,
   reconnect, gate-b, render-list, presentation events + legacy denylist
   PASS; only pre-existing warnings in non-owned files).
2. Bench compile with build.ps1's exact flags (MSVC 2019 v16.11.42 vcvars64):
   `cl /nologo /std:c++20 /EHsc /W4 ...` compile+link COMPILE_EXIT=0, zero
   warnings.
3. Validator matrix: all six committed positive captures VALIDATE exit=0;
   all twelve negative fixtures (five original + seven new) exit=1 with the
   expected diagnostics (transcript above in the fixture table; originals
   unchanged).
4. Deterministic double-process probe: two fresh processes
   `--n 1000 --run 1 --seed 42424242` both exit 0, checksums
   `fnv1a64:9f2964a4df5ac069` identical to each other and to the committed
   n=1000 capture; counts equal across all three. The fresh evidence also
   passes the new stricter `--validate` (exit 0).
5. Usage errors unchanged: `--scenario bogus` → exit 2, `--n 0` → exit 2.
6. Scope check: `git status --short` shows only
   `native/tools/entity_density_bench.cpp` and
   `orchestration/tasks/TASK-0152-native-density-benchmark-evidence/**`.
   `git diff --check` clean (exit 0).

No gameplay, simulation, presentation, or other-task behavior changed; the
producer's run-mode output format is byte-identical to revision 0 (verified
by the fresh capture validating and matching the committed checksum).
