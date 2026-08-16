---
task: TASK-0002
state: BLOCKED
branch: codex/TASK-0002-build-ci-hardening
commits: []
base_commit: 0e02aa7
spec_base_commit: f5b4b72
---

## Executive summary

The build/CI implementation is staged but cannot be committed or marked
review-ready because the current environment cannot satisfy the task's
required CMake presets v3 acceptance gate.

## Implementation in preserved worker worktree

- `native/build.ps1`: vswhere discovery, edition-aware fallback, clear probe
  errors, suppressed vswhere stderr, and a script-side
  `/DVERDIGRIS_NATIVE_WINDOWS=1` guard.
- `native/CMakePresets.json`: schema-v3 `default` and `windows-msvc` presets.
- `.github/workflows/native.yml`: native-scoped Windows configure/build/test/
  denylist workflow.

These changes remain uncommitted in the isolated worker worktree so they can
be resumed after the environment decision. No forbidden files were touched.

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests -RunClient`: PASS.
- Bundled CMake `3.20.21032501-MSVC_2`: rejects the required presets schema v3
  with `Unrecognized "version" field`.
- `cmake` is not on `PATH`.

## Stop condition and question

The spec explicitly says to stop when the toolchain cannot support presets v3.
`orchestration/questions/QUESTION-0001-cmake-presets-toolchain.md` records
the evidence, options, and recommendation to provide CMake 3.23+ rather than
silently weakening the acceptance contract.

## Follow-up

After a compatible CMake is available, rerun the worker's acceptance commands,
commit the preserved changes, and return the task to `REVIEW_REQUESTED`.
