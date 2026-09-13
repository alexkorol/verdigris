---
task: TASK-0103
title: Monster, pack, rarity, and encounter gap audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 1a3be2af1c1eef28bd738ae35313e5b492b45938
reviewed_at: 2026-08-23T18:05:00Z
revision: 1
---

# Review — TASK-0103 (monster, pack, rarity, and encounter gap audit)

## Verdict: ACCEPTED

Frozen head `1a3be2af` (worker branch `worker/verdigris/pc/ox-pc-bd`)
reviewed in detached worktree `review-task0103-1a3be2af`. Content head
`06232fa4`; STATUS flipped at `1a3be2af`.

## Scope

Worker's own commits (`6d4effdb..1a3be2af`) touch only
`orchestration/tasks/TASK-0103-monster-encounter-gap-audit/**` (FINDINGS.md,
REPORT.md, STATUS.md, captures/acceptance-rg-transcript.txt,
captures/encounter-matrix.json). A base-ancestry diff view shows a RUN_STATUS.md
entry, but it is a base artifact, not a worker edit — the worker's own delta is
task-folder-only. Read-only capsule honored. `git diff --check` clean.

## Acceptance gates (run literally at frozen head)

1. `rg -n "monster|pack|spawn|rarity|unique|warden|boss|aggro|telegraph|elite|role" native/include native/src native/client native/tests playtest/scenarios`
   → 1322 lines, exit 0.
2. `node -e "JSON.parse(...encounter-matrix.json...); console.log('encounter matrix: PASS')"`
   → prints `encounter matrix: PASS`, exit 0.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → empty at clean worktree (owned additions only),
   exit 0.

## Evidence quality

- FINDINGS.md is excellent and deeply-cited. It correctly identifies **two
  parallel encounter engines** (D-114 world-unit `Simulation` vs N-series
  tile-space `WorldSimulation`) and maps 16 surfaces: spawning, pack
  composition, roles, aggro, rarity, equipment, unique/boss seams, telegraphs,
  rewards, deterministic generation, network snapshots, presentation, tests,
  plus ranked engine gaps (E1-E9) separated from owner-content gaps (O1-O7).
- **Negative control verified genuine:** the rarity invariant has no
  authoritative coverage. Producers emit only `"rare"` (`core.cpp:1753`) and
  `"elite"` (`:1763`) plus the default `"common"` (`core.hpp:789`), yet the
  loot-gate consumer table recognizes `"uncommon"` (`core.cpp:3161`) — the tier
  is orphaned/unreachable from any generator. `rarity` is an unvalidated open
  string with silent fallback to 0.05; the tier→chance table has no direct test.
- **Equipment claim verified:** `resolve_damage` applies `equipped_attack_bonus`
  only when `attacker.kind == ActorKind::Player` (`core.cpp:410`), so monsters
  cannot be equipment-scaled — blocking the constitution's
  "differently built/equipped actors" elite-difficulty clause
  (`VERDIGRIS_CONSTITUTION.md:90-91`).
- **Buffer role verified label-only:** `behaviour_type == "buffer"` is set at
  generation (`core.cpp:1746-1750`) but no simulation code consumes it (only a
  static wire `aura:damage` decoration at `networking.cpp:933`).
- Successor scaffolding S1-S5 (PackContract, RarityAuthority, RoleBinding,
  BossContract, EncounterSnapshotContract) is concrete and test-shaped; no
  monsters authored. Machine-readable twin `captures/encounter-matrix.json`
  parses.

## Capsule

Read-only audit respected: no code patched, no ports bound, port 6500
untouched, only owned task-folder paths changed.

## Follow-up

Engine packets should be cut from E1-E9; S1-S5 are their acceptance seeds for
TASK-0110.
