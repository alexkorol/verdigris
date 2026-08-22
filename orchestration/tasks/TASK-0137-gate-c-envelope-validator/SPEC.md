---
task: TASK-0137
title: Gate C envelope validator CLI
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
dependencies: [TASK-0130 ACCEPTED]
base_commit: be6d555688619819084b352660fc0336a90d0ec3
owned_paths: [orchestration/tasks/TASK-0137-gate-c-envelope-validator/**]
forbidden_paths: [native/**, server/**, src/**, playtest/**, campaign or reward values]
promotion_provenance:
  parent_packet: TASK-0130
  dependency_event: TASK-0130 ACCEPTED and integrated
  validator: task-folder-only executable; collision clear at be6d5556
---

# Outcome

Implement dependency-free Node CLI `validate-gate-c-envelope.mjs`, tests, and
synthetic fixtures under this task folder. Validate the accepted TASK-0130
envelope with deterministic error ordering, reject route-name-only input,
unsupported versions, missing provenance, and every missing decision field.
The CLI must preserve honest `MISSING` and `OWNER_PENDING` values and can never
turn them into invented campaign/reward content.

# Acceptance commands

```powershell
node --test orchestration/tasks/TASK-0137-gate-c-envelope-validator/validator.test.mjs
node orchestration/tasks/TASK-0137-gate-c-envelope-validator/validate-gate-c-envelope.mjs --schema orchestration/tasks/TASK-0130-gate-c-decision-envelope/gate-c-decision-envelope.json --fixture orchestration/tasks/TASK-0137-gate-c-envelope-validator/fixtures/valid-incomplete.json --json
node orchestration/tasks/TASK-0137-gate-c-envelope-validator/validate-gate-c-envelope.mjs --schema orchestration/tasks/TASK-0130-gate-c-decision-envelope/gate-c-decision-envelope.json --fixture orchestration/tasks/TASK-0137-gate-c-envelope-validator/fixtures/route-name-only.json --json
git diff --check
git diff --name-only be6d555688619819084b352660fc0336a90d0ec3..HEAD
```

The valid-but-incomplete fixture exits 0 with `ready:false`; route-name-only
exits nonzero with `ROUTE_NAME_ONLY`. STOP before product values or source edits.
