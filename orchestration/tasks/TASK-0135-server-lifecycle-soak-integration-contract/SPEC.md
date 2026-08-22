---
task: TASK-0135
title: Server lifecycle soak integration policy contract
state: READY
packet: ARCHITECTURE
topology: INDEPENDENT
job: ARCHITECTURE
priority: P1
dependencies: [TASK-0129 ACCEPTED]
base_commit: eef04840465f8b3e0400be182ff29506a520eb60
owned_paths: [orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/**]
forbidden_paths: [native/**, server/**, src/**, playtest/**, .github/**, CI or machine mutation]
promotion_provenance:
  generator: codex-pc-architect
  parent_packet: TASK-0129
  dependency_event: TASK-0129 ACCEPTED and integrated
  validator: task-folder-only contract; collision clear at eef04840
---

# Outcome

Produce `soak-integration-policy.json`, `VALIDATION.md`,
`fixtures/negative-cases.json`, and `REPORT.md`. Define a content-neutral,
machine-readable policy for when the accepted TASK-0129 lifecycle soak should
run in local, pre-merge, nightly, and release-proof contexts. Bind source head,
platform, port capsule, repetition count, timeout, artifact retention,
quarantine, retry, escalation, and verdict evidence without changing CI or
declaring a release policy on the owner's behalf.

The policy must distinguish deterministic failure from environmental port
contention and must never convert a failed or missing soak into green by retry.
Port 6500 remains forbidden. All schedule/cost choices that require owner or
hosting authority remain `OWNER_PENDING` with the smallest future owner path.

# Acceptance commands

Run from repository root and paste literal output plus exit codes in REPORT:

```powershell
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/soak-integration-policy.json','utf8'));for(const k of ['schema_version','contexts','source_head','platform','port_capsule','repetition','timeout','artifacts','retry','quarantine','escalation','verdict'])if(!(k in j))throw Error('missing '+k);if(j.port_capsule.forbidden_ports.indexOf(6500)<0)throw Error('6500 not forbidden');console.log('soak integration policy: PASS')"
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/fixtures/negative-cases.json','utf8'));for(const k of ['STALE_SOURCE_HEAD','MISSING_PLATFORM_EVIDENCE','PORT_CAPSULE_COLLISION','FORBIDDEN_PORT_6500','TIMEOUT','NONZERO_SOAK','MISSING_ARTIFACT','RETRY_MASKED_FAILURE'])if(!j.cases.some(x=>x.expected_error===k))throw Error('missing '+k);console.log('soak policy negatives: PASS')"
rg -n 'RunServerLifecycleSoak|server_lifecycle_soak|nightly|workflow_dispatch|timeout|artifact|quarantine|retry|6500' native .github orchestration -g '*.ps1' -g '*.cpp' -g '*.yml' -g '*.yaml' -g '*.md' -g '*.json'
git diff --check
git diff --name-only eef04840465f8b3e0400be182ff29506a520eb60..HEAD
```

# Stop conditions

STOP rather than modifying CI, selecting paid runner/schedule policy, weakening
a failure into warning, touching credentials, or writing outside this task
folder. Preserve unsupported platform and release claims as `OWNER_PENDING`.
