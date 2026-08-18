---
task: TASK-0054
state: REVIEW_REQUESTED
coordinator: deepseek
worker_branch: codex/TASK-0054-native-client-c2-polish-deepseek
base_commit: 60ba1305cf7299759523469d4c879c6c92e48e47
started_at: 2026-08-18T15:00:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests -RunClientScenarios (all 6 scenarios PASS); headless 1|1; one authentic negative
---

Implemented and pushed for architect review. All four deliverables done:

1. Pane title doubling fixed ("Gear / House Verdigris").
2. Combat juice: target-sprite hit flash, ~600ms rise/fade damage numbers,
   150ms screen-edge red pulse on player damage.
3. Uniform mouse-wheel zoom + Home reset (0.5x–2x envelope around the
   played-verified 0.85 default; see REPORT deviation note on the spec's
   "24–96 px/unit" units).
4. Two new scenarios: `combat-juice` (7 checks) + `zoom-invariance` (9
   checks), both PASS alongside the existing four.

Authentic negative: suppressing the target-flash draw makes `combat-juice`
fail; restored green. Details in `REPORT.md`.

Commits:
- `424de9f9` — claim
- `73bd91c0` — implementation (title fix + combat juice + zoom + scenarios)
- `f831be70` — REPORT implementation SHA
