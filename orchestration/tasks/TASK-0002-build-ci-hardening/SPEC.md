---
id: TASK-0002
title: Native build hardening and CI for the native workspace
state: READY
track: tooling
priority: high
base_commit: f5b4b72
dependencies: []
parallel_safe: true
owned_paths:
  - native/build.ps1
  - native/CMakeLists.txt
  - native/CMakePresets.json
  - native/tools/**
  - .github/workflows/native.yml
forbidden_paths:
  - native/src/**
  - native/include/**
  - native/tests/**
  - native/client/**
  - src/**
  - server/**
  - prototypes/**
  - .github/workflows/ci.yml
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests -RunClient
---

## Goal

The native workspace builds reliably on any Windows machine with VS Build
Tools (any recent version, any install path) and is guarded by CI.

## Why this task exists

`build.ps1` hardcodes two vcvars paths (2019/2017 BuildTools) and each MSVC
invocation prints a noisy `vswhere.exe` not-found error from the VS
environment scripts. A wrong or missing define already caused a real defect:
the client silently compiled its console fallback until
`VERDIGRIS_NATIVE_WINDOWS` was added this session (commit 170f3e2). Nothing
runs the native suite automatically.

## Product and architectural invariants

- The core remains dependency-free; do not add packages or vcpkg here.
- `build.ps1` remains the one-command Windows entry; its three modes
  (default build, `-RunTests`, `-RunClient` headless) keep working.
- The denylist check (`native/tools/check_legacy_denylist.py`) stays in the
  build path.

## Inputs and references

- `native/build.ps1`, `native/CMakeLists.txt`, `native/README.md`.
- Existing CI: `.github/workflows/` (do not modify existing workflows; add a
  separate `native.yml`).

## Scope

1. `build.ps1`: locate MSVC via `vswhere.exe` when present
   (`${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe`,
   `-latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64`),
   falling back to the current hardcoded candidates, and also accepting full
   VS editions (Community/Professional/Enterprise), not only BuildTools.
   Fail with a clear message listing what was probed.
2. Keep `/DVERDIGRIS_NATIVE_WINDOWS=1` on the client compile; add a guard so
   a missing define cannot silently regress: after linking, the script runs
   the client with `--assert-windowed` OR simpler, greps `cl` args — choose
   the cheapest reliable mechanism and document it in the script. (If a
   client-side flag is chosen, STOP — client code is forbidden here; instead
   assert within the script that the define was passed.)
3. Add `native/CMakePresets.json` with at least `windows-msvc` and
   `default` (Ninja or platform default) configure+build+test presets so
   `cmake --preset` works on Windows/macOS/Linux.
4. Add `.github/workflows/native.yml`: on push/PR touching `native/**`,
   windows-latest job: configure with CMake, build, `ctest
   --output-on-failure`, run the denylist script. (CI uses CMake, not
   build.ps1, so the workflow also proves the CMake path that macOS will
   use. Note: CMake defines `VERDIGRIS_NATIVE_WINDOWS` already.)
5. Suppress or bypass the harmless vswhere stderr noise in build.ps1 output.

## Non-goals

- No changes to simulation, client, or test sources.
- No macOS runner yet (CMake presets prepare for it; adding the runner is a
  follow-up).
- No changes to the browser game's CI.

## Deliverables

- Updated `native/build.ps1`, new `native/CMakePresets.json`,
  new `.github/workflows/native.yml`, README note if invocation changed
  (README edit allowed? No — `native/README.md` is not in owned_paths; if
  the README must change, say so in REPORT.md instead of editing).
- One coherent commit.

## Acceptance criteria

- `powershell -File native/build.ps1 -RunTests -RunClient` exits 0 on the
  current machine (VS2019 BuildTools) with tests + denylist + headless loop
  green and no vswhere error noise in output.
- `cmake --list-presets` shows the new presets and a local
  `cmake --preset windows-msvc` configure succeeds (build may be exercised
  via the preset too if the toolchain allows).
- `native.yml` is syntactically valid YAML and self-consistent (job builds,
  tests, denylist). CI cannot run locally; reviewer checks the workflow by
  reading it.

## Required verification

```powershell
powershell -File native/build.ps1 -RunTests -RunClient
cmake --list-presets --preset-dir native  # or equivalent from native/
```

Paste outputs into REPORT.md.

## File ownership

Only the five owned paths. Do not touch native sources/tests/client.

## Dependencies

None. Parallel-safe with TASK-0001 (sources) and TASK-0003 (prototypes) —
disjoint files.

## Parallel-safety assessment

Build files are not edited by TASK-0001/0003. The acceptance run compiles
whatever sources are present; run it on your own worktree state.

## Review focus

- vswhere discovery correctness and the fallback chain.
- That the define-guard actually fails the build if the define is dropped.
- Workflow triggers scoped to `native/**` so browser CI is unaffected.

## Stop conditions

- CMake or the VS installation on the machine cannot support presets v3 →
  stop, report versions, propose an alternative.
- Any need to edit files outside owned_paths → stop and file a question.
