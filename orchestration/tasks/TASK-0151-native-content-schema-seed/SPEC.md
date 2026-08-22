---
task: TASK-0151
title: Native content schema seed
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
dependencies: []
base_commit: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2
owned_paths: [native/content/**, orchestration/tasks/TASK-0151-native-content-schema-seed/**]
forbidden_paths: [native/client/**, native/src/**, native/include/**, native/tests/**, native/tools/**, server/**, src/**, CI, production lore, balance]
---

# Outcome

Replace the prose-only native content placeholder with a deterministic,
versioned, content-neutral schema and validator for zones, encounters, and
visual-role references. Seed only synthetic/example data and existing accepted
identifiers; no production lore or balance.

# Acceptance

Validator accepts the committed seed, rejects unknown roles/duplicate IDs and
invalid graph references, emits deterministic diagnostics, and runs through a
documented dependency-free command; `git diff --check` passes.
