---
task: TASK-0100
title: Deterministic replay coverage and divergence audit
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P0
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: makes regressions reproducible across core, server, persistence, and future content
dependencies: []
owner_input_dependency: none
owned_paths: [orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports
---

# Outcome

Produce `FINDINGS.md` and `captures/replay-surfaces.json` mapping commands,
seeds, ticks, clocks, RNG streams, snapshots, persistence, networking adapters,
and existing byte-equality/replay tests. Define a versioned replay record and
divergence report contract for a successor without implementing it.

# Frozen invariants and evidence

Fixed-step headless simulation and commands/events remain authoritative. Wall
clock, sockets, renderer, and persistence may not enter core determinism. Cite
every RNG/clock boundary and test. Report base SHA, risks, exact gates, and
smallest scaffold.

# Acceptance

```powershell
rg -n "seed|rng|random|tick|fixed|replay|snapshot|determin|clock|time" native/include native/src native/tests
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/captures/replay-surfaces.json','utf8')); console.log('replay surfaces: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: identify one state or
adapter input not captured by current replay proof. Stop on an authority
violation; do not patch core. Continue defining the record around proven seams.
