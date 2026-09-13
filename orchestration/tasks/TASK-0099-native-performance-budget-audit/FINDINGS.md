# FINDINGS — TASK-0099 native performance budget and benchmark inventory

Lane `ox-pc-bd` · branch `worker/verdigris/pc/ox-pc-bd` · audit head
`ee9628aa` · 2026-08-23 · machine DESKTOP-TVU7OR7 (AMD Ryzen 5 1600,
Windows 10 Pro 19045, Node v22.23.2).

Machine-readable companion:
[`captures/benchmark-inventory.json`](captures/benchmark-inventory.json)
(schema `verdigris-benchmark-inventory/1`). This file is the narrative; the
JSON carries every number with its citation.

## 0. Authority and constraints

- Read-only resource capsule honored: **no server was started, no benchmark was
  re-executed, port 6500 untouched**. Every number below is cited from
  committed evidence in this repo.
- No performance budget is invented and no code is tuned. Numbers are labeled
  **OBSERVED** (with provenance) or **ABSENT**; pre-existing codified contracts
  are labeled as such. Owner hardware tiers and product budgets are explicitly
  out of scope (`owner_input_dependency`).
- Method: the literal acceptance sweep (1983 matching lines, transcript
  retained) plus targeted reads of each tool, workflow, policy, and capture it
  surfaced.

## 1. Inventory by surface

### 1.1 Native simulation — the only strong-provenance numbers in the repo

**OBSERVED — entity-density evidence v2**
(`native/tools/entity_density_bench.cpp` tool_version 2.0.0; captures under
`orchestration/tasks/TASK-0152-native-density-benchmark-evidence/captures/`).

Scenario `density-melee-contact`: enter `route:tin:1:0`, pack N monsters at
melee contact via `Simulation::spawn_monster`, drive T scripted `Melee`
dispatches at the fixed 20 Hz cadence. "Frame" = dispatch + read-only
presentation-state read (not a GDI paint). Nearest-rank percentiles.

| n | seed | ticks/s | update p99 ms | frame p50 ms | frame p99 ms (A/B) |
|---|---|---|---|---|---|
| 50 | 424242 | ~25,000 | 0.0009 | 0.0384 | 0.0627 / 0.0587 |
| 500 | 777 | ~2,400 | 0.0041–0.0062 | 0.373 | 1.1191 / 1.1226 |
| 1000 | 42424242 | 1238.46 | 0.0098–0.0101 | 0.747 | 2.1217 / 2.2373 |

Provenance is complete in every capture: git ref, MSVC 19.29, C++20,
`debug-no-ndebug`, Windows x86_64, CPU model + core count, steady-clock timer
and its 100 ns resolution. Seeded A/B runs agree on counts and fnv1a64 state
checksums. The audited machine's CPU matches the capture provenance, making
these the only native simulation timings that are cross-run comparable today.

Pre-existing threshold contract `verdigris-density-threshold/1` (not invented
here): spawn delivered, samples complete, reproducible, and frame/mean/update
p99 ≤ 50 ms against the existing `kSimulationTickMs = 50` (20 Hz) budget;
`ticks_per_sec_floor ≥ 20`. `--validate` recomputes every check from the
evidence fields rather than trusting stored pass flags.

**OBSERVED (superseded form) — TASK-0065-era bench**
(`TASK-0065-entity-density-benchmark/RESULTS.md`, 2026-08-20): medians N=50 →
1.65e6 ticks/s, N=200 → 5.41e5, N=500 → 3.11e5, N=1000 → 1.82e5; N=1000 median
p99 tick 0.0075 ms. Provenance is weaker ("Windows 10, MSVC 2019 x64", no CPU
model, no seeds/checksums) and the composition differs (~150× the v2 tick rate
because v2 includes the presentation read in a debug build). **These two forms
must never be averaged or compared.**

### 1.2 JS server

