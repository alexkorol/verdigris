---
task: TASK-0097
title: Native persistence durability and fault-model audit
state: SUPERSEDED
superseded_by: integrated (reviewed head 0c373d2f, 2026-08-23)
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P0
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: protects Houses, Scions, items, and progress from crash or partial-write loss
dependencies: []
owner_input_dependency: production storage service choice is deferred; local durability audit is not blocked
owned_paths: [orchestration/tasks/TASK-0097-persistence-durability-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; do not mutate real saves
---

# Outcome

Produce `FINDINGS.md` and `captures/persistence-contract.json` mapping every
persisted field, save trigger, serialization/version seam, file location,
atomicity behavior, stale-data compatibility, reconnect semantics, and failure
mode. Define a deterministic disposable-profile fault matrix for a successor.

# Frozen invariants and evidence

D-106 and D-109 are frozen: death transforms recoverability; disconnect never
loses progress. Never touch real owner profiles. Cite source/tests for every
claim. Required report includes base/head, commands, red risks, and smallest
locking tests.

# Acceptance

```powershell
rg -n "persist|save|load|profile|serialize|version|reconnect|relic|House|Scion" native/src native/include native/tests
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0097-persistence-durability-audit/captures/persistence-contract.json','utf8')); console.log('persistence contract: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: identify one realistic
partial-write or stale-version case not covered by a current test. Stop before
modifying or opening non-disposable saves; continue source/test mapping.
