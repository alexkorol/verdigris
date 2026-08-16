---
task: TASK-0011
state: REVIEW_REQUESTED
worker: Luna native monster-skill-AI implementer
worker_branch: codex/TASK-0011-monster-skill-ai
worktree: .codex/worktrees/TASK-0011-monster-skill-ai
base_commit: e6d3f48
spec_base_commit: e6d3f48
started_at: 2026-08-16T11:08:37-07:00
expected_verification: powershell -File native/build.ps1 -RunTests
known_risks: windup ordering and cancellation must preserve non-elite melee cadence bit-for-bit; elite cone/resource rules must remain deterministic
dependencies: TASK-0010 integrated as afcd1d3
architect_review_required: true
implementation_commit: 8c68aed
validator: pending
validator_verdict: pending
---