**OBSERVED — instance-generation** (`bench/instance-generation.mjs`,
`bench/baseline.json`): frozen pre-optimization baseline median 5.328 ms for
`Map.generateInstance` over a fixed 12-case matrix (Node v22.11.0, win32/x64,
2026-07-21); post-optimization six clean-process runs 0.330–0.523 ms median,
10.32×–16.21× speedup; cold-path 500-seed run 0.970 ms median. Explicit warmup
rounds; sha256 semantic digests with a double-generation determinism
self-check. Gap: baseline records no CPU model → not comparable across
machines; `--verify` gates correctness digests, never timing.

**OBSERVED — playtest scenario wall times + harness lag**
(`PLAYTEST_TIMING_LOG=1` JSONL): the fullest distribution in the repo is
TASK-0074 (Luna Mac capsule, 10 serialized suites): `gear-outcomes`
min 30,779 / median 33,731.5 / P90 34,283.9 / max 34,688 ms; suite p99
event-loop lag 22.15–22.59 ms, max 43.2–65.5 ms. Windows lanes differ
materially: TASK-0062 suite walls 165.6–186.5 s with p99 lag 32.2–43.9 ms;
TASK-0043 ten loaded runs walls 183.9–193.1 s, p99 32.16–32.23 ms; TASK-0155
wall 190.0 s. Slowest/most variable scenarios: gear-outcomes (spread up to
21.7 s), quest, session-arc, zones, respawn, loot. The adaptive guard
(`playtest/timing.mjs`) intentionally adds bounded slack (cap 1.75×) from
observed lag — TASK-0120 already ruled it "a fairness cap, not a performance
budget".

**Tool exists, no committed numbers found — playtest soak**
(`playtest/soak.mjs`, ~150 s default on :6511): survival exit-code only; no
percentile or memory output; invoked by no gate or workflow.

### 1.3 Networking

**OBSERVED — native lifecycle soak** (`native/tools/server_lifecycle_soak.cpp`
via `build.ps1 -RunServerLifecycleSoak`; two independent captures from
2026-08-21 in `TASK-0129-server-lifecycle-soak/captures/`): 100/100 cycles and
8/8 burst upgrades/logins/clean closes both runs; stop durations typically
138–143 ms (first cycle 155, max 155, total 14,037 ms per 100 cycles);
burst stop 149 ms; total duration 15,764 ms. Policy wrapper exists and is
good: TASK-0135 binds source head, requires platform evidence, forbids
retry-to-green, hard watchdogs; TASK-0140 validates bundles deterministically.
Gaps: artifact JSON carries **no os/cpu/compiler/git provenance block**
(unlike density evidence), stores raw stop lists without percentiles, measures
survival only — no latency/throughput metric exists for either stack, and the
tool is Windows-only (WS2_32).

**OBSERVED (informational only) — dual-server matrix** (TASK-0082 smoke):
scenario durations JS vs native (e.g. zones 18,236 ms vs 1,230 ms) reflect
authored waits, not engine cost; REPORT.md marks timing informational.

**Test-bound constant, not a benchmark**: Gate-B session tests keep a 400 ms
per-attempt echo window (TASK-0163); the native WS server tick loop sleeps
150 ms between session ticks (`networking.cpp:2811`). Neither is a latency
distribution.

### 1.4 Renderer

**Browser — OBSERVED** (committed RAF-callback-duration methodology, fixed
1440×1000, ≥10 s scripted workload, ≥100 samples):

| context | samples | mean ms | p95 ms | max ms | source |
|---|---|---|---|---|---|
| 0029-phase5 before | 260 | 45.015 | 129.5 | 137.8 | TASK-0029 REPORT + `before-frame-time.json` |
| 0029-phase5 after | 276 | 42.976 | 122.5 | 127.1 | TASK-0029 REPORT + `after-frame-time.json` |
| 0053 before | 198 | 65.224 | 212.0 | 271.2 | TASK-0053 REPORT |
| 0053 after | 286 | 40.910 | 114.9 | 128.4 | TASK-0053 REPORT |

No bound anywhere; machines/browsers unrecorded; change-evidence within single
tasks, never a stored baseline.

