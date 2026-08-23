---
task: TASK-0094
title: Native asset provenance manifest audit
state: CLAIMED
lane: ox-pc-bc
model: openrouter/stealth/ox-alpha
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
branch: worker/verdigris/pc/ox-pc-bc
worktree: Z:\Code\.worktrees\verdigris\ox-pc-bc
head_at_claim: 5059d485ad444abb924cad01aa7e33760b364043
started_at: 2026-08-23T21:30:00-07:00
scope: >
  Mechanical audit only. Produce FINDINGS.md and captures/assets.json listing
  every asset consumed or proposed by native presentation (relative path,
  type, dimensions, bytes, hash, provenance evidence, license status,
  build/package use, KEEP/UNKNOWN/BLOCKED). No asset, license, package
  manifest, or product canon changes; hash only files already present; no
  network downloads. Only files under
  orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/ change.
known_risks: >
  Production asset policy and final selections remain owner-only; assets with
  missing provenance are classified UNKNOWN, never inferred.
---

# TASK-0094 claim

- Preflight (AGENTS.md) verified 2026-08-23: `git status --short` clean;
  remotes origin/upstream/codexclone as configured; `git fetch --prune origin`
  clean; branch `worker/verdigris/pc/ox-pc-bc` in sync with upstream (0/0);
  SPEC base `d2423873` ancestor-verified of HEAD via
  `git merge-base --is-ancestor`.
- First STATUS write wins; no competing STATUS.md existed in this folder at
  claim time (folder contained only SPEC.md).
- Resource capsule honored: read-only, no downloads, port 6500 untouched.
