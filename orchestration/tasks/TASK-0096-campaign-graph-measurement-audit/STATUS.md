---
task: TASK-0096
state: INTEGRATED
reviewed_commit: 1b2f9ebf
reviewed_at: 2026-08-23T21:10:00Z
lane: ox-pc-bb
worker: ox-pc-bb
model: openrouter/stealth/ox-alpha
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
branch: worker/verdigris/pc/ox-pc-bb
worktree: Z:/Code/.worktrees/verdigris/ox-pc-bb
started_at: 2026-08-23
completed_at: 2026-08-23
claim_commit: f852254d
review_head_branch: worker/verdigris/pc/ox-pc-bb (pushed tip of the commit containing this STATUS; append-only from claim)
evidence:
  - FINDINGS.md
  - captures/graph.json
  - captures/acceptance-rg-transcript.txt
  - tools/build-graph.mjs
  - REPORT.md
acceptance: all four SPEC commands run literally; transcripts + exit codes in REPORT.md; node gate printed "campaign graph: PASS"
negative_control: campaign.act_count and campaign.target_duration_hours preserved MISSING rather than derived from route names (FINDINGS §7; graph.json missing_authoring[])
owner_block_restatement: acts, branch density, naming, lore, and final pacing remain owner-only (SPEC owner_input_dependency); fast-travel risk model stays behind OD-012
preflight:
  worktree_clean: true
  upstream_divergence: "0 0 (HEAD...@{upstream})"
  base_is_ancestor_of_head: true
scope_note: mechanical measurement audit only; no zone/act/reward/duration/risk invented; only the TASK-0096 folder touched
---
