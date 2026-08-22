---
task: TASK-0150
title: Native clean-build convergence
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
dependencies: []
base_commit: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2
owned_paths: [native/build.ps1, native/CMakeLists.txt, native/CMakePresets.json, orchestration/tasks/TASK-0150-native-clean-build-convergence/**]
forbidden_paths: [native/client/**, native/src/**, native/include/**, native/tests/**, native/tools/**, server/**, src/**, CI, installers, signing]
---

# Outcome

Prove and harden a clean native build from repository sources using installed
toolchains only, including client, server, tests, and scenarios. Remove only
evidenced build-helper friction; do not alter gameplay or package/sign.

# Acceptance

From a disposable build directory, configure/build/test and run all client
scenarios with exit 0; default `native/build.ps1 -RunTests
-RunClientScenarios` remains green; `git diff --check` passes.
