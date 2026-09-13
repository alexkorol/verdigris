---
task: TASK-0096
title: Campaign and zone-graph measurement audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 1b2f9ebf
reviewed_at: 2026-08-23T21:10:00Z
revision: 1
---

# Review — TASK-0096 (campaign and zone-graph measurement audit)

## Verdict: ACCEPTED

Frozen head `1b2f9ebf` (worker branch `worker/verdigris/pc/ox-pc-bb`) reviewed
in detached worktree `review-task0096-1b2f9ebf`.

## Scope

Worker-only delta `f852254d..1b2f9ebf` touches only
`orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/graph.json,
captures/acceptance-rg-transcript.txt, tools/build-graph.mjs). No zone/act/
reward/duration/travel risk invented; read-only capsule honored (no ports,
port 6500 untouched). `git diff --check` clean.

## Acceptance gates

1. `rg -n "road|route|node|branch|warden|waymark|stairs|extract|campaign" native/src native/include native/tests playtest/scenarios/world-web.mjs playtest/scenarios/quest.mjs`
   → 677 lines, exit 0.
2. `node -e "...graph.json...; console.log('campaign graph: PASS')"` → prints
   `campaign graph: PASS`, exit 0. **18 nodes / 15 edges** with full
   node/edge/gate/branch/return/traversal/missing_authoring sections.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → owned additions only, exit 0.

## Evidence quality

- FINDINGS.md is excellent and well-cited. It correctly identifies the central
  structural fact: **two graphs, two books** — the headless core route table
  (Graph A: `route:tin:1:0` → `route:tin:2:0` + optional `branch:ash`) and the
  protocol world-web (Graph B: 4 roads × lazy tiers, per-house FNV-1a
  determinism), which reconcile only at two seeded tin ids
  (`networking.cpp:2386`). Both are measured with gates, branches, return
  paths, deterministic IDs, House-owned unlocks, shortest/longest traversals,
  and missing authoring info.
- **Negative control verified genuine:** 8 MISSING campaign fields preserved
  in `captures/graph.json` `missing_authoring[]` with blockers and citations —
  `campaign.act_count`, `campaign.target_duration_hours` (explicitly NOT derived
  from node counts), `mandatory_spine`, `branch_density_targets`,
  `fast_travel.risk_model` (OD-012), `repeatable_endgame.definition`,
  `world_web.persistence_across_sessions`, `authored_zone_names_and_lore`.
  No campaign field was derived from a route name.
- The delta map (measured → constitutional target) and measurement findings
  requiring successor attention (single onward-gate exposure, no unlock
  authorization on world:zone:enter, dual bookkeeping, placeholder completion,
  session-scoped persistence) are concrete and correctly parked outside owned
  paths.
- Regenerable tool contract (`tools/build-graph.mjs`) and test inventory are
  solid. Machine-readable twin `captures/graph.json` parses.

## Capsule

Read-only audit respected: no campaign content invented, no ports bound, port
6500 untouched, only owned task-folder paths changed.

## Follow-up

Successor work should author campaign layers as NEW data tables (not encoded in
names), centralize unlock authorization on world:zone:enter, and resolve the
session-scoped web persistence question — all pending owner campaign/endgame
decisions.
