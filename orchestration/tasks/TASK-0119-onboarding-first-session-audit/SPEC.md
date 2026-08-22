---
task: TASK-0119
title: Native onboarding and first-session journey audit
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P0
base_commit: 9bd689b4cebac0fe1f79ba54edcc9967a1a8f0d4
owner_visible_contribution: turns the first launch through first extraction into an explicit understandable owner journey
dependencies: []
owner_input_dependency: final narrative wording, names, and lore remain owner-only
owned_paths: [orchestration/tasks/TASK-0119-onboarding-first-session-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports or play-server mutation
---

# Outcome and invariants

Produce `FINDINGS.md`, `captures/first-session.json`, and a stepwise journey
matrix from launch/connect through House/Scion, controls, goal choice, combat,
loot/equip, extraction, progression, death/recovery explanation, quit/relaunch,
and error/reconnect states. Map what the client currently shows, the player
decision each step requires, evidence, friction, and smallest non-lore fix.
Preserve loose quest guidance and avoid checklist tutorial design.

# Acceptance and evidence

```powershell
rg -n "launch|connect|House|Scion|guide|tutorial|quest|goal|route|equip|extract|death|reconnect|error" native/client native/tests docs/rebuild docs/product orchestration/benchmarks
node -e "const x=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0119-onboarding-first-session-audit/captures/first-session.json','utf8')); if(!Array.isArray(x.steps)||!x.steps.length) process.exit(1); console.log('first session: PASS')"
git diff --check
git diff --name-only
```

Expected: only this folder changes. Negative control: preserve one step where
the required player decision is not currently legible. Stop before inventing
narrative copy/lore; continue with interaction and evidence contracts.
