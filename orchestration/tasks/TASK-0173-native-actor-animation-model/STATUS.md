---
task: TASK-0173
state: REVIEW_REQUESTED
worker: ox-alpha-pc-w4
worktree: Z:\Code\.worktrees\verdigris\worker-t0173
branch: ox/TASK-0173
base_commit: 1498048722d1d3a54ae8d1d48677b87e503af241
spec_base_commit: 3d358812f86c02e5ad405566413108f97ac4e090
started_at: 2026-08-24T00:00:00-07:00
revision: 1
implementation_commit: c093d278
expected_verification: powershell -NoProfile -ExecutionPolicy Bypass -File orchestration/tasks/TASK-0173-native-actor-animation-model/run-tests.ps1; python native/tools/check_legacy_denylist.py; git diff --check; git status --porcelain
---

Claimed TASK-0173 (native actor animation-state model) at base
1498048722d1d3a54ae8d1d48677b87e503af241 on branch ox/TASK-0173. Work confined to
owned paths native/client/actor_animation.hpp and orchestration/tasks/TASK-0173-native-actor-animation-model/**;
forbidden paths (main.cpp, native/src/**, native/include/**, server/**, src/**) untouched.

IMPLEMENTED and REVIEW_REQUESTED (revision 1): header-only actor_animation.hpp
(idle/locomotion-blend/windup/swing-thrust-slam-cast/hit/death/recovery,
event-free tick, interruption rules, irreversible death, 8-way facing,
renderer-ready pose) plus self-contained deterministic tests and MSVC dual-build
harness. Acceptance build: 87 checks PASS. /DNEGATIVE_CONTROL build fails with
exit 1 as required (inverted zero-duration-attack assertion), proving the
nonzero-visible-attack checks bite. Legacy denylist PASS; git diff --check clean;
worktree clean after final commits.
