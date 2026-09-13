---
task: TASK-0099
title: Native performance budget and benchmark inventory
state: SUPERSEDED
superseded_by: integrated (reviewed head a12b4999, 2026-08-23)
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: defines measurable headroom for dense ARPG combat and presentation instead of anecdotal speed
dependencies: []
owner_input_dependency: target hardware tiers remain owner-only; audit records current machine facts
owned_paths: [orchestration/tasks/TASK-0099-native-performance-budget-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no servers or long-running benchmarks
---

# Outcome

Produce `FINDINGS.md` and `captures/benchmark-inventory.json` mapping current
simulation, server, networking, renderer, startup, memory, entity-density, and
capture benchmarks; identify missing percentile, hardware, warmup, determinism,
and regression thresholds. Propose a machine-tagged benchmark ladder.

# Frozen invariants and evidence

Do not invent performance budgets or tune code. Separate observed numbers from
future targets. Cite scripts/tests/results and record machine/config provenance
for every usable number. Include CI feasibility and false-green risks.

# Acceptance

```powershell
rg -n "benchmark|duration|latency|frame|memory|density|soak|p95|p99|timing" native orchestration playtest --glob "*.md" --glob "*.json" --glob "*.mjs" --glob "*.cpp" --glob "*.ps1"
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0099-native-performance-budget-audit/captures/benchmark-inventory.json','utf8')); console.log('benchmark inventory: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: name one current number
that lacks enough configuration provenance to compare across runs. Stop before
declaring owner hardware targets; continue inventorying objective metrics.
