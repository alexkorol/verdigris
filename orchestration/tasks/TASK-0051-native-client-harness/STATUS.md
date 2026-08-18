---
task: TASK-0051
state: REVIEW_REQUESTED
coordinator: deepseek
worker_branch: codex/TASK-0051-native-client-harness-deepseek
base_commit: 8ea0887cbfca8f024ca8c8ddb6b19746e5fab4f5
started_at: 2026-08-18T14:00:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests -RunClientScenarios (all scenarios PASS); one authentic negative; README note
---

Implemented and pushed for architect review. D-119 scenario harness is live:

- `render_list.hpp` records semantic draw ops through the real paint
  pipeline; scenarios assert on core state + render list + pane/HUD.
- `verdigris_client.exe --scenario <name|all>` runner; four starter
  scenarios (`move-and-camera`, `first-fight`, `loot-to-bank`,
  `telegraph-dodge`) all PASS.
- `native/build.ps1 -RunClientScenarios` gate wired.
- Authentic negative: suppressing the swing draw makes `first-fight` fail;
  restored green.
- `native/README.md` documents how to add a scenario.

Details in `REPORT.md`. The architect should rerun the scenario set
personally (G5).
