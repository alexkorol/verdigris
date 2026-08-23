---
task: TASK-0164
title: Native content seed cross-reference hardening
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
base_commit: 75ef6b7b
owner_visible_contribution: prevents authored encounter and zone seeds with dangling or contradictory references from reaching the native game pipeline
dependencies: [TASK-0151 ACCEPTED]
owner_input_dependency: none for schema consistency; authored names, balance, lore, and content selection remain owner-only
owned_paths: [native/content/validate_content.py, native/content/tests/**, orchestration/tasks/TASK-0164-native-content-cross-reference-hardening/**]
forbidden_paths: [native/content/schema.json, native/content/seeds/**, native/client/**, native/src/**, native/include/**, native/CMakeLists.txt, server/**, src/**, authored content or balance changes, everything else]
---

# Outcome

Extend the accepted native content validator so every zone/encounter seed
reference is checked as a closed, deterministic graph: unique IDs, valid
schema-version linkage, existing referenced IDs, compatible tier/type fields,
and no unreachable seeded encounter. This packet hardens the validator and
negative fixtures only. It must not rewrite the accepted seeds or invent game
content.

# Required proof

- Preserve validation of the committed positive seeds.
- Add isolated negative fixtures for duplicate IDs, missing references,
  incompatible tier/type references, unreachable encounters, and incorrect
  schema-version linkage; each must fail with a targeted diagnostic.
- Prove deterministic diagnostics and exit codes across two runs.

```powershell
python native/content/validate_content.py native/content/seeds/zones.json native/content/seeds/encounters.json
python native/content/tests/run_negative_tests.py
python native/content/tests/run_negative_tests.py
git diff --check
git diff --name-only
```

# Negative controls and STOP conditions

No accepted seed edit, authored content, lore, balance, runtime loader, CMake,
third-party dependency, client/server change, or permissive warning downgrade.
STOP if the accepted schema cannot express a required relation without an
owner/architect schema decision.
