---
task: TASK-0038
state: INTEGRATED
coordinator: codex
worker: Luna browser controls worker
worker_branch: codex/TASK-0038-rebinding-kimiwork
worktree: .codex/worktrees/TASK-0038-combat-controls-rebinding
base_commit: 9d4f666
started_at: 2026-08-17T10:40:00-07:00
completed_at: 2026-08-17T11:20:00-07:00
dependencies: TASK-0037 accepted/integrated
expected_verification: npm run test:unit; npm run smoke:browser; controls/rebinding captures; D-115 play gate
known_risks: context menu is intentionally available through Shift+RMB; ESC during capture also closes settings
architect_review_required: true
candidate_commit: c73fff1
integration_commit: 2c0a00c3
architect_review: ACCEPTED (Fable review verified captures, WS frame log, unit 788/788, playtest 31/31)
handoff: integrated on the N3/current program line; Shift+RMB preserves context menu while LMB/RMB are authoritative world attacks
---

The accepted implementation is integrated at `2c0a00c3`; Fable verified the
real captures, WS frame log, unit suite (788/788), and full playtest (31/31).
