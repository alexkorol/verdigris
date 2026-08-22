---
task: TASK-0139
title: Clean-machine evidence manifest validator CLI
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
dependencies: [TASK-0132 ACCEPTED]
base_commit: be6d555688619819084b352660fc0336a90d0ec3
owned_paths: [orchestration/tasks/TASK-0139-clean-machine-manifest-validator/**]
forbidden_paths: [native/**, server/**, src/**, playtest/**, .github/**, CI or machine mutation]
promotion_provenance:
  parent_packet: TASK-0132
  dependency_event: TASK-0132 ACCEPTED and integrated
  validator: task-folder-only executable; collision clear at be6d5556
---

# Outcome

Implement dependency-free Node CLI `validate-clean-machine-evidence.mjs`, tests,
and synthetic fixtures under this task folder. Validate evidence manifests
against the accepted TASK-0132 stage contract: pinned clean checkout, declared
toolchain/dependencies, cache provenance, green build/tests/smoke, complete
cleanup, no leaked processes, loopback capsule only, and no port 6500 contact.
The tool validates evidence only; it does not provision or mutate a machine/CI.

# Acceptance commands

```powershell
node --test orchestration/tasks/TASK-0139-clean-machine-manifest-validator/validator.test.mjs
node orchestration/tasks/TASK-0139-clean-machine-manifest-validator/validate-clean-machine-evidence.mjs --contract orchestration/tasks/TASK-0132-clean-machine-harness-contract/clean-machine-contract.json --fixture orchestration/tasks/TASK-0139-clean-machine-manifest-validator/fixtures/valid-synthetic.json --json
node orchestration/tasks/TASK-0139-clean-machine-manifest-validator/validate-clean-machine-evidence.mjs --contract orchestration/tasks/TASK-0132-clean-machine-harness-contract/clean-machine-contract.json --fixture orchestration/tasks/TASK-0139-clean-machine-manifest-validator/fixtures/forbidden-port.json --json
git diff --check
git diff --name-only be6d555688619819084b352660fc0336a90d0ec3..HEAD
```

The valid synthetic fixture exits 0. The forbidden-port fixture exits nonzero
with `FORBIDDEN_PORT_6500`. STOP before provisioning, CI edits, or source edits.
