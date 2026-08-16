---
task: TASK-0020
state: INTEGRATED
coordinator: codex
worker: Codex coordinator
worker_branch: codex/TASK-0020-browser-disconnect-safety-rev1
worktree: .codex/worktrees/TASK-0020-browser-disconnect-safety-rev1
base_commit: 1290937
spec_base_commit: 237e5dd
started_at: 2026-08-16T14:55:00-07:00
expected_verification: npm run test:unit; npm run playtest
known_risks: preserve the existing guest/Chronicles persistence seams while making persistence precede world removal and preventing all post-close combat/movement work
dependencies: none
architect_review_required: true
implementation_commit: e5d87a5
revision: 1
revision_base: 1290937
revision_reason: architect review 42fd837 bisected a 31/31 to 27/31 playtest regression to the disconnect change, clustered around zone transitions and dev:state
revision_commit: 174d769
architect_review: ACCEPTED (0c69920; rev1 race fix independently verified)
verification: targeted tests (PASS: 40); full unit (PASS: 115 files/744 tests); ESLint (PASS); git diff --check (PASS); npm run playtest on port 6520 (PASS: 31/31)
integration_commit: b035b56
integration_verification: architect gate PASS at program tip; full unit 744/744 and playtest 31/31
---
