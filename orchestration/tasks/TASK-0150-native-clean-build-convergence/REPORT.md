# TASK-0150 REPORT — Native clean-build convergence

Worker: ox-pc-k · Branch: `codex/TASK-0150-native-clean-build-convergence-ox-pc-k` · Worktree: `Z:\Code\.worktrees\verdigris\ox-pc-k`

## Executive summary

Proved a clean native configure/build/test plus all seven client scenarios from
a disposable build directory using only installed toolchains (VS 2019 BuildTools
MSVC 14.29, its bundled CMake 3.20.21032501 and Ninja 1.10.2). The default
`native/build.ps1 -RunTests -RunClientScenarios` gate was proven green twice from
a wiped `native/build` (before and after the change). One evidenced convergence
gap was found and fixed: the CMake/ctest path silently proved fewer tests than
the canonical helper — it omitted `camera2d_tests` entirely and had no scenario
coverage. `native/CMakeLists.txt` gained the `camera2d_tests` target/test and a
`verdigris_client --scenario all` ctest entry, so `ctest` now covers exactly the
helper's test+scenario surface. No gameplay, client, server, or tool sources were
altered; no packaging/signing; port 6500 untouched.

## Approach

1. Preflight per AGENTS.md/PROTOCOL: clean tree, exact routed HEAD
   `30e98e02…`, fetch/prune, no competing STATUS.md/RELEASE.md → CLAIMED commit
   pushed first (`ffb51437`, ~5 min after start).
2. Baseline evidence before any edit:
   - Wiped `native\build`; ran default helper `-RunTests -RunClientScenarios`
     → exit 0 (4 test exes + 7 scenarios PASS).
   - Disposable dir `%TEMP%\opencode\task0150-build`: Ninja Debug configure →
     build 20/20 → ctest 3/3 → direct `--scenario all` 7×PASS, exit 0.
3. Gap analysis: helper runs core/networking/camera2d/session tests and
   scenarios; ctest ran only 3 tests, no scenarios. `tests/camera2d_tests.cpp`
   is self-contained (`"../client/camera2d.hpp"`), so the missing target was a
   pure omission, not a coupling problem.
4. Minimal fix in owned `native/CMakeLists.txt`; re-proved both gates from
   scratch (no incremental-build claims).

## Changed files

- `native/CMakeLists.txt` (+4 lines): `camera2d_tests` executable + ctest
  registration; `verdigris_client_scenarios` ctest running
  `verdigris_client --scenario all`.

## Public interfaces added/changed

None at source level. Build surface only: `ctest` from any native configure now
executes 5 tests (was 3); the two supported build paths prove identical
test/scenario surfaces.

## Test commands + outcomes (all literal acceptance gates)

Toolchains: VS 2019 BuildTools vcvars64 (MSVC 14.29.30133), bundled cmake
3.20.21032501, ninja 1.10.2 — installed only.

1. Clean disposable-dir cycle (after fix, fresh dir):
   - `cmake -S native -B <disposable> -G Ninja -DCMAKE_BUILD_TYPE=Debug` → exit 0
   - `cmake --build <disposable>` → exit 0 (22/22 steps)
   - `ctest --test-dir <disposable> --output-on-failure` → exit 0,
     **5/5 passed** (core, networking, camera2d, session, client_scenarios)
   - `<disposable>\verdigris_client.exe --scenario all` → exit 0, 7×PASS
     (move-and-camera, first-fight, loot-to-bank, telegraph-dodge,
     combat-juice, remote-render-list, zoom-invariance)
2. Default helper remains green: wiped `native\build`, then
   `native\build.ps1 -RunTests -RunClientScenarios` → **exit 0**.
3. `git diff --check` → clean (exit 0).

## Manual verification

Scenario output inspected for per-check `ok:` lines (e.g. remote-render-list
floor/tile/orb/minimap ops, zoom-invariance uniform-delta checks) rather than
trusting exit codes alone. Server exercised indirectly via session tests and the
remote scenarios, which drive the real WebSocketServer on an ephemeral port
(never 6500).

## Commits

- `ffb51437` — STATUS-only CLAIMED claim commit (pushed).
- `ae54f024` — CMakeLists convergence fix (this report accompanies REVIEW_REQUESTED).

## Deviations

- START_HERE references `delaford_game/AGENTS.md`; that path does not exist in
  this worktree (only root `AGENTS.md`). Root guide followed instead.
- OpenCode CLI version recorded as 1.18.21 from supervisor verification per the
  recovery instruction; no out-of-worktree probing was performed.
- A fresh-worktree `git commit` initially failed because the yorkie pre-commit
  hook needs `node_modules` absent from this worktree. Resolved by `npm install`
  (untracked/gitignored deps restore; hooks then ran legitimately on every
  commit). No tracked repo files changed for this.

## Unresolved questions

None blocking. Optional follow-up: presets' `binaryDir` targets in-tree
`build/cmake/*`; left untouched since the disposable-dir gate uses explicit
`-S/-B` and presets are not part of the acceptance commands.

## Risks

Low. The added scenario ctest lengthens `ctest` by ~24 s (real server round
trip inside `--scenario all`) and requires the built client exe — both already
true of the helper gate.

## Follow-ups

- If more test executables are added to `build.ps1`, mirror them in
  `CMakeLists.txt` to keep the surfaces converged (the gap class fixed here).
