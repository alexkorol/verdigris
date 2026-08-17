---
task: TASK-0030
state: CLAIMED
coordinator: codex
worker: Luna native persistence implementer
worker_branch: codex/TASK-0030-native-persistence
worktree: C:\Users\Alex\Documents\ChatGPT\verdigris\.codex\worktrees\TASK-0030-native-persistence
base_commit: a4513ac4
started_at: 2026-08-16T21:31:23-07:00
dependencies: none; current program tip includes accepted and integrated TASK-0029
expected_verification: powershell -File native/build.ps1 -RunTests; round-trip byte equality; deterministic continuation; D-109 mid-instance semantics; unknown-field tolerance
known_risks: preserve fixed-step/headless core boundary; do not serialize live instance state; atomic adapter I/O must remain outside the core
architect_review_required: true
---
