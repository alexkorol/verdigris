---
task: TASK-0138
title: Release-proof manifest validator CLI
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
dependencies: [TASK-0131 ACCEPTED]
base_commit: be6d555688619819084b352660fc0336a90d0ec3
owned_paths: [orchestration/tasks/TASK-0138-release-proof-validator/**]
forbidden_paths: [native/**, server/**, src/**, playtest/**, deployment, signing, external accounts]
promotion_provenance:
  parent_packet: TASK-0131
  dependency_event: TASK-0131 ACCEPTED and integrated
  validator: task-folder-only executable; collision clear at be6d5556
---

# Outcome

Implement dependency-free Node CLI `validate-release-proof.mjs`, tests, and
synthetic fixtures under this task folder. Validate TASK-0131 manifests against
exact head, command/exit, environment, artifact/hash, platform, rollback, and
owner-action evidence. Missing or stale proof must produce deterministic
nonzero verdicts; prose/CI labels never substitute for artifacts. No release,
build, installer, signing, deployment, credential, or external action occurs.

# Acceptance commands

```powershell
node --test orchestration/tasks/TASK-0138-release-proof-validator/validator.test.mjs
node orchestration/tasks/TASK-0138-release-proof-validator/validate-release-proof.mjs --manifest orchestration/tasks/TASK-0131-release-proof-manifest/release-proof-manifest.json --expected-head b3599c80122d09cd0685ae96830990cc5bada5cf --json
node orchestration/tasks/TASK-0138-release-proof-validator/validate-release-proof.mjs --manifest orchestration/tasks/TASK-0138-release-proof-validator/fixtures/false-green.json --expected-head be6d555688619819084b352660fc0336a90d0ec3 --json
git diff --check
git diff --name-only be6d555688619819084b352660fc0336a90d0ec3..HEAD
```

Both supplied manifests are expected non-release-ready: the accepted manifest
exits nonzero with evidence gaps, and false-green exits nonzero with at least
one precise integrity error. STOP before any external/release action.
