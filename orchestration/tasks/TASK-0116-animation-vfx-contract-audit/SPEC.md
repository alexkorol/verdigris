---
task: TASK-0116
title: Native animation and visual-effects contract audit
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P1
base_commit: 9bd689b4cebac0fe1f79ba54edcc9967a1a8f0d4
owner_visible_contribution: maps the path from placeholder motion/effects to readable production combat animation and VFX
dependencies: []
owner_input_dependency: final art style and authored assets remain owner-only; contract audit proceeds
owned_paths: [orchestration/tasks/TASK-0116-animation-vfx-contract-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports, asset generation, or source edits
---

# Outcome and invariants

Produce `FINDINGS.md` and `captures/animation-vfx-matrix.json` mapping current
actor facing, movement, attack/telegraph/hit/death/dodge timing, render-list
events, particles/auras/orbs, layering, camera response, deterministic capture,
and tests. Separate simulation-authored timing/events from presentation-only
interpolation and authored asset needs. Commands/events authority, D-114 timing
coherence, render-list determinism, and D-115 play verdicts are frozen.

# Acceptance and evidence

Every row cites source/test/capture evidence and classifies COMPLETE, PARTIAL,
MISSING, or OWNER-ASSET. Include locking tests and phased successor boundaries.

```powershell
rg -n "animation|frame|facing|swing|telegraph|impact|death|dash|effect|particle|aura|orb|camera" native/client native/include native/src native/tests orchestration/benchmarks
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0116-animation-vfx-contract-audit/captures/animation-vfx-matrix.json','utf8')); console.log('animation/VFX matrix: PASS')"
git diff --check
git diff --name-only
```

Expected: only this folder changes. Negative control: document at least one
combat event with no proved visual timing/capture. Stop before choosing or
generating production art; continue contract/test decomposition while pending.
