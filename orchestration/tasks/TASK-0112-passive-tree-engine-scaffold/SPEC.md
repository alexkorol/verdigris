---
task: TASK-0112
title: Versioned passive-tree authority schema and validation contract
state: READY
packet: ARCHITECTURE
topology: INDEPENDENT
job: ARCHITECTURE
priority: P0
dependencies: [TASK-0105 ACCEPTED]
base_commit: cab50d62cb121ab6a88fa513257e645447226959
owned_paths:
  - orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/**
forbidden_paths:
  - native/**
  - server/**
  - src/**
  - playtest/**
  - docs/product/**
  - invented nodes, effects, topology, economy, or balance
promotion_provenance:
  generator: codex-pc-architect
  parent_packet: TASK-0105
  dependency_event: TASK-0105 ACCEPTED and integrated
  validator: schema-only fallback selected; owned-path collision clear at cab50d62
---

# Outcome

Produce a content-neutral, machine-readable contract for the future native
passive-tree authority without selecting topology, nodes, effects, economy, or
balance. Deliver:

1. `passive-tree-contract.json` defining versioned graph, node, edge,
   allocation, budget, validation-result, migration, and persistence envelopes;
2. `VALIDATION.md` defining deterministic error codes and ordering for unknown
   graph version, unknown node, duplicate node, disconnected allocation,
   overspend, malformed edge, counter confusion, and unsupported migration;
3. `fixtures/negative-cases.json` containing content-neutral invalid shapes and
   expected error codes, including the native raw-snapshot trust gap identified
   by TASK-0105;
4. `REPORT.md` mapping every field and error to current browser authority,
   current native approximation, or explicitly owner-pending content.

The two point sources remain structurally distinct: persistent commission-chain
`quests.questPoints` and the live tree-budget counter may not be collapsed or
given new semantics. The native +2/axis walk remains a named negative control,
never an accepted authority. Schema identifiers and placeholder fixture values
must be obviously synthetic and carry no player-facing content.

# Acceptance commands

Run from repository root and paste literal output plus exit codes in REPORT:

```powershell
node -e "const fs=require('fs');const p='orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/passive-tree-contract.json';const j=JSON.parse(fs.readFileSync(p,'utf8'));const req=['schema_version','graph','allocation','budget','validation_result','migration','persistence'];for(const k of req)if(!(k in j))throw new Error('missing '+k);if(j.budget.persistent_commission_points===j.budget.live_tree_points)throw new Error('counters collapsed');console.log('passive-tree contract: PASS')"
node -e "const fs=require('fs');const p='orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/fixtures/negative-cases.json';const j=JSON.parse(fs.readFileSync(p,'utf8'));const req=['UNKNOWN_GRAPH_VERSION','UNKNOWN_NODE','DUPLICATE_NODE','DISCONNECTED_ALLOCATION','OVERSPENT','MALFORMED_EDGE','COUNTER_CONFUSION','UNSUPPORTED_MIGRATION'];for(const k of req)if(!j.cases.some(x=>x.expected_error===k))throw new Error('missing '+k);console.log('passive-tree negative fixtures: PASS')"
rg -n 'resolveVerdigrisTree|validateSnapshot|questPoints|live_tree_points|player:skilltree:save|\+2|STUB NOTE' server/core/passives/verdigris-authority.js server/game/verdigris-skill-tree.js native/src/networking.cpp native/include/verdigris/networking.hpp
git diff --check
git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD
```

# Stop conditions

STOP rather than choosing player-visible topology, authored nodes/effects,
point awards/caps, migration outcomes, or balance. Mark such fields
`OWNER_PENDING` with the smallest future owner path. Stop if evidence requires
writing outside this task folder or if current source contradicts TASK-0105's
accepted separation of the two counters.
