---
task: TASK-0103
title: Monster, pack, rarity, and encounter gap audit
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: stages readable packs, elites, uniques, and boss encounters on the shared actor model
dependencies: []
owner_input_dependency: final monster roster, names, lore, and balance remain owner-only
owned_paths: [orchestration/tasks/TASK-0103-monster-encounter-gap-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports
---

# Outcome

Produce `FINDINGS.md` and `captures/encounter-matrix.json` mapping spawning,
pack composition, roles, aggro, rarity, equipment, unique/boss seams,
telegraphs, rewards, deterministic generation, network snapshots,
presentation, and tests. Rank content-neutral engine gaps and owner-dependent
content gaps separately.

# Frozen invariants and evidence

Monster/player stat symmetry and deterministic seeds are frozen. Cite all
claims and preserve scarcity/reward unknowns. Define scaffolding and negative
tests for proposed successor waves without authoring monsters.

# Acceptance

```powershell
rg -n "monster|pack|spawn|rarity|unique|warden|boss|aggro|telegraph|elite|role" native/include native/src native/client native/tests playtest/scenarios
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0103-monster-encounter-gap-audit/captures/encounter-matrix.json','utf8')); console.log('encounter matrix: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: identify one rarity or
pack invariant without authoritative coverage. Stop before roster/lore/balance
choices; continue isolating engine-level packets.
