---
task: TASK-0102
title: Skill system and binding gap audit
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: maps the route from current actions to a coherent LMB/RMB/Q/E/R skill system
dependencies: []
owner_input_dependency: production magic and skill content remain owner-only
owned_paths: [orchestration/tasks/TASK-0102-skill-system-gap-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports
---

# Outcome

Produce `FINDINGS.md` and `captures/skill-matrix.json` mapping input slots,
authoritative skill definitions, costs/cooldowns, targeting, effects, wire
events, client model, HUD presentation, persistence, and tests. Separate
physical/action infrastructure that can proceed from magic content blocked by
OD-003.

# Frozen invariants and evidence

D-007 controls, shared actors, and server authority are frozen. Cite each
skill/action path and test. No generic wizard, name, effect, cost, or balance
may be invented. Required successors must state which paths stay owner-blocked.

# Acceptance

```powershell
rg -n "skill|primary|secondary|cooldown|cost|mana|LMB|RMB|Quickbar|keybind" native/include native/src native/client native/tests docs/product
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0102-skill-system-gap-audit/captures/skill-matrix.json','utf8')); console.log('skill matrix: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: show one Q/E/R slot or
skill field without end-to-end authority. Stop before magic/content decisions;
continue with content-neutral infrastructure mapping.
