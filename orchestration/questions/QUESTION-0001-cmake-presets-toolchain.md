---
question: QUESTION-0001
related_task: TASK-0002
state: OPEN
---

# CMake preset schema support

## Decision needed

TASK-0002 requires `native/CMakePresets.json` with schema version 3 and a
successful local preset configure. The available bundled CMake is
`3.20.21032501-MSVC_2` and rejects the `version` field used by schema v3;
`cmake` is not available on `PATH`.

## Evidence

- `powershell -NoProfile -File native/build.ps1 -RunTests -RunClient` passes in
  the worker worktree.
- `cmake --list-presets --preset-dir native` cannot run from `PATH`.
- Invoking the bundled CMake 3.20 binary against the v3 presets fails with
  `Unrecognized "version" field`.

## Options

1. Install/provide CMake 3.23+ (recommended) and resume TASK-0002 unchanged;
   this preserves the specified schema-v3 contract and validates the intended
   cross-platform path.
2. Permit a schema-v1/v2 compatibility preset for this environment; this
   weakens the task's explicit v3 acceptance criterion and needs an architect
   decision.

## Recommendation

Use option 1. Keep the worker's uncommitted build/CI changes preserved in its
isolated worktree until a compatible CMake is available. Do not silently lower
the preset schema from the implementation task.
