---
task: TASK-0067
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0067-native-journey-ci-cursor
base_commit: 1f45eb337b29995485ba2b5adf60f5cdb00393c3
architect_review_required: true
---

# TASK-0067 REPORT — native journey CI

## Executive summary

The Native workflow's `journey` job on `windows-latest` now proves the owner
path: CMake configure/build, legacy denylist, core/networking/session ctest
(session tests start their own loopback server and cover the remote guest
journey plus clean shutdown), camera2d tests, and `verdigris_client.exe
--scenario all`. Density bench is not invoked. Failures upload
`native/build/ci-logs/`. Runtime on the green run was ~1m20s (limit 14m).

`native/build.ps1` is unchanged (not owned). It still cannot find VS 18 on
GitHub-hosted runners, so CI uses `ilammy/msvc-dev-cmd@v1` plus the existing
`windows-msvc` CMake preset, then the extra owner steps in
`native/tools/ci-native.ps1`.

## Approach

- Replace the previous native job with `journey` calling
  `native/tools/ci-native.ps1`.
- Camera2d is still compiled ad-hoc (not in CMake). VS 18 rejects
  range-for over a braced list unless `<initializer_list>` is visible;
  the test file does not include it. CI compiles via `cmd.exe` with
  `/std:c++20 /FIinitializer_list` so the unowned test is not edited.
- Local scenarios present into an offscreen GDI DC and passed on the
  hosted runner (no interactive desktop required).

## Changed files

- `.github/workflows/native.yml` — `journey` job, 14m timeout, failure
  artifact upload
- `native/tools/ci-native.ps1` — owner-path gate script
- `orchestration/tasks/TASK-0067-native-journey-ci/**` — STATUS/REPORT

## Verification

Green (owner path complete):
https://github.com/alexkorol/verdigris/actions/runs/32365296275
(`25584b57`, ~1m21s)

Canary (inverted `remote_guest_journey` extract assertion, then reverted):
https://github.com/alexkorol/verdigris/actions/runs/32365594226
(`83202f6a`) — `verdigris_session_tests` failed with
`FAIL CANARY: journey extract must fail CI`; ctest exited 8; camera2d and
`--scenario all` did not run because the journey ctest step failed first.

Architect: retrigger Native once on this worker branch after review.

## Deviations

- Did not invoke `native/build.ps1` in Actions. First attempt
  (https://github.com/alexkorol/verdigris/actions/runs/32364579841) died in
  ~24s: `vcvars64.bat was not found` because the script probes
  `Program Files (x86)` / vswhere's first line, while the runner has VS 18
  under `C:\Program Files\Microsoft Visual Studio\18\Enterprise`.
- `/FIinitializer_list` on the camera2d `cl` line only (test source
  untouched).
- Canary edited `native/tests/session_tests.cpp` for one commit, then
  reverted before review (not in owned_paths; required by acceptance).
