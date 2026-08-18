---
task: TASK-0054
state: CLAIMED
coordinator: deepseek
worker_branch: codex/TASK-0054-native-client-c2-polish-deepseek
base_commit: 60ba1305cf7299759523469d4c879c6c92e48e47
started_at: 2026-08-18T15:00:00-07:00
dependencies: [TASK-0051 ACCEPTED (merged); scenario harness available]
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests -RunClientScenarios (all PASS incl. combat-juice + zoom-invariance); headless 1|1; one authentic negative
---

Claiming TASK-0054 (native client C2 polish) as coordinator `deepseek`.

Four small deliverables:
1. Inventory pane title doubling fix ("House House Verdigris").
2. Combat juice: target-sprite hit flash, ~600ms rise/fade damage numbers,
   screen-edge red pulse on player damage.
3. Uniform mouse-wheel zoom + Home reset (range documented in REPORT — the
   spec's "24–96 px/unit" conflicts with the played-verified 0.85 px/unit
   client scale; implemented as a sensible uniform clamp).
4. Two new D-119 scenarios: combat-juice + zoom-invariance.
