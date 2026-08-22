---
task: TASK-0132
title: Clean-machine execution harness contract
state: READY
packet: ARCHITECTURE
topology: INDEPENDENT
job: ARCHITECTURE
priority: P0
dependencies: [TASK-0120 ACCEPTED]
base_commit: cab50d62cb121ab6a88fa513257e645447226959
owned_paths: [orchestration/tasks/TASK-0132-clean-machine-harness-contract/**]
forbidden_paths: [native/**, server/**, src/**, playtest/**, CI or machine mutation]
promotion_provenance:
  parent_packet: TASK-0120
  validator: task-folder-only contract; collision clear at cab50d62
---

# Outcome

Produce `clean-machine-contract.json`, `VALIDATION.md`,
`fixtures/negative-cases.json`, and `REPORT.md`. Define deterministic checkout,
dependency, toolchain, build, test, launch, smoke, cleanup, artifact, cache,
process, port, and platform evidence stages for a future disposable-host
harness. The contract must distinguish cached developer success from a clean
machine and must keep port 6500 forbidden.

# Acceptance commands

```powershell
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0132-clean-machine-harness-contract/clean-machine-contract.json','utf8'));for(const k of ['schema_version','checkout','toolchain','dependencies','build','tests','launch','smoke','cleanup','artifacts','platform_matrix'])if(!(k in j))throw Error('missing '+k);console.log('clean-machine contract: PASS')"
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0132-clean-machine-harness-contract/fixtures/negative-cases.json','utf8'));for(const k of ['DIRTY_BASE','CACHE_LEAK','MISSING_TOOLCHAIN','DEPENDENCY_DRIFT','NONZERO_STAGE','LEAKED_PROCESS','NON_LOOPBACK_BIND','FORBIDDEN_PORT_6500'])if(!j.cases.some(x=>x.expected_error===k))throw Error('missing '+k);console.log('clean-machine negatives: PASS')"
rg -n 'npm ci|cmake|build.ps1|RunTests|RunClientScenarios|playtest|smoke|6500|clean.machine|artifact' .github native docs package.json orchestration -g '*.yml' -g '*.yaml' -g '*.md' -g '*.ps1' -g '*.json'
git diff --check
git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD
```

# Stop conditions

STOP rather than installing system software, modifying CI, launching shared
services, or claiming a platform is covered without durable evidence.
