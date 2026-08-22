---
task: TASK-0121
title: Owner art, lore, naming, balance, economy, and content approval matrix
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P1
base_commit: 9bd689b4cebac0fe1f79ba54edcc9967a1a8f0d4
owner_visible_contribution: batches every owner-only content gate without interrupting implementation or letting agents invent canon
dependencies: []
owner_input_dependency: this packet inventories decisions but does not require answers
owned_paths: [orchestration/tasks/TASK-0121-owner-content-approval-matrix/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no asset generation, external messages, or ports
---

# Outcome and invariants

Produce `FINDINGS.md` and `captures/owner-gates.json` mapping every current or
terminal owner-only decision for art/assets, lore, naming, magic, balance,
economy, campaign content, bosses/monsters/items/skills, music, season rules,
distribution, and irreversible accounts. For each, record evidence prerequisite,
critical-path deadline, recommended evidence-gathering step, at least two viable
decision classes, acceptance rubric, dependent tasks/gates, and fallback work.
Do not recommend or choose canon before evidence exists.

# Acceptance and evidence

```powershell
rg -n "owner-only|OWNER|OD-[0-9]|asset|lore|naming|balance|economy|music|season|distribution" docs/product orchestration/DECISIONS.md orchestration/owner-input orchestration/PROGRAM_GRAPH.md
node -e "const x=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0121-owner-content-approval-matrix/captures/owner-gates.json','utf8')); if(!Array.isArray(x.gates)||!x.gates.length) process.exit(1); console.log('owner gates: PASS')"
git diff --check
git diff --name-only
```

Expected: only this folder changes; no choice is falsely marked resolved.
Negative control: include at least one parked noncritical gate with executable
fallback. Stop at the evidence boundary and batch, rather than interrupting,
noncritical decisions.
