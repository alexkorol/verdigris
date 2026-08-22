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
revision: rev2
revised_at: 2026-08-22 (bounded final handoff)
---

# TASK-0153 status (ox-pc-v)

REVISED (rev2) and REVIEW_REQUESTED (bounded final handoff).

## Revision cause

Review blocked rev1 (`8d386e24`): at 960x600 the long chronicle-derived
identity line ("House … - Scion …") collided with the centered objective
strip. Before image: `captures/review-blocker-960x600.png`. Rev2 replaces the
independent fixed-position top-HUD painters with one pure integer-geometry
planner (`plan_top_hud`, native/client/main.cpp) that measures real text
extents and places identity, objective, connection, art-status, and controls
regions across up to four rows so they cannot overlap at any width. The
painter draws exactly what the planner returns; the deterministic scenario
runs the same function over worst-case strings.

## Revision evidence (exact commands/outcomes)

- `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1
  -RunTests -RunClientScenarios` — exit 0; denylist, core/networking/
  camera2d tests, all session tests, all NINE client scenarios PASS.
- `native/build/verdigris_client.exe --scenario first-session-clarity` —
  exit 0, 20/20 checks ok.
- `git diff --check` — clean.
- `captures/accepted-hud-960x600.png` and
  `captures/accepted-hud-1366x768.png` — visually non-overlapping top HUD;
  `captures/review-blocker-960x600.png` retained as the before image.
- Changed paths verified confined to owned files: native/client/main.cpp,
  this task folder (STATUS.md, REPORT.md, captures/).

Full detail in REPORT.md (Revision section).
