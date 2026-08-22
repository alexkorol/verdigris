---
task: TASK-0096
title: Campaign and zone-graph measurement audit
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: measures the current route graph against the eventual 6-30 hour campaign without inventing campaign content
dependencies: []
owner_input_dependency: acts, branch density, naming, lore, and final pacing remain owner-only
owned_paths: [orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports
---

# Outcome

Produce `FINDINGS.md` and `captures/graph.json` describing current nodes,
edges, gates, branches, return paths, deterministic IDs, House-owned unlocks,
shortest/longest traversals, and missing authoring information. Map the delta
to campaign, optional branches, repeatable endgame, and fast travel seams.

# Frozen invariants and evidence

Do not invent zones, acts, rewards, duration, or travel risk. Cite source and
tests for every node/edge. Required evidence includes base SHA, graph JSON,
commands, unresolved rows, and a successor tool contract.

# Acceptance

```powershell
rg -n "road|route|node|branch|warden|waymark|stairs|extract|campaign" native/src native/include native/tests playtest/scenarios/world-web.mjs playtest/scenarios/quest.mjs
node -e "const g=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/captures/graph.json','utf8')); if(!g.nodes||!g.edges) process.exit(1); console.log('campaign graph: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: preserve at least one
MISSING campaign field rather than deriving it from a route name. Stop on an
owner-only campaign choice; continue measuring topology while it is pending.
