---
task: TASK-0093
state: CLAIMED
lane: ox-pc-bc
model: openrouter/stealth/ox-alpha
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
branch: worker/verdigris/pc/ox-pc-bc
worktree: Z:\Code\.worktrees\verdigris\ox-pc-bc
head_at_claim: 8223d8d73c5fd7c894bc2c9b8b21c8bf2d93ec6a
started_at: 2026-08-23T00:00:00-07:00
scope: >
  Audit only. Produce FINDINGS.md (current native text operations, browser text
  roles, resolution/DPI requirements, glyph ranges, wrapping, alignment,
  clipping, contrast, offscreen-capture determinism, Windows/macOS backend
  needs; recommended backend-neutral text contract + locking tests). No font
  download/generation/licensing/selection. Only files under
  orchestration/tasks/TASK-0093-native-typography-contract-audit/ change.
known_risks: >
  Owner font/license approval pending per spec owner_input_dependency; audit
  proceeds with backend-neutral metrics. Frozen invariants respected:
  render-list determinism and D-113 art authority.
---
