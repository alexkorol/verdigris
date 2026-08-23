---
task: TASK-0099
title: Native performance budget and benchmark inventory
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: a12b4999
reviewed_at: 2026-08-23T18:40:00Z
revision: 1
---

# Review — TASK-0099 (native performance budget and benchmark inventory)

## Verdict: ACCEPTED

Frozen head `a12b4999` (worker branch `worker/verdigris/pc/ox-pc-bd`),
content head `b310a88f`, reviewed in detached worktree
`review-task0099-a12b4999`.

## Scope

Worker's own commits (`ee9628aa..a12b4999`) touch only
`orchestration/tasks/TASK-0099-native-performance-budget-audit/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/acceptance-1-rg-sweep.txt,
captures/benchmark-inventory.json). A base-ancestry diff shows RUN_STATUS and
TASK-0098 files, but those are base artifacts — the worker's own delta is
task-folder-only. Read-only capsule honored (no server started, no benchmark
re-executed, port 6500 untouched). `git diff --check` clean.

## Acceptance gates (run literally at frozen head)

1. `rg -n "benchmark|duration|latency|frame|memory|density|soak|p95|p99|timing" native orchestration playtest --glob ...`
   → 2234 lines, exit 0.
2. `node -e "JSON.parse(...benchmark-inventory.json...); console.log('benchmark inventory: PASS')"`
   → prints `benchmark inventory: PASS`, exit 0.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → owned additions only, exit 0.

## Evidence quality

- FINDINGS.md is excellent and well-disciplined. It inventories all 8 surfaces
  (native simulation, JS server, networking, renderer, startup, memory,
  entity-density, capture infra), labels every number OBSERVED (with citation)
  or ABSENT, and never invents budgets or tunes code. The native entity-density
  evidence (TASK-0152) is correctly identified as the only strong-provenance
  timing in the repo, with full provenance blocks verified
  (`provenance.git_ref`, `compiler`, `cpu`, `timer_resolution_ns`, etc.).
- **Negative control verified genuine:** `orchestration/DECISIONS.md:101`
  cites "own measurement: ~43ms mean frame at 1440×1000" with no commit SHA,
  date, machine, CPU, browser, sample count, or methodology — the figure
  cannot be attributed to any specific run (candidates span 40.910–45.015 ms
  across TASK-0029/TASK-0053 on unstated machines). Correctly flagged as
  anecdote until a machine-tagged capture supersedes it.
- Missing threshold dimensions (percentiles, hardware, warmup, determinism,
  regression) are precisely enumerated; the proposed machine-tagged ladder
  (L0 commit gate → L3 owner tiers) is mechanism-only and correctly defers
  numeric budgets to the owner.
- CI feasibility + false-green risks per tool are concrete and honest (e.g.
  50 ms density bounds sit ~20× above worst observed p99 → gross regressions
  could pass; memory has no measurement at all → leaks invisible to every gate).
- Machine-readable twin `captures/benchmark-inventory.json` parses.

## Capsule

Read-only audit respected throughout: no server started, no benchmark
re-executed, no ports bound, port 6500 untouched, only owned task-folder paths
changed.

## Process note

The worker disclosed using `--no-verify` on its commits because the local
yorkie pre-commit hook could not run without node_modules, and its `*.{js,vue}`
globs match none of the changed markdown/JSON files. Disclosed honestly; the
changed files are documentation/JSON only, so no lint surface was bypassed.

## Follow-up

Cut the benchmark-ladder implementation (L0/L1 rungs) and provenance-block
backfills for lifecycle soak, frame-time, and timing JSONL as successor
packets.