**Native — ABSENT.** No frame-time/paint benchmark exists in `native/`. The
closest proxies are not renderer cost: the density bench's frame includes a
read-only presentation-state read (~0.747 ms median at N=1000, debug build),
and staged client scenarios prove render-list content via two-run byte
equality — visual determinism, not timing. The client paints through a
memory-DC double buffer (`main.cpp:3887-3895`) that nothing measures.

### 1.5 Startup — ABSENT

No boot/cold-start/time-to-ready measurement exists for the JS server, native
server, or native client. Only indirect traces: `soak.mjs` polls ≤15 s for
first login readiness (functional, untimed); playtest walls include hermetic
boot implicitly (e.g. focused loot runs wall 5.0–10.9 s including boot,
TASK-0155). Nothing records time-to-listen or time-to-first-frame.

### 1.6 Memory — ABSENT

No RSS/heap benchmark or memory gate exists in any workspace. Related facts
that are not measurements: pm2 `max_memory_restart: '768M'` ops guard
(`ecosystem.config.cjs:20`); TASK-0097 R6 (P2) risk that native `sessions_`
never evicts identities → unbounded growth on long-lived servers. Both soak
tools watch survival, not footprint, so a slow leak passes green today.

### 1.7 Entity-density cross-reference

Fully covered in 1.1. Note the JS side has **no** `spawn_monster`/`dev:spawn`
seam, so no apples-to-apples JS density benchmark exists (TASK-0065 RESULTS.md
records this as a deliberate note).

### 1.8 Capture infrastructure

- **Side-by-side visual benchmark** (D-124,
  `orchestration/benchmarks/side-by-side-2026-08-20/`): PNG/JPG parity scenes +
  hash-verifiable reference manifest (TASK-0084). Visual, not timing.
- **Native staged captures**: five scenes × {JSON, PNG@1920×1080,
  PNG@1366×768} with mandatory two-run byte equality — a real determinism gate
  for single staged frames; multi-tick animation sequences are not yet dumped.
- **Soak evidence bundle validator** (TASK-0140, 33 tests): deterministic CLI
  judging lifecycle-soak artifacts against the accepted policy; rejects
  retry-masked failures and missing provenance.
- **Playtest timing JSONL**: per-scenario wall + suite p99/max lag records
  (96–320 rows committed under TASK-0062/TASK-0074); records lack structured
  machine fields.

## 2. Missing threshold dimensions

- **Percentiles.** Density v2 computes p50/p90/p99/max/mean but bounds only
  p99+mean at the loose 50 ms budget; max deliberately unbounded. Browser
  frame-time has mean+p95+max and no bound. Soak stop lists store no computed
  percentiles. Playtest logs p99/max informationally. No p95 exists for any
  native/server metric.
- **Hardware.** Target tiers are owner-owned (not defined here). Structured
  CPU/build provenance exists *only* in density v2 captures. Cross-machine
  pooling is already demonstrably wrong (macOS capsule p99 lag ~22 ms vs
  Windows lanes ~32 ms on the identical suite).
- **Warmup.** instance-generation warms up; density bench does not (cold spikes
  land in unbounded max); browser harness settles 3 s; soak tools have no
  settled-state concept.
- **Determinism.** Density A/B checksums and instance digests are solid; soak
  has policy-level retry rules but no state checksum; browser frame-time has no
  determinism expectation; staged captures cover single frames, not sequences.
- **Regression.** No stored-baseline comparison for any timing; no CI job runs
  any performance benchmark (`ci-native.ps1` explicitly omits the density
  bench; TASK-0120 release-gates.json confirms no frame/memory/p95 budget gate
  exists anywhere in CI or acceptance).

## 3. Negative control (required by SPEC)

**`~43ms mean frame at 1440×1000` — `orchestration/DECISIONS.md:101`.** The
decision line cites an "own measurement" with no commit SHA, date, machine,
CPU, browser version, sample count, or methodology. Candidate underlying
measurements span 40.910–45.015 ms mean across at least two tasks/commits
(TASK-0029 phase 5 before/after; TASK-0053 after) on unstated machines, so the
decision figure cannot be attributed to any specific run. Consequence: it is
anecdote until a machine-tagged capture supersedes it; do not use it as a
regression base.

