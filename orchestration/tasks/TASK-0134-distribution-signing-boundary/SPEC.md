---
task: TASK-0134
title: Distribution, signing, and owner-action boundary contract
state: READY
packet: ARCHITECTURE
topology: INDEPENDENT
job: ARCHITECTURE
priority: P1
dependencies: [TASK-0120 ACCEPTED]
base_commit: cab50d62cb121ab6a88fa513257e645447226959
owned_paths: [orchestration/tasks/TASK-0134-distribution-signing-boundary/**]
forbidden_paths: [native/**, server/**, src/**, playtest/**, credentials, external accounts, release publication]
promotion_provenance:
  parent_packet: TASK-0120
  validator: task-folder-only contract; collision clear at cab50d62
---

# Outcome

Produce `distribution-boundary.json`, `VALIDATION.md`,
`fixtures/negative-cases.json`, and `REPORT.md`. Separate machine-verifiable
artifact/hash/installer/update/rollback evidence from owner-only certificate,
signing, notarization, store, account, pricing, and publication actions. Define
handoff inputs/outputs without acquiring credentials or contacting services.

# Acceptance commands

```powershell
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0134-distribution-signing-boundary/distribution-boundary.json','utf8'));for(const k of ['schema_version','artifacts','hashes','installer','update','rollback','machine_actions','owner_actions','handoff'])if(!(k in j))throw Error('missing '+k);console.log('distribution boundary: PASS')"
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0134-distribution-signing-boundary/fixtures/negative-cases.json','utf8'));for(const k of ['MISSING_ARTIFACT','HASH_MISMATCH','UNSIGNED_ARTIFACT','MISSING_LICENSE','OWNER_CREDENTIAL_REQUIRED','NOTARIZATION_UNPROVEN','ROLLBACK_UNPROVEN'])if(!j.cases.some(x=>x.expected_error===k))throw Error('missing '+k);console.log('distribution negatives: PASS')"
rg -n 'installer|sign|certificate|notari|distribution|publish|release|license|rollback|update' .github native docs orchestration -g '*.yml' -g '*.yaml' -g '*.md' -g '*.ps1'
git diff --check
git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD
```

# Stop conditions

STOP before credentials, account actions, publication, legal/license rulings,
or claims that signing/notarization occurred.
