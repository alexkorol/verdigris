---
task: TASK-0105
title: Passive-tree and progression authority gap audit
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P0
base_commit: 42718fbc4340589e606fff94a6eaa3dfbd03ad1c
owner_visible_contribution: replaces the known approximation with an evidence-backed route to authoritative build progression
dependencies: []
owner_input_dependency: final tree topology, node content, and balance remain owner-only
owned_paths: [orchestration/tasks/TASK-0105-passive-tree-progression-gap-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports
---

# Outcome

Produce `FINDINGS.md` and `captures/progression-matrix.json` mapping levels,
attributes, quest-point counters, allocation commands, current hex projection,
the referenced 271-node gap, persistence/reset rules, network payloads, client
presentation, and tests. Separate authoritative behavior from approximation
and unruled content.

# Frozen invariants and evidence

Two quest-point counters and their distinct persistence semantics are frozen.
Do not ratify the +2 approximation, invent nodes, or tune progression. Cite
source/test evidence for every green row and propose a scaffold-first successor.

# Acceptance

```powershell
rg -n "passive|tree|questPoints|attribute|strength|dexterity|intelligence|allocate|271|hex" native/include native/src native/client native/tests playtest docs/rebuild
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0105-passive-tree-progression-gap-audit/captures/progression-matrix.json','utf8')); console.log('progression matrix: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: explicitly mark the
approximate formula non-authoritative. Stop before tree/content/balance choices;
continue isolating persistence, wire, and UI contracts.
