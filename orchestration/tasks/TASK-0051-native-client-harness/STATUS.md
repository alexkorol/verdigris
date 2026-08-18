---
task: TASK-0051
state: CLAIMED
coordinator: deepseek
worker_branch: codex/TASK-0051-native-client-harness-deepseek
base_commit: 8ea0887cbfca8f024ca8c8ddb6b19746e5fab4f5
started_at: 2026-08-18T14:00:00-07:00
dependencies: [TASK-0050 ACCEPTED (merged at cc67a15e..8ea0887c)]
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests -RunClientScenarios (all scenarios PASS); one authentic negative; README note
---

Claiming TASK-0051 (native client test harness) as coordinator `deepseek`.
TASK-0050 (the client itself) is ACCEPTED, so the harness builds on the
current tip with no file conflicts.

Plan (per SPEC):
1. Render-list recording layer (semantic draw ops) in the client paint
   pipeline, headless-capable via a memory DC.
2. `verdigris_client.exe --scenario <name>` runner feeding deterministic
   input scripts through input→simulation→presentation.
3. Starter scenarios: move-and-camera (FIRST), first-fight, loot-to-bank,
   telegraph-dodge — with core/render-list/pane assertions.
4. Gate wiring: `native/build.ps1 -RunClientScenarios`.
5. Authentic negative + native/README.md scenario authoring note.
