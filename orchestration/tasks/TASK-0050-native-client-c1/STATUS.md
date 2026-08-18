---
task: TASK-0050
state: CLAIMED
coordinator: deepseek
worker_branch: codex/TASK-0050-native-client-c1-deepseek
base_commit: cc67a15ee7adb4244ba12d2e14296097f6afa288
started_at: 2026-08-18T12:40:00-07:00
dependencies: []
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests -RunClient (camera2d tests PASS + headless trophies/items stored); scripted demo captures for combat + inventory
---

Claiming TASK-0050 (native client C1) as coordinator `deepseek`.

Three owner-visible deliverables, D-118 (drop the broken 2.5D billboard
projection for a clean 2D top-down) first:
1. 2D top-down presentation via the architect's `camera2d.hpp`.
2. Visible combat (swings/projectiles, telegraphs, hit flashes, damage
   numbers, death removal, drop visibility).
3. Real inventory pane (grid + equip slots + stats readout + extraction).

Work on worker branch `codex/TASK-0050-native-client-c1-deepseek`.
