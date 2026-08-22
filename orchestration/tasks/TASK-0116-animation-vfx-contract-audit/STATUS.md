---
task: TASK-0116
state: CLAIMED
coordinator: ox-alpha
worker: ox-pc-s (headless OpenCode Ox Alpha worker)
provider: openrouter
model: stealth/ox-alpha
worker_branch: codex/TASK-0116-animation-vfx-contract-audit-ox-pc-s
worktree: Z:\Code\.worktrees\verdigris\ox-pc-s
base_commit: 9fe673b66ffc082e865e0f0fb66f454ec1984949
spec_base_commit: 9bd689b4cebac0fe1f79ba54edcc9967a1a8f0d4
started_at: 2026-08-22T06:30:31-07:00
expected_verification: >
  rg -n "animation|frame|facing|swing|telegraph|impact|death|dash|effect|particle|aura|orb|camera" native/client native/include native/src native/tests orchestration/benchmarks ;
  node JSON.parse gate on captures/animation-vfx-matrix.json ;
  git diff --check ; git diff --name-only (only this task folder)
known_risks: audit-only capsule; no source edits, ports, or asset generation; owner asset decisions remain open per D-113
---
