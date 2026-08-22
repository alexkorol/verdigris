---
task: TASK-0159
state: REVIEW_REQUESTED
coordinator: codex
worker: ox-pc-z
machine: DESKTOP-TVU7OR7
root: Z:\Code\.worktrees\verdigris\ox-pc-z2
worker_branch: codex/TASK-0159-native-hud-pane-readability-ox-pc-z
base_commit: 1f69e82af310121ec0d20548ff15735984467406
spec_base_commit: dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17
ports: 7100-7119 loopback only; port 6500 never touched
provider: openrouter
model: stealth/ox-alpha
harness: OpenCode CLI 1.18.21 variant max
started_at: 2026-08-22T21:16:45Z
claim_commit: 0e74c1854c2a0531ec6d071cb21c0cbde1811f15
implementation_commit: 22377cf0bed832afba18f0b2ebfeaecea8d02e37
expected_verification: powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios; native/build/verdigris_client.exe --scenario first-session-clarity; git diff --check; git diff --name-only
verification_outcome: full native gate PASS with EXIT=0 (denylist PASS; core/networking/camera2d/session/presentation-events suites green; all 12 client scenarios 0 failures incl. new hud-pane-readability); focused first-session-clarity PASS exit 0 (Esc contracts re-proven); git diff --check clean; changed files exactly the owned paths
captures:
  - orchestration/tasks/TASK-0159-native-hud-pane-readability/captures/hud-pane-readability-closed-960x600.png
  - orchestration/tasks/TASK-0159-native-hud-pane-readability/captures/hud-pane-readability-open-960x600.png
  - orchestration/tasks/TASK-0159-native-hud-pane-readability/captures/hud-pane-readability-closed-1366x768.png
  - orchestration/tasks/TASK-0159-native-hud-pane-readability/captures/hud-pane-readability-open-1366x768.png
known_risks: none blocking; see REPORT.md deviations (pre-existing gate-b session-test timing flake documented at base; sibling capture regeneration during --scenario all reverted outside owned paths)
---

Claimed TASK-0159 (native HUD and gear-pane readability pass) at routed base
1f69e82af310121ec0d20548ff15735984467406 on worker branch
codex/TASK-0159-native-hud-pane-readability-ox-pc-z. Preflight proved: clean
HEAD exactly at the routed base, which contains the immutable SPEC base
dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17 as an ancestor; origin
codex/native-reconstitution tip equals the routed base; no competing
STATUS.md/claim or RELEASE.md in the task folder on current origin; owned-path
isolation with zero dirty files. Work is confined to
native/client/main.cpp and this task folder.

REVIEW_REQUESTED: implementation commit 22377cf0bed832afba18f0b2ebfeaecea8d02e37
removes every owner-visible collision found at base — identity line painted
across the minimap, the doubled "House House" prefix, the global controls hint
and objective/art/connection chips entering the open gear pane — via one pure
region-geometry source shared by painter, planner, and scenario, with
deterministic fallback ladders and a two-line wrap for the controls hint when
no single line fits beside the pane. The new `hud-pane-readability` scenario
opens the real pane through the production seam at 960x600 and 1366x768,
hard-fails on any rectangle intersection across the contract regions, proves
first-Escape/bare-Escape through the production seams, and saved four fresh
real-GDI captures under this task folder that were inspected before handoff.
Full literal acceptance command passed EXIT=0. See REPORT.md for literal
transcripts, the pre-existing session-suite flake note, and exact SHAs.
