---
task: TASK-0131
title: Release-proof manifest schema
state: READY
packet: ARCHITECTURE
topology: INDEPENDENT
job: ARCHITECTURE
priority: P0
dependencies: [TASK-0120 ACCEPTED]
base_commit: cab50d62cb121ab6a88fa513257e645447226959
owned_paths: [orchestration/tasks/TASK-0131-release-proof-manifest/**]
forbidden_paths: [native/**, server/**, src/**, playtest/**, deployment actions, signing credentials]
promotion_provenance:
  parent_packet: TASK-0120
  validator: task-folder-only contract; collision clear at cab50d62
---

# Outcome

Produce `release-proof-manifest.json`, `VALIDATION.md`,
`fixtures/negative-cases.json`, and `REPORT.md`. The contract must bind exact
source head, commands/exit codes, environment identity, artifacts/hashes,
platform coverage, rollback evidence, external owner actions, and final
evidence verdict. Missing proof remains missing; CI labels and prose cannot
stand in for artifacts. No build, deployment, installer, signing, notarization,
or account action is authorized.

# Acceptance commands

```powershell
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0131-release-proof-manifest/release-proof-manifest.json','utf8'));for(const k of ['schema_version','source_head','commands','environment','artifacts','platform_coverage','rollback','owner_actions','verdict'])if(!(k in j))throw Error('missing '+k);console.log('release proof manifest: PASS')"
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0131-release-proof-manifest/fixtures/negative-cases.json','utf8'));for(const k of ['STALE_HEAD','NONZERO_EXIT','MISSING_ARTIFACT','HASH_MISMATCH','UNVERIFIED_ENVIRONMENT','MISSING_ROLLBACK','OWNER_ACTION_UNPROVEN'])if(!j.cases.some(x=>x.expected_error===k))throw Error('missing '+k);console.log('release proof negatives: PASS')"
rg -n 'release|artifact|installer|sign|notari|rollback|deploy|workflow_dispatch' .github native docs orchestration -g '*.yml' -g '*.yaml' -g '*.md' -g '*.ps1'
git diff --check
git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD
```

# Stop conditions

STOP rather than asserting release readiness, performing external actions, or
inventing signing/distribution evidence. Write only inside this task folder.