Secondary examples of thin provenance: lifecycle-soak JSONs (no os/cpu/git
block), `bench/baseline.json` (no CPU model), TASK-0065 RESULTS.md (no CPU
model/seeds).

## 4. Proposed machine-tagged benchmark ladder (mechanism only)

Principle: compare only like with like; absolute budgets stay limited to
already-accepted contracts (50 ms tick budget, soak exit codes/watchdogs); any
new numeric budget is an owner decision.

- **Run tag:** `{host-id}/{cpu-short}/{build-config-or-runtime}/{git-sha7}@{utc-date}`
  e.g. `DESKTOP-TVU7OR7/R5-1600/debug-no-ndebug/ee9628a@2026-08-23`.
- **Artifact rule:** every rung writes JSON carrying the
  `verdigris-density-evidence/2`-style provenance block (git ref, compiler or
  runtime versions, build config, os/arch/cpu/cores, timer + resolution).
  Tools lacking the block (lifecycle soak, frame-time harness, timing JSONL)
  gain it before their numbers enter any ladder.
- **Rungs:**
  - **L0 commit gate (CI, windows-latest):** density bench n=500 one seed +
    `--validate` (seconds, offline, portless); `npm run bench:instance
    -- --verify` digest gate (timing columns informational);
    validate-soak-evidence self-tests. Assertions restricted to existing loose
    contracts/digests so shared-runner noise cannot flake them.
  - **L1 lane pre-review (worker capsule, owned ports):** full density ladder
    n=50/500/1000 seeds A/B; one full `npm run playtest` with
    `PLAYTEST_TIMING_LOG=1` archived to the task's `captures/`; native
    lifecycle soak (≈16 s) or `playtest/soak.mjs` ≈150 s when the respective
    surface changed.
  - **L2 integration/nightly (named stable host):** same-tag repetition ≥3 to
    expose spread; stored same-tag baselines enabling relative-delta reporting
    (delta % reported; fail thresholds set later by owner); `SOAK_SECONDS=600`.
  - **L3 owner-hardware tiers:** reserved slots; definitions and budgets are
    owner input and intentionally absent here.
- **Comparison rules:** never pool across tags; regression signal = same-tag
  relative delta vs most recent stored same-tag baseline; no retry converts a
  red to green (mirrors the accepted TASK-0135 invariant).

## 5. CI feasibility and false-green risks

Zero performance benchmarks execute in CI today; all numbers are opt-in local
evidence. Feasibility per tool:

| tool | CI-ready? | false-green risk |
|---|---|---|
| density bench v2 | yes (offline, seconds) | 50 ms bounds sit ~20× above worst observed p99 → gross regressions can pass; tighter bounds would false-red on shared runners; debug build config surprises release intuition |
| instance-generation | yes | digest verify gates semantics only; a 10× slowdown stays green if digests match |
| playtest diagnostics | runs already | high if misused as perf gate — adaptiveTimeoutMs deliberately absorbs load (1.75× cap); lag metrics measure the harness process |
| native lifecycle soak | windows runners yes | survival-only verdict passes slow leaks/degradation; artifact provenance gap weakens auditability (retry masking already rejected structurally) |
| browser frame-time | not recommended | headless software-GL timings neither bind owner-GPU behavior nor reproduce locally; false results likely in both directions |
| memory | nothing to run | absent entirely — leaks invisible to every gate; pm2 restart limit is an ops backstop, not evidence |

## 6. Observed vs future targets

- **Observed:** every value in §1 with its citation; mirrored in
  `captures/benchmark-inventory.json`.
- **Existing codified contracts (pre-existing, restated for completeness):**
  20 Hz/50 ms fixed-step budget; `verdigris-density-threshold/1` bounds;
  TASK-0135 soak watchdogs/budgets; pm2 768 M restart guard; 400 ms session
  echo test window.
- **Future targets:** none defined by this audit. Owner hardware tiers and
  product budgets remain owner decisions, per the SPEC's stop point.
