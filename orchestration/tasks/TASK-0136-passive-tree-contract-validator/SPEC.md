---
task: TASK-0136
title: Passive-tree contract validator CLI
state: SUPERSEDED
superseded_by: integrated (reviewed head aad6dadb, 2026-08-23)
packet: MECHANICAL
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
dependencies: [TASK-0112 ACCEPTED]
base_commit: be6d555688619819084b352660fc0336a90d0ec3
owned_paths: [orchestration/tasks/TASK-0136-passive-tree-contract-validator/**]
forbidden_paths: [native/**, server/**, src/**, playtest/**, content or balance decisions]
promotion_provenance:
  parent_packet: TASK-0112
  dependency_event: TASK-0112 ACCEPTED and integrated
  validator: task-folder-only executable; collision clear at be6d5556
---

# Outcome

Implement a dependency-free Node CLI `validate-passive-tree-contract.mjs` plus
tests and synthetic fixtures under this task folder. It validates the accepted
TASK-0112 contract and candidate graph/allocation/budget/persistence envelopes,
emits deterministic JSON errors in the ordering defined by TASK-0112, and
fails closed on all eight required error codes. It must preserve the two point
ledgers and treat the native +2/axis walk and raw snapshot as negative controls.
No authored node/effect/topology/cost/balance value is permitted.

# Acceptance commands

```powershell
node --test orchestration/tasks/TASK-0136-passive-tree-contract-validator/validator.test.mjs
node orchestration/tasks/TASK-0136-passive-tree-contract-validator/validate-passive-tree-contract.mjs --contract orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/passive-tree-contract.json --fixture orchestration/tasks/TASK-0136-passive-tree-contract-validator/fixtures/valid-synthetic.json --json
node orchestration/tasks/TASK-0136-passive-tree-contract-validator/validate-passive-tree-contract.mjs --contract orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/passive-tree-contract.json --fixture orchestration/tasks/TASK-0136-passive-tree-contract-validator/fixtures/counter-confusion.json --json
git diff --check
git diff --name-only be6d555688619819084b352660fc0336a90d0ec3..HEAD
```

The valid fixture exits 0. The counter-confusion fixture exits nonzero and emits
`COUNTER_CONFUSION`. STOP on any need to choose gameplay content or write
outside this task folder.
