---
task: TASK-0095
title: Native content and asset-authoring schema audit
state: SUPERSEDED
superseded_by: integrated (reviewed head d902c861, 2026-08-23)
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: exposes the smallest safe tooling path for adding zones, monsters, items, and encounters at scale
dependencies: []
owner_input_dependency: naming, lore, balance, and final content choices remain owner-only
owned_paths: [orchestration/tasks/TASK-0095-content-authoring-schema-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no generators or servers
---

# Outcome

Produce `FINDINGS.md` and `captures/content-surfaces.json` mapping where zone,
layout, actor, skill, item, trophy, quest, and presentation content is authored
today, which parts are code-bound, validation gaps, stable IDs, deterministic
seed boundaries, and a proposed versioned schema/tool pipeline. No schema or
content is implemented.

# Frozen invariants and evidence

Simulation authority, seeded generation, legacy firewall, and existing wire
IDs are frozen. Cite every authoring surface and consumer. Separate mechanical
schema needs from owner-only content. Include locking validators and migration
risks in the successor proposal.

# Acceptance

```powershell
rg -n "catalog|theme|zone|monster|skill|item|trophy|quest|seed" native/content native/src native/include server --glob "*.cpp" --glob "*.hpp" --glob "*.js" --glob "*.json"
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0095-content-authoring-schema-audit/captures/content-surfaces.json','utf8')); console.log('content surfaces: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: name one code-bound
content field that cannot be safely externalized without a locking test. Stop
before choosing names, lore, drops, or balance; continue with schema-neutral
validation design.
