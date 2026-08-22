---
task: TASK-0101
title: Combat depth and feel gap audit
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: converts the post-parity combat backlog into measurable, player-visible waves
dependencies: []
owner_input_dependency: balance and new action design remain owner-only
owned_paths: [orchestration/tasks/TASK-0101-combat-depth-gap-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no play server
---

# Outcome

Produce `FINDINGS.md` and `captures/combat-matrix.json` comparing constitution
combat vocabulary with native authoritative actions, enemy responses,
telegraphs, impact feedback, equipment effects, tests, and presentation. Rank
gaps by owner-visible value and dependency; distinguish missing mechanics from
feel tuning and from owner-only design.

# Frozen invariants and evidence

Actor symmetry, one damage pipeline, D-114 coherence, and D-115 play gate are
frozen. Cite implementation/test/presentation evidence. Do not propose numeric
retunes or novel skill designs. Include exact locking tests for each successor.

# Acceptance

```powershell
rg -n "attack|damage|hit|telegraph|dodge|dash|guard|slam|thrust|combo|war cry|effect" native/include native/src native/client native/tests docs/product/VERDIGRIS_CONSTITUTION.md
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0101-combat-depth-gap-audit/captures/combat-matrix.json','utf8')); console.log('combat matrix: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: mark one constitution
action family absent rather than treating a generic attack as parity. Stop at
balance/new-design choices; continue sequencing proven gaps.
