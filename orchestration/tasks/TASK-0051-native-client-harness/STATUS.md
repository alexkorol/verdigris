---
task: TASK-0051
state: INTEGRATED
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

Board re-check 2026-08-18 (post-0051): no further unclaimed READY tasks â€”
0053 (kimi) and 0047 (kimi-work) are peer-claimed. Re-checking next cycle.

Architect close-out 2026-08-20: work shipped in earlier merged PRs; state line was stale. Verified end-to-end today by the full 32/32 attach run on master (PR #45/#46).
