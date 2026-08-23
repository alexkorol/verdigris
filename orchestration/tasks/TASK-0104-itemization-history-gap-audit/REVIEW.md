---
task: TASK-0104
title: Itemization, extraction, and item-history gap audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 40c505ac9b05ecc6d14a17391f252d3ddd2e3246
reviewed_at: 2026-08-23T17:50:00Z
revision: 1
---

# Review — TASK-0104 (itemization, extraction, and item-history gap audit)

## Verdict: ACCEPTED

Frozen head `40c505ac` (worker branch `worker/verdigris/pc/ox-pc-bb`)
reviewed in detached worktree `review-task0104-40c505ac`.

## Scope

Worker-only delta `fca5b7c5..40c505ac` touches only
`orchestration/tasks/TASK-0104-itemization-history-gap-audit/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/item-lifecycle.json). No forbidden
path; read-only capsule honored; `git diff --check` clean.

## Acceptance gates (run literally at frozen head)

1. `rg -n "item|inventory|equip|drop|pickup|extract|relic|scar|brand|bond|forge|stable.*id" native/include native/src native/client native/tests docs/product`
   → 2046 lines, exit 0.
2. `node -e "JSON.parse(...item-lifecycle.json...); console.log('item lifecycle: PASS')"`
   → prints `item lifecycle: PASS`, exit 0.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → empty at clean worktree (owned untracked
   additions only), exit 0.

## Evidence quality

- FINDINGS.md is outstanding and deeply-cited. It correctly recognizes the
  native tree carries **two disjoint item universes** — (A) the D-114 combat
  simulation (`Item`/`Scion`/`House`) and (B) the tile-space browser-parity
  world (`GameItem`/`VesselItem`/`PlayerInventory`/`WearSet`/`GroundItem`,
  N4/N5) — and maps both independently across stable IDs, generation,
  equip/unequip/use, drops, pickup, inventory, extraction, D-106 death
  recovery, relic candidacy/circulation, scars/history, persistence, wire
  payloads, presentation, and a full test-coverage matrix (22 edges).
- **Negative control NC-1 verified genuine:** `ItemHistoryUpdated` emissions
  (core.cpp:436,448,523,536) are asserted by **no** test — `rg -n
  "ItemHistoryUpdated" native/tests` returns 0 hits (exit 1), while the
  underlying state (`use_count`) IS tested (core_tests.cpp:1558). A refactor
  that dropped the history event would pass the whole suite.
- **NC-2 verified genuine (inherited from TASK-0097):** schemaVersion rejection
  (core.cpp:1299-1301) has only the happy-path `schemaVersion=1` header check
  (core_tests.cpp:763), no rejection test.
- Ranked gaps R1-R8 (durability seam, scar lifecycle, cross-session stable
  uuid, universe convergence, history-event test gap, wire projection,
  version rejection, equip idempotence smearing history) are concrete and
  content-neutral; formulas/economy/names correctly left owner-only.
- Machine-readable twin `captures/item-lifecycle.json` parses.

## Capsule

Read-only audit respected: no production file modified, no ports bound, port
6500 untouched, only owned task-folder paths changed.

## Follow-up

Successor should start from the smallest locking tests (L1 in G-05, G-07),
then the identity spine decision (G-04) before G-01/G-02/G-03 implementations.
