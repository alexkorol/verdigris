---
task: TASK-0100
title: Deterministic replay coverage and divergence audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 74e5dc1730d3efa970e8935a534a9ad05dd8399d
reviewed_at: 2026-08-23T17:35:00Z
revision: 1
---

# Review — TASK-0100 (deterministic replay coverage and divergence audit)

## Verdict: ACCEPTED

Frozen head `74e5dc17` (worker branch `worker/verdigris/pc/ox-pc-bd`)
reviewed in detached worktree `review-task0100-74e5dc17`. Content head
`1a311ab5`; STATUS flipped to REVIEW_REQUESTED at `74e5dc17`.

## Scope

Worker-only delta `0bee7f1e..74e5dc17` touches only
`orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/acceptance-rg-transcript.txt,
captures/replay-surfaces.json). No forbidden path; no core patch; read-only
capsule honored. `git diff --check` clean.

## Acceptance gates (run literally at frozen head)

1. `rg -n "seed|rng|random|tick|fixed|replay|snapshot|determin|clock|time" native/include native/src native/tests`
   → 494 lines, exit 0.
2. `node -e "JSON.parse(...replay-surfaces.json...); console.log('replay surfaces: PASS')"`
   → prints `replay surfaces: PASS`, exit 0.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → empty at clean worktree (evidence files are the
   worker's committed task-folder additions), exit 0.

## Evidence quality

- FINDINGS.md is an excellent, deeply-cited determinism audit: command surface
  (replayable today, strongest seam), seeds table, **five distinct RNG streams**
  with serialization status, ticks-vs-wall-clock boundaries, snapshot/
  persistence analysis, ranked divergence risks, a full existing replay-proof
  inventory, and two concrete versioned contracts (ReplayRecord v1 and
  DivergenceReport v1) with a smallest scaffold for the successor.
- **Headline negative control verified genuine:** `WorldSimulation` live state
  has no capture path — `world_random_state_` advances on every loot roll
  (`core.cpp:3070-3071`) yet appears in no snapshot (verified: the
  `snapshot()` block at `core.cpp:1226+` serializes only `rng.state`/
  `rng.serial`/tick/legend/house fields). Identical command histories diverge
  in loot after any restart.
- **Session RNG truncation verified:** `session_rng_(static_cast<std::uint32_t>(seed ^ (seed >> 32)))`
  (`networking.cpp:579`) — two distinct 64-bit seeds can collide onto one
  32-bit Mulberry32 state, as claimed.
- Machine-readable twin `captures/replay-surfaces.json` parses; contracts are
  definition-only (no code shipped), correctly reserved for TASK-0106 lineage.

## Capsule

Read-only audit respected: no core patched, no ports bound, port 6500
untouched, only owned task-folder paths changed.

## Follow-up

Successor (TASK-0106 lineage) should build the golden-record scaffold (§10);
WorldSimulation recording is correctly deferred until GAP-1/GAP-2 have an owner
to avoid legitimizing the wall-clock leak.
