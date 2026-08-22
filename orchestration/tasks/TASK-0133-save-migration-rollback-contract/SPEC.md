---
task: TASK-0133
title: Save migration and rollback evidence contract
state: READY
packet: ARCHITECTURE
topology: INDEPENDENT
job: ARCHITECTURE
priority: P0
dependencies: [TASK-0120 ACCEPTED]
base_commit: cab50d62cb121ab6a88fa513257e645447226959
owned_paths: [orchestration/tasks/TASK-0133-save-migration-rollback-contract/**]
forbidden_paths: [native/**, server/**, src/**, playtest/**, persistent user data]
promotion_provenance:
  parent_packet: TASK-0120
  validator: task-folder-only contract; collision clear at cab50d62
---

# Outcome

Produce `save-migration-contract.json`, `VALIDATION.md`,
`fixtures/negative-cases.json`, and `REPORT.md`. Define version detection,
preflight, backup, migration, verification, idempotence, rollback, failure
isolation, data-loss detection, and evidence envelopes. Map known browser/native
persistence seams without choosing unresolved legacy mappings or touching any
real profile/database.

# Acceptance commands

```powershell
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0133-save-migration-rollback-contract/save-migration-contract.json','utf8'));for(const k of ['schema_version','source_version','target_version','preflight','backup','migration','verification','idempotence','rollback','evidence'])if(!(k in j))throw Error('missing '+k);console.log('save migration contract: PASS')"
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0133-save-migration-rollback-contract/fixtures/negative-cases.json','utf8'));for(const k of ['UNKNOWN_SOURCE_VERSION','BACKUP_FAILED','MIGRATION_FAILED','VERIFY_FAILED','NON_IDEMPOTENT','ROLLBACK_FAILED','DATA_LOSS'])if(!j.cases.some(x=>x.expected_error===k))throw Error('missing '+k);console.log('save migration negatives: PASS')"
rg -n 'schemaVersion|migration|rollback|backup|SQLite|persist|save|guest-save-store|profile' native server docs playtest orchestration -g '*.cpp' -g '*.hpp' -g '*.js' -g '*.mjs' -g '*.md'
git diff --check
git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD
```

# Stop conditions

STOP before mutating persistent data, selecting owner-only compatibility
policy, or treating an untested mapping as reversible.
