---
question: QUESTION-0002
related_task: TASK-0002
state: ANSWERED
---

# TASK-0002 generator portability versus CMake 3.20

## Decision needed

Should TASK-0002 retain the revision-3, VS-version-neutral `NMake Makefiles`
preset together with an explicit `ilammy/msvc-dev-cmd@v1` workflow setup, or
should the implementation pursue another generator strategy?

## Evidence

- Architect review revision 2 prefers removing the `Visual Studio 16 2019`
  generator pin so `windows-latest` can use VS2022.
- The available bundled CMake is `3.20.21032501-MSVC_2` and accepts schema v2.
- On that CMake, removing the `generator` field makes the configure preset
  invalid; inheriting `default` selects Ninja, which is unavailable in a
  clean shell.
- Revision 2's NMake preset configured, built, and tested only after manually
  initializing `vcvars64.bat`; a clean workflow-equivalent shell failed
  because `cl` was not on PATH.
- Revision 3 keeps unpinned NMake and adds `ilammy/msvc-dev-cmd@v1` before
  configure/build/test. Independent validation returned **ACCEPT** after a
  clean temporary configure/build/CTest, full native gate, YAML parse, and
  diff check.

## Options

1. Accept revision 3 (recommended by current evidence): retain an explicit,
   version-neutral NMake generator and initialize the runner's installed MSVC
   environment in CI. This avoids both VS-version pinning and a clean-runner
   compiler failure.
2. Require a different preset/toolchain strategy. This needs a concrete
   CMake-version and local-compatibility acceptance rule before implementation.
3. Restore a Visual Studio generator pin. This preserves local configure
   behavior but contradicts the portability correction and risks CI failure on
   runners without the pinned edition.

## Current impact

No implementation work is blocked: revision 3 is review-ready and the other
wave-2 tasks are independently review-ready. The question only records the
architectural tradeoff for TASK-0002 acceptance.

## Recorded answer

The architect accepted option 1 in the wave-2 review: keep the version-neutral
NMake preset and initialize MSVC through `ilammy/msvc-dev-cmd@v1`. TASK-0002 is
accepted as revision 3 (`f9c979b`).
