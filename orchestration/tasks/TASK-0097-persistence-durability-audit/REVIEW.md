---
task: TASK-0097
title: Native persistence durability and fault-model audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 0c373d2ff2c921a1bfb02ec85d34ac5a380ea77a
reviewed_at: 2026-08-23T17:10:00Z
revision: 1
---

# Review — TASK-0097 (native persistence durability and fault-model audit)

## Verdict: ACCEPTED

Frozen head `0c373d2f` (worker branch `worker/verdigris/pc/ox-pc-bc`)
reviewed in detached worktree `review-task0097-0c373d2f`.

## Scope

Worker-only delta `0bee7f1e..0c373d2f` touches only:
`orchestration/tasks/TASK-0097-persistence-durability-audit/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/persistence-contract.json).
No forbidden path modified. `git diff --check` clean.

## Acceptance gates (run literally at frozen head)

1. `rg -n "persist|save|load|profile|serialize|version|reconnect|relic|House|Scion" native/src native/include native/tests`
   → exit 0, 627 lines.
2. `node -e "JSON.parse(...persistence-contract.json...); console.log('persistence contract: PASS')"`
   → prints `persistence contract: PASS`, exit 0.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → empty at clean worktree (evidence files are the
   worker's committed task-folder additions), exit 0.

## Evidence quality

- FINDINGS.md is a deep, correctly-cited layer map (core snapshot format,
  file adapter, session/profile layer), persisted-field inventory, save-trigger
  analysis, three version seams with divergent validation policies, and a
  reconnect-vs-durability truth table. Every claim carries a `file:line`
  citation.
- **Headline finding verified:** the persistence *library* (snapshot/restore +
  atomic temp+rename adapter) is unit-tested, but has **zero production
  callers** — `write_snapshot`/`read_snapshot`/adapter appear nowhere under
  `native/src` or `native/include`; the adapter include graph is
  `core_tests.cpp:9` + `include/verdigris/persistence.hpp:6` only. D-109's
  crash-safety currently holds only inside a living process.
- **Negative control (F-03) verified genuine:** the `schemaVersion != 1`
  hard-throw (`core.cpp:1297-1301`) is implemented but locked by **no test** —
  the only reference in `native/tests` is the happy-path
  `schemaVersion=1` prefix check (`core_tests.cpp:763`). A refactor that
  silently accepted foreign versions would pass the whole suite.
- 14-row deterministic disposable-profile fault matrix (F-01..F-14) with
  coverage today + smallest locking tests (L1-L7) is concrete and actionable;
  machine-readable twin `captures/persistence-contract.json` parses.

## Capsule

Read-only audit respected: no real owner save opened or mutated, no ports
bound, port 6500 untouched, only owned task-folder paths changed.

## Follow-up for the successor

R1 (no production save trigger) and R2 (ProtocolSession profile has no
serialization seam) are the real P0s; successor work should begin from the
L1-L7 locking tests and ADR-002's one-file-per-House model.
