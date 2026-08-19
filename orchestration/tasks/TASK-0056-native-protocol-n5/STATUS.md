---
task: TASK-0056
state: CLAIMED
coordinator: deepseek
worker_branch: codex/TASK-0056-native-protocol-n5-deepseek
base_commit: def01194ab9438603cbe9252d19a113c9ee41b3d
started_at: 2026-08-18T18:00:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests (denylist/core/networking PASS); PLAYTEST_WS_URL attach of Chronicles/death family + N1-N4 regression, harness UNCHANGED; one authentic negative; C++ unit coverage list
---

Claiming TASK-0056 (N5 Chronicles/death/persistence parity over the C++
server), re-routed to deepseek lane by the architect (kimi-work
quota-stalled). Precondition verified: TASK-0047 INTEGRATED (PR #24) and
its item/inventory/equip/Vesselforge types exist in native/src/core.cpp
(resolve_equip/resolve_unequip/carried_items/stored_items present).

Base = current program tip def01194 (post 0057 integration, PR #26).
