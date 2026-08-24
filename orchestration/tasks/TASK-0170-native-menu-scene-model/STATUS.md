---
task: TASK-0170
state: REVIEW_REQUESTED
coordinator: openrouter
worker: cursor (composer-2.5)
machine: DESKTOP-TVU7OR7
root: Z:\Code\.worktrees\verdigris\owner-demo-runway
worker_branch: codex/TASK-0170-native-menu-scene-model-cursor
base_commit: 3d358812f86c02e5ad405566413108f97ac4e090
spec_base_commit: 3d358812f86c02e5ad405566413108f97ac4e090
ports: 6580-6599 loopback capsule reserved; port 6500 never touched
provider: cursor
model: composer-2.5
harness: Cursor agent
started_at: 2026-08-24T07:15:00Z
revision: 1
implementation_commit: pending
expected_verification: powershell -NoProfile -ExecutionPolicy Bypass -File orchestration/tasks/TASK-0170-native-menu-scene-model/run-tests.ps1; python native/tools/check_legacy_denylist.py; git diff --check; git diff --name-only
---

Claimed TASK-0170 (native menu and Escape-state model) at routed base
3d358812f86c02e5ad405566413108f97ac4e090 on worker branch
codex/TASK-0170-native-menu-scene-model-cursor. Work is confined to owned paths
native/client/menu_scene.hpp and orchestration/tasks/TASK-0170-native-menu-scene-model/**;
forbidden paths (native/client/main.cpp, native/src/**, native/include/**,
server/**, src/**) will not be touched.

IMPLEMENTED and REVIEW_REQUESTED (revision 1): adds header-only menu_scene.hpp,
self-contained test source, and PowerShell MSVC harness. Acceptance harness 69
checks PASS; legacy denylist PASS; diff --check clean. Zero forbidden-path touches.
Escape never requests quit; only explicit Quit/Confirm on confirm-quit dialog does.
