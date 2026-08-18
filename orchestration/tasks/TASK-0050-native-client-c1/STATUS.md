---
task: TASK-0050
state: REVIEW_REQUESTED
coordinator: deepseek
worker_branch: codex/TASK-0050-native-client-c1-deepseek
base_commit: cc67a15ee7adb4244ba12d2e14296097f6afa288
started_at: 2026-08-18T12:40:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests -RunClient (camera2d PASS + headless trophies/items 1/1); documented manual play sequence
---

Implemented and pushed for architect review. All three deliverables done:

1. 2D top-down via `camera2d.hpp` (2.5D path removed).
2. Visible combat (telegraphs, swings, hit flashes, floating damage numbers,
   death removal, drop visibility).
3. Real inventory pane (grid + weapon seat + stats + banked) with
   equip/unequip via the pane (new deterministic `Command::unequip`).

Gates green: denylist, core, networking, and camera2d tests all PASS; headless
proof `trophies stored: 1 | items stored: 1`. The architect should build and
PLAY the exe before ACCEPTED (D-117). Details in `REPORT.md`.
