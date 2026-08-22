---
task: TASK-0130
title: Gate C route-decision envelope and validation contract
state: READY
packet: ARCHITECTURE
topology: INDEPENDENT
job: ARCHITECTURE
priority: P0
dependencies: [TASK-0086 ACCEPTED]
base_commit: cab50d62cb121ab6a88fa513257e645447226959
owned_paths:
  - orchestration/tasks/TASK-0130-gate-c-decision-envelope/**
forbidden_paths:
  - native/**
  - server/**
  - src/**
  - playtest/**
  - docs/product/**
  - campaign, reward, economy, risk, or balance values
promotion_provenance:
  generator: codex-pc-architect
  parent_packet: TASK-0086
  dependency_event: TASK-0086 ACCEPTED and integrated
  validator: task-folder-only schema packet; collision clear at cab50d62
---

# Outcome

Turn TASK-0086's accepted six-field audit into an exact, content-neutral wire
and validation contract for a future Gate C route decision. Deliver:

1. `gate-c-decision-envelope.json` defining version, route identity, concrete
   goal, boss/danger, expected trophy/material/item family, depth, branch
   consequence, extraction/return condition, evidence provenance, and
   completeness result;
2. `VALIDATION.md` defining deterministic error codes and ordering for missing
   fields, route-name-only input, unsupported version, contradictory depth,
   missing provenance, and owner-pending content;
3. `fixtures/negative-cases.json` with synthetic invalid envelopes and expected
   error codes for every required failure;
4. `REPORT.md` mapping each contract field to TASK-0086's AVAILABLE,
   DERIVABLE-WITHOUT-GAMEPLAY-RULES, or MISSING evidence and naming the smallest
   future owner/implementation path.

The contract must represent MISSING honestly. It must not fill concrete goal
or expected item-family content, reconcile native/browser node identity, or
choose campaign, reward, economy, risk, or balance values. A route name, tier,
or blurb alone remains invalid.

# Acceptance commands

Run from repository root and paste literal output plus exit codes in REPORT:

```powershell
node -e "const fs=require('fs');const p='orchestration/tasks/TASK-0130-gate-c-decision-envelope/gate-c-decision-envelope.json';const j=JSON.parse(fs.readFileSync(p,'utf8'));const req=['schema_version','route_identity','concrete_goal','boss_or_danger','expected_item_family','depth','branch_consequence','extraction_or_return','evidence_provenance','completeness'];for(const k of req)if(!(k in j))throw new Error('missing '+k);if(j.completeness.ready===true&&(j.concrete_goal.state==='MISSING'||j.expected_item_family.state==='MISSING'))throw new Error('false complete');console.log('gate-c envelope: PASS')"
node -e "const fs=require('fs');const p='orchestration/tasks/TASK-0130-gate-c-decision-envelope/fixtures/negative-cases.json';const j=JSON.parse(fs.readFileSync(p,'utf8'));const req=['MISSING_CONCRETE_GOAL','MISSING_BOSS_OR_DANGER','MISSING_EXPECTED_ITEM_FAMILY','MISSING_DEPTH','MISSING_BRANCH_CONSEQUENCE','MISSING_EXTRACTION_OR_RETURN','ROUTE_NAME_ONLY','UNSUPPORTED_VERSION','MISSING_PROVENANCE'];for(const k of req)if(!j.cases.some(x=>x.expected_error===k))throw new Error('missing '+k);console.log('gate-c negative fixtures: PASS')"
rg -n 'world:road:chart|world:zone:enter|nodeId|warden|trophy|depth|stairs|extract' native/src/networking.cpp native/tests/networking_tests.cpp playtest/scenarios/world-web.mjs playtest/scenarios/quest.mjs
git diff --check
git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD
```

# Stop conditions

STOP and preserve MISSING when evidence is absent. Do not turn schema examples
into owner decisions or propose product values. Stop if the accepted TASK-0086
classifications cannot be represented without modifying source or product
authority outside this task folder.
