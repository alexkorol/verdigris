---
task: TASK-0128
title: Accepted-throughput normalization and runway evidence
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: MECHANICAL
priority: P0
base_commit: 10740898ee967bbff3025737ffc895480e20545c
owner_visible_contribution: replaces guessed queue sufficiency with reproducible accepted-throughput and autonomous-runway evidence
dependencies: []
owner_input_dependency: none; missing provider/model/harness telemetry remains unknown rather than guessed
owned_paths:
  - orchestration/throughput/**
  - orchestration/tasks/TASK-0128-accepted-throughput-normalization/**
forbidden_paths:
  - native/**
  - server/**
  - src/**
  - playtest/**
  - orchestration/tasks/* except this task
  - orchestration/backlog-factory/generated/**
resource_capsule: read-only Git/task evidence; Node.js 22; no server or ports
---

# Outcome

Implement a deterministic, dependency-free Node.js 22 collector that turns
explicit accepted task evidence and read-only Git history into normalized
throughput observations and an honest autonomous-runway snapshot. It must make
D-128 measurable without treating missing telemetry as zero, aggregating unlike
experimental units, or inventing human estimates.

The reusable entry point is `orchestration/throughput/collect.mjs`. Its
committed evidence for this task is:

- `captures/throughput-observations.json`;
- `captures/runway-snapshot.json`;
- `REPORT.md` and the worker-owned `STATUS.md`.

# Frozen interfaces and rules

## Observation schema

Each accepted/integrated observation records, when explicitly evidenced:

- task id and title;
- accepted/integrated revision;
- claim/start and acceptance/end timestamps;
- timestamp source (`task_explicit`, `git_authored`, or missing) and confidence;
- wall-clock duration only when both ordered endpoints exist;
- endpoint, provider, model id/alias, harness, harness version, configuration
  provenance, machine, task family, packet type, and job type;
- base/head SHAs, review cycles, first-pass outcome, false-green state, changed
  test/assertion disclosure, and accepted owner-visible result;
- source file/commit provenance for every non-null field.

Unknown values are JSON `null`, never `0`, empty strings, aliases promoted to
facts, or inferred providers. The current `opencode/x-preview-f-free` claims
remain provider-unconfirmed unless authoritative evidence says otherwise.

## Aggregation key

Aggregate only observations with an identical complete experimental unit:

`endpoint × provider × model id × harness × harness version × configuration × machine × task family × packet type`.

Incomplete units remain individual observations and cannot produce a throughput
rate. Rates use accepted outcomes only and report sample count, observation
window, tasks/hour, duration distribution, and provenance. No cross-unit pooled
headline is allowed.

## Runway snapshot

Read the canonical effective READY table and D-128 generated reserve summary.
For each currently registered lane, select a rate only from its exact complete
experimental unit and compatible packet type. If no comparable accepted sample
exists, emit `hours: null`, confidence `UNKNOWN`, and the missing dimensions.
Do not use historical fleet averages or the owner directive's illustrative
numbers as a substitute.

The snapshot includes target/warning/critical thresholds 72/48/24, READY and
AUTO_RELEASE counts, compatible packet counts by lane, observed rates, and a
deterministic overall state. `UNKNOWN` is non-green and cannot be reported as
72-hour coverage.

# CLI contract

```text
node orchestration/throughput/collect.mjs --repo <root> --out <task-captures>
node orchestration/throughput/collect.mjs --repo <root> --out <task-captures> --check
```

- normal mode writes canonical stable-order JSON;
- `--check` is read-only and fails if outputs are missing/stale or validation
  fails;
- repeated runs at the same Git revision are byte-identical;
- paths resolve inside the supplied repository root and reject escape;
- Git/task parsing failure is explicit and nonzero; no managed state mutates.

# Tests and fixtures

Under `orchestration/throughput/fixtures/**`, include at least:

1. complete comparable accepted observations aggregate correctly;
2. one changed dimension prevents aggregation;
3. missing provider/model/harness/version/configuration/machine remains null and
   produces UNKNOWN runway;
4. zero/missing duration is rejected rather than converted to infinite/zero
   throughput;
5. REVISE then ACCEPTED records review cycles and not-first-pass;
6. claim without acceptance is excluded from accepted throughput;
7. provider/model alias disagreement remains unconfirmed;
8. stale output makes `--check` fail;
9. path escape and malformed task evidence fail closed;
10. identical inputs produce byte-identical outputs.

# Acceptance commands

Run exactly from the canonical task worktree and paste literal output plus exit
codes into `REPORT.md`:

```powershell
node orchestration/throughput/collect.mjs --repo . --out orchestration/tasks/TASK-0128-accepted-throughput-normalization/captures
node orchestration/throughput/collect.mjs --repo . --out orchestration/tasks/TASK-0128-accepted-throughput-normalization/captures --check
node --test orchestration/throughput/*.test.mjs
node -e "const fs=require('fs'); const o=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0128-accepted-throughput-normalization/captures/throughput-observations.json','utf8')); const r=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0128-accepted-throughput-normalization/captures/runway-snapshot.json','utf8')); if(!Array.isArray(o.observations)||!o.schema_version) throw Error('bad observations'); if(r.thresholds.target_hours!==72||r.thresholds.warning_below_hours!==48||r.thresholds.critical_below_hours!==24) throw Error('bad thresholds'); if(r.hours===0 && r.confidence==='UNKNOWN') throw Error('unknown encoded as zero'); console.log('throughput/runway schema: PASS')"
git diff --check
git diff --name-only 10740898ee967bbff3025737ffc895480e20545c...HEAD
```

Expected: deterministic collector/check/test/schema gates pass; changed paths
stay inside ownership; the current Ox lane remains UNKNOWN until a comparable
accepted sample exists. Negative control: deliberately alter a copied fixture
output and show `--check` fails without rewriting it.

# Stop conditions

Stop rather than: mutating Git/task evidence; guessing provider/model identity;
pooling unlike units; attaching human estimates; editing backlog-factory
generated manifests; changing acceptance history; following a path outside the
repo; or weakening a fixture to make the collector pass. Questions go in this
task folder. Push only the worker branch and request independent review.
