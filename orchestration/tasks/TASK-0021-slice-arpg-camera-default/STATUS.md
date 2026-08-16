---
task: TASK-0021
state: REVIEW_REQUESTED
coordinator: codex
worker: Codex coordinator
worker_branch: codex/native-reconstitution
worktree: .
base_commit: 363ad39
spec_base_commit: 237e5dd
started_at: 2026-08-16T15:10:00-07:00
expected_verification: node prototypes/founding-slice/build.mjs; node prototypes/founding-slice/run-checks.mjs
known_risks: preserve the lab presets and generated index drift guard while making the ARPG default and zoom blend reversible
dependencies: none
architect_review_required: true
implementation_commit: d079e70
verification: node prototypes/founding-slice/build.mjs (PASS); node prototypes/founding-slice/run-checks.mjs (PASS: 4/4); git diff --check (PASS); headless parameter dump in REPORT.md (PASS)
---
