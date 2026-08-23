---
state: REVIEW_REQUESTED
task: TASK-0115-browser-panel-typography-inventory
title: Browser panel and typography inventory
lane: ox-pc-bc
model: openrouter/stealth/ox-alpha
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
branch: worker/verdigris/pc/ox-pc-bc
worktree: Z:\Code\.worktrees\verdigris\ox-pc-bc
resource_capsule: loopback port 6620 main run + 6621 negative control only; port 6500 untouched
claimed_at: 2026-08-23 (commit bd3bde95, pushed)
review_requested_at: 2026-08-23
implementation_commits: 086cefa4 (FINDINGS.md + REPORT.md + captures/, incl. panels.json with 19 panels x both viewports)
frozen_review_head: worker/verdigris/pc/ox-pc-bc pushed tip at request time — the STATUS-flip commit that carries this file; no force-push, no rebase after this line
architect_review_required: true
---

REVIEW_REQUESTED for TASK-0115 by lane ox-pc-bc.

Deliverables (MECHANICAL inventory; zero code changes outside this task folder):
- FINDINGS.md — frozen browser panel + typography contract: GameFont/ChatFont/
  UIFont roles with citations, measured 16px@1920 vs 15.026px@1366 fluid scale,
  weight 400 / #eee2c5 defaults, Georgia serif deviations (LootMoment,
  DeathOverlay, zone-note), orphaned UIFont face, 12px pinned micro-text.
- captures/panels.json — 19 panels with trigger, anchor, measured sizes at
  1920x1080 and 1366x768, mounted Vue paths, computed fonts, gameplay load,
  native phasing ranks; unmounted legacy game-panes excluded with citations.
- 38 hard-fail PNG captures (every panel at both viewports) via shared helper
  tests/e2e/lib/capture-harness.mjs on capsule ports 6620/6621 loopback only.
- Negative control PASS: injected false visibility assertion →
  "CAPTURE FAILED: negative-control.false-visibility-inventory", exit 1;
  disposable output removed, transcript on file.
- All five SPEC acceptance commands run literally, all exit 0 ("panel
  inventory: PASS"; git diff --check clean; git diff --name-only empty);
  transcripts + exit codes in REPORT.md.

No RED panels; fallback not needed. Final font choice remains owner-only.
