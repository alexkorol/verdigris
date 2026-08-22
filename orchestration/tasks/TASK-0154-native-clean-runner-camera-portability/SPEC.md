---
task: TASK-0154
title: Native clean-runner camera-test portability hotfix
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 3933c366d8b6205e74a588634698627786e66767
owner_visible_contribution: restores a trustworthy green protected release without weakening the camera contract
dependencies: [TASK-0050, TASK-0150]
owner_input_dependency: none
owned_paths: [native/tests/camera2d_tests.cpp, orchestration/tasks/TASK-0154-native-clean-runner-camera-portability/**]
forbidden_paths: [native/client/**, native/src/**, native/include/**, native/CMakeLists.txt, native/build.ps1, .github/**, server/**, src/**, everything else]
resource_capsule: compile-and-test only; no live server and no port allocation required
---

# Incident and outcome

The protected `master` release at
`db3fc0467ed0cc978f5152aeec558208825bd0af` passed the local Visual Studio
2019 release gate, but GitHub Native run `32577972059` failed on the current
MSVC 19.51 clean runner while compiling `native/tests/camera2d_tests.cpp:64`.
The first diagnostic is `C3312` (no callable `begin`/`end` for the braced
range); subsequent syntax and undeclared-variable diagnostics are cascading.

Make the camera contract test standards-correct and portable across both the
repository's local MSVC toolchain and the current clean GitHub runner. Preserve
all zoom cases and assertions. This is a test portability hotfix, not a camera
behavior change.

# Required implementation

- Diagnose the missing direct standard-library dependency or equivalent
  portability defect at the range-for zoom cases.
- Apply the smallest explicit, standards-correct source fix in the owned test
  file.
- Do not alter production camera behavior, test values, assertions, CMake,
  workflows, warning policy, or CI conditions.
- Record the exact clean-runner failure and the local verification transcript
  in REPORT.md. Commit and push only the worker branch.

# Acceptance and evidence

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
git diff --check
git diff --name-only 3933c366d8b6205e74a588634698627786e66767...HEAD
```

Expected: the full native gate exits zero, every existing camera zoom case is
still compiled and executed, and only the two owned path families change.
Set STATUS.md to `REVIEW_REQUESTED` with the pushed implementation head.

# Negative controls

- Never remove, skip, condition, or weaken `camera2d_tests`.
- Never modify `.github`, `native/CMakeLists.txt`, or `native/build.ps1`.
- Never change production camera code or add compiler-version branches.
- Never merge, force-push, or modify another actor's state.
