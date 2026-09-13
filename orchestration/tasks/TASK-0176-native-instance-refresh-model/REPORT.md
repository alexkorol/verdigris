# TASK-0176 report

## Deliverable

`native/client/instance_refresh.hpp` — deterministic reuse vs fresh-instance
requests, expiry messaging (`RejectedExpired`), town/non-refreshable rejection,
no accidental refresh on reuse.

## Evidence

- `orchestration/tasks/TASK-0176-native-instance-refresh-model/run-tests.ps1` — exit 0, 17 checks PASS
- `python native/tools/check_legacy_denylist.py` — PASS
- `git diff --check` — clean (task paths only staged)

## Residual gaps

No `main.cpp` integration; Owner-visible instance lifecycle awaits TASK-0189.

## Successor

TASK-0189 instance lifecycle integration (0176+0175+0178).
