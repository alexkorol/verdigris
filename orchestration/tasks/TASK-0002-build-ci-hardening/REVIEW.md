---
task: TASK-0002
verdict: ACCEPTED
reviewed_commits:
  - 659b8802f82dfb6839207c05700a5d1cf27380a0
  - 44a20b277f87bd0c1af0686ca2119aaf3d09b23d
  - f9c979b40afce5ccf43e3f73a3bc82400649b212
---

# Final verdict: ACCEPTED (2026-08-16, rev 2 verified)

Both rev-2 corrections independently verified on this machine:

- Correction 5: the VS16 generator pin is gone; `windows-msvc` now uses the
  version-neutral NMake generator, and `native.yml` supplies the developer
  environment via `ilammy/msvc-dev-cmd@v1` before configure — works on
  windows-latest (VS2022) and locally under vcvars. Note for the record:
  the preset description's claim that schema v2 *requires* a generator is
  not strictly accurate, but the chosen solution is sound regardless.
- Correction 6: PATH-injection of the VS Installer directory inside
  `Invoke-Msvc` eliminates the vswhere noise; my acceptance run is clean,
  and vcvars now even resolves the true toolset version (16.11.42).
- Reconfirmed: `build.ps1 -RunTests -RunClient` exits 0 (denylist, tests,
  headless loop), bundled CMake 3.20 lists both presets, define guard
  present. Integration approved.

Historical review below.

---

## What was reviewed

The BLOCKED report, QUESTION-0001 (CMake 3.20.21032501-MSVC_2 rejects
presets schema v3; no `cmake` on PATH), and the preserved uncommitted work
described as held in the worker worktree. The worker stopped correctly at a
spec stop condition instead of silently lowering the schema — that is
exactly the intended behavior.

## Decision on QUESTION-0001

Option 2, bounded (recorded as D-104 in DECISIONS.md): use **presets schema
version 2**, which CMake 3.20 accepts and every newer CMake (including CI
runners) also accepts. Nothing this task needs is v3-only: configure, build,
and test presets all exist in v2. Do not require the owner to install a new
toolchain overnight for a schema field. If a genuinely v3-only capability is
ever needed, a follow-up task may add a CI-only preset file.

## Required corrections (revision 1)

1. `native/CMakePresets.json` uses `"version": 2` with configure presets
   `windows-msvc` and `default`, plus build and test presets referencing
   them. No v3-only fields (no `toolchainFile`, no `condition`).
2. Acceptance criterion "presets validate locally" is amended to: invoking
   the MSVC-bundled CMake 3.20 binary (full path is acceptable; PATH
   presence is NOT required) with `--list-presets` from `native/` succeeds
   and shows the presets. Record the exact binary path used in REPORT.md.
3. The `.github/workflows/native.yml` job uses `cmake --preset default`
   (or `windows-msvc`) so CI exercises the same preset file.
4. All other original scope items stand unchanged: vswhere discovery with
   BuildTools+full-edition fallback and a clear failure message, the
   `VERDIGRIS_NATIVE_WINDOWS` define guard asserted script-side, denylist
   kept in the build path, vswhere stderr noise suppressed, and the
   `build.ps1 -RunTests -RunClient` acceptance run.

## What is correct

Stop-condition discipline, evidence quality in the question file, and
preserving the partial work instead of discarding it.

## Architectural effect

D-104 (provisional): CMake presets are pinned to schema v2 until a concrete
v3-only need is demonstrated. QUESTION-0001 can be closed by Codex.

---

# Revision 2 (2026-08-16, review of commit 659b880)

Verdict remains **REVISE** — close, two concrete defects. Independently
verified on this machine: `build.ps1 -RunTests -RunClient` exits 0 with the
define guard active, and the MSVC-bundled CMake 3.20 lists both presets
(v2 schema accepted). vswhere discovery, probe-list error, full-edition
fallbacks, and the script-side define guard are all correct — keep them.

## Required corrections

5. `native/CMakePresets.json`: the `windows-msvc` configure preset pins
   `"generator": "Visual Studio 16 2019"`. GitHub `windows-latest` runners
   ship VS2022, so the CI job's `cmake --preset windows-msvc` will fail.
   Remove the `generator` field from that preset (CMake then selects the
   newest installed Visual Studio; local VS2019 BuildTools and CI VS2022
   both work), or split a `ci` preset — removing the pin is preferred.
6. Spec item 5 (noise suppression) is not met: every `Invoke-Msvc` call
   still prints `'vswhere.exe' is not recognized …` because
   `vcvars64.bat` itself invokes vswhere, which is not on PATH inside the
   spawned cmd. Fix inside `Invoke-Msvc`: prepend the VS Installer
   directory to PATH in the cmd command line before `call vcvars64.bat`,
   e.g. `set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%" && call …`
   (guard for the directory existing). Re-run acceptance and confirm the
   noise is gone from the transcript pasted into REPORT.md.

## What was verified this pass

- `powershell -File native/build.ps1 -RunTests -RunClient` → denylist PASS,
  core tests PASS, headless loop OK (worktree `659b880`).
- Bundled CMake 3.20 `--list-presets` shows `default` and `windows-msvc`.
