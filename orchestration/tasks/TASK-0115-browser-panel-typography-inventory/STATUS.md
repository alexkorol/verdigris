---
state: CLAIMED
task: TASK-0115-browser-panel-typography-inventory
title: Browser panel and typography inventory
lane: ox-pc-bc
model: openrouter/stealth/ox-alpha
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
branch: worker/verdigris/pc/ox-pc-bc
worktree: Z:\Code\.worktrees\verdigris\ox-pc-bc
resource_capsule: loopback 6620-6639 only via shared capture helper; port 6500 untouched
claimed_at: 2026-08-23
---

CLAIMED for TASK-0115 by lane ox-pc-bc.

Plan: static source/style inventory (rg sweeps over src/**/*.vue + css), then one
hard-fail Playwright capture per persistent/situational browser panel through
tests/e2e/lib/capture-harness.mjs on a disposable capsule port (6620-6639),
with visibility assertions per panel at 1920x1080 and 1366x768, negative
control included. Deliverables land under this task folder only:
FINDINGS.md, captures/panels.json, captures/*.png, REPORT.md.
Inventory only — no src/, server/, or native/ changes; no font selection.
