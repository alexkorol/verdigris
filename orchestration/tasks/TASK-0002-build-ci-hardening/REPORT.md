---
task: TASK-0002
state: REVIEW_REQUESTED
branch: codex/TASK-0002-build-ci-hardening
commits:
  - 659b8802f82dfb6839207c05700a5d1cf27380a0
  - 44a20b277f87bd0c1af0686ca2119aaf3d09b23d
  - f9c979b40afce5ccf43e3f73a3bc82400649b212
base_commit: 0e02aa7
spec_base_commit: f5b4b72
---

## Executive summary

Revision 1 implements the architect's D-104 schema-v2 correction while
preserving the requested Windows toolchain discovery, define guard, denylist,
workflow, and native acceptance behavior. It is independently validated and
awaits architect acceptance; it is not integrated yet.

## Revision 1 report

Revision 1 implements D-104's CMake schema-v2 compatibility requirement and is
ready for architect review.

Implementation commit: `659b8802f82dfb6839207c05700a5d1cf27380a0`.

- `native/CMakePresets.json` uses schema version 2 and defines `default` and
  `windows-msvc` configure/build/test presets.
- `.github/workflows/native.yml` invokes the Windows preset workflow.
- `native/build.ps1` preserves vswhere discovery/fallback, the Windows define
  guard, quiet probe stderr, and the denylist.

Independent validator `/root/validate_task_0002`: ACCEPT.

Verification used the bundled CMake 3.20.21032501-MSVC_2 by absolute path:

- preset listing showed `default` and `windows-msvc`;
- configure/build succeeded;
- bundled `ctest --preset windows-msvc`: 1/1 passed;
- `powershell -File native/build.ps1 -RunTests -RunClient`: denylist, core
  tests, and headless client passed;
- YAML parsing passed and no vswhere error noise appeared.

## Initial revision history

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

## Risks and limitations

The repository does not provide `cmake` on PATH, so local preset evidence uses
the bundled Visual Studio 2019 CMake 3.20 binary by absolute path. No v3-only
preset feature is required by this task. The implementation remains isolated
until architect review.

## Integration notes

Architect review is still `REVISE` for the original v3 attempt; revision 1
addresses D-104 and has independent `ACCEPT` validation. Integrate worker
commit `659b8802f82dfb6839207c05700a5d1cf27380a0` only after a new
architect `ACCEPTED` review is present.

## Revision 2 requested corrections

Architect review `REVISE` requires two concrete fixes to commit `659b880`:

1. Remove the `Visual Studio 16 2019` generator pin from the
   `windows-msvc` preset so GitHub `windows-latest` can select VS2022 while
   local VS2019 CMake 3.20 remains supported.
2. In `native/build.ps1`, prepend the VS Installer directory to the spawned
   `cmd.exe` PATH before calling `vcvars64.bat`, guarded by an existence check,
   so vcvars-internal vswhere diagnostics are actually suppressed.

The existing vswhere discovery, full-edition fallback, define guard, denylist,
and schema-v2 behavior must remain unchanged. Return the revised commit to
independent validation before requesting architect review again.

## Revision 2 independent validation

Validator `/root/validate_task_0002_rev2` returned **REVISE**. The native
script gate, denylist, define guard, and manually initialized NMake
configure/build/CTest all passed, but a clean workflow-equivalent shell failed
at `cmake -S native --preset windows-msvc` because `NMake Makefiles` requires
`cl` from `vcvars64.bat` and `.github/workflows/native.yml` does not initialize
that environment. The required correction is to remove the `generator` field
so CMake selects the installed Visual Studio generator on VS2019/VS2022
 runners, matching the architect's preferred resolution. No files were edited
by the validator.

## Revision 3 implementation

Revision 3 is committed as `f9c979b40afce5ccf43e3f73a3bc82400649b212`.
Because CMake 3.20 schema-v2 presets cannot omit the generator without
becoming invalid (or inheriting Ninja), the worker retained the unpinned
`NMake Makefiles` preset and added `ilammy/msvc-dev-cmd@v1` with `arch: x64`
before the workflow's configure/build/test steps. This makes the preset
self-consistent on a clean `windows-latest` runner without pinning VS2019 or
VS2022. The worker reports clean temporary configure/build/CTest, full
`build.ps1 -RunTests -RunClient`, YAML parse, diff-check, and no-vswhere-noise
passes.

Independent validator `/root/validate_task_0002_rev3` returned **ACCEPT**.
The validator confirmed the owned-path scope, schema-v2 preset listing, clean
temporary NMake configure/build/CTest under initialized x64 MSVC, full native
build gate, denylist, YAML parse, diff check, and quiet output. It also
confirmed `ilammy/msvc-dev-cmd@v1` runs before workflow configure/build/test,
closing the clean-runner defect from revision 2. Architect review and
integration follow.

## Changed files

- `native/build.ps1`
- `native/CMakePresets.json`
- `.github/workflows/native.yml`

## Interfaces

The Windows entrypoint remains `native/build.ps1`; schema-v2 configure,
build, and test presets remain `default` and `windows-msvc`. CI now initializes
the x64 MSVC developer environment before invoking the unpinned NMake preset.

## Manual checks

The revision-3 validator used a clean temporary preset directory with
initialized x64 MSVC and exercised configure, build, CTest, the full PowerShell
gate, YAML parsing, diff check, and captured-output inspection.

## Specification deviations

The `windows-msvc` preset uses unpinned `NMake Makefiles` plus an explicit CI
MSVC setup action because bundled CMake 3.20 schema v2 cannot express an
omitted generator without invalidating the preset or inheriting Ninja. This is
documented in the worker commit and avoids a VS2019/VS2022 pin.

## Risks and limitations

The workflow depends on the external `ilammy/msvc-dev-cmd@v1` action to provide
the runner's MSVC environment; the local build.ps1 path remains independent.

## Questions for Fable or the owner

`orchestration/questions/QUESTION-0002-task-0002-generator-environment.md`
records the CMake 3.20/schema-v2 generator tradeoff. The architect answered it
by accepting the validated revision-3 environment setup.

## Integration notes

Integrated revision-3 commit `f9c979b40afce5ccf43e3f73a3bc82400649b212`
(plus its accepted revision ancestors) as `ddd7198`, `8bf4ee5`, and
`5ed0739`; the architect review is `ACCEPTED`.
