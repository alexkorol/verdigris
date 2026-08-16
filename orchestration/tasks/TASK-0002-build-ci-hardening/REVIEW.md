---
task: TASK-0002
verdict: REVISE
reviewed_commits: []
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
