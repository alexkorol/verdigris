---
task: TASK-0002
state: BLOCKED
worker: Luna build/CI implementer
worker_branch: codex/TASK-0002-build-ci-hardening
worktree: .codex/worktrees/TASK-0002-build-ci-hardening
base_commit: 0e02aa7
spec_base_commit: f5b4b72
started_at: 2026-08-15T23:41:33-07:00
expected_verification: powershell -File native/build.ps1 -RunTests -RunClient; cmake --list-presets --preset-dir native
known_risks: VS discovery portability; preserve Windows macro guard; no source edits
blocked_reason: Bundled CMake 3.20 rejects required presets schema v3; cmake is not on PATH.
---
