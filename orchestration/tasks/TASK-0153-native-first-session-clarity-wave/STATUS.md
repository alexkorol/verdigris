---
task: TASK-0153
state: REVIEW_REQUESTED
worker: ox-pc-v
provider: openrouter
model: stealth/ox-alpha
branch: codex/TASK-0153-native-first-session-clarity-wave-ox-pc-v
worktree: Z:\Code\.worktrees\verdigris\ox-pc-v
routed_base: 4dfa4f1eac8853fcf82393e41abcf14419cff7b4
ports: 7040-7059
claimed_at: 2026-08-22 07:03 PDT
review_requested_at: 2026-08-22 07:52 PDT
claim_commit: 8474ac5125d3725c7fd119ac907e907c14da75d6
implementation_commit: 8d386e24
---

# TASK-0153 status (ox-pc-v)

IMPLEMENTED and REVIEW_REQUESTED. All literal SPEC gates pass:

- `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1
  -RunTests -RunClientScenarios` — legacy denylist PASS; core, networking,
  camera2d unit tests PASS; session tests PASS (local / remote-negative /
  remote / journey / reconnect / replaced / render-list); all NINE client
  scenarios PASS with 0 failures each (move-and-camera, first-fight,
  loot-to-bank, telegraph-dodge, combat-juice, remote-render-list,
  zoom-invariance, chronicles-gate-b, first-session-clarity).
- `native/build/verdigris_client.exe --scenario first-session-clarity`
  exits 0, proving all three clarity contracts plus the owner Esc contract.
- `git diff --check` clean. Changed paths confined to owned files:
  native/client/main.cpp, native/client/presentation_state.{hpp,cpp},
  native/README.md (+ this task folder). A gate-b run regenerated an unowned
  TASK-0145 evidence PNG; it was restored before commit.

Full evidence in REPORT.md.
