---
task: TASK-0140
title: Server lifecycle soak evidence-bundle validator
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
dependencies: [TASK-0135 ACCEPTED]
base_commit: 6a10e862cc40a5aeb09694baa8d8446257df5382
owned_paths: [orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/**]
forbidden_paths: [native/**, server/**, src/**, playtest/**, .github/**, CI or machine mutation]
promotion_provenance:
  generator: codex-pc-architect
  parent_packet: TASK-0135
  dependency_event: TASK-0135 ACCEPTED and integrated
  validator: task-folder-only executable; collision clear at 6a10e862
---

# Outcome

Implement a deterministic Node CLI that evaluates a submitted lifecycle-soak
evidence bundle against the accepted TASK-0135 policy. It must fail closed on
missing or stale evidence, incomplete platform identity, forbidden ports,
nonzero/timed-out attempts, incomplete artifacts, hidden earlier failures, and
invalid PASS conclusions. It may return PASS, FAIL, or BLOCKED_ENVIRONMENTAL;
it must never run the soak, mutate CI, schedule work, contact external systems,
or decide an OWNER_PENDING release/hosting question.

Keep the CLI, tests, fixtures, validation, status, and report entirely in this
task folder. Accept the policy and bundle paths explicitly; emit one stable JSON
result to stdout and diagnostics to stderr. Exit 0 only for a policy-valid PASS
bundle, exit 1 for a deterministically rejected bundle, and exit 2 for usage,
parse, schema, or unsupported-policy errors. Include positive PASS and
BLOCKED_ENVIRONMENTAL fixtures plus negative coverage for every TASK-0135
canonical error code.

# Acceptance commands

Run from repository root and record literal output plus exit codes in REPORT:

```powershell
node --test orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/validate-soak-evidence.test.mjs
node orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/validate-soak-evidence.mjs --policy orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/soak-integration-policy.json --bundle orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/fixtures/valid-pass.json
node orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/validate-soak-evidence.mjs --policy orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/soak-integration-policy.json --bundle orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/fixtures/retry-masked-failure.json
git diff --check
git diff --name-only 6a10e862cc40a5aeb09694baa8d8446257df5382..HEAD
```

The valid fixture command must exit 0. The retry-masked negative control must
exit 1 and emit `RETRY_MASKED_FAILURE`; a zero exit is a failed gate.

# Stop conditions

STOP rather than executing a soak, binding/probing any port, editing the
accepted TASK-0135 policy, inventing missing evidence, choosing a nightly host
or release policy, modifying CI, touching credentials, or writing outside this
task folder.
