---
task: TASK-0136
title: Passive-tree contract validator CLI
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: aad6dadb
reviewed_at: 2026-08-23T23:05:00Z
revision: 1
---

# Review — TASK-0136 (passive-tree contract validator CLI)

## Verdict: ACCEPTED

Frozen head `aad6dadb` (worker branch `worker/verdigris/pc/ox-pc-bd`), content
head `aab7ad42`, reviewed in detached worktree `review-task0136-aad6dadb`.

## Scope

Worker-only delta `f1ffa64b..aad6dadb` touches only
`orchestration/tasks/TASK-0136-passive-tree-contract-validator/**`
(validate-passive-tree-contract.mjs, validator.test.mjs, fixtures/
valid-synthetic.json + counter-confusion.json, REPORT.md, STATUS.md). No
forbidden path (native/server/src/playtest/content) touched; no ports; port
6500 untouched. `git diff --check` clean.

## Acceptance gates (run literally at frozen head)

1. `node --test validator.test.mjs` → **21/21 pass**, exit 0.
2. `node validate-passive-tree-contract.mjs --contract .../passive-tree-contract.json --fixture fixtures/valid-synthetic.json --json` → exit 0.
3. `node ... --fixture fixtures/counter-confusion.json --json` → emits
   `COUNTER_CONFUSION` and **exits nonzero (1)**.
4. `git diff --check` → clean, exit 0.
5. `git diff --name-only be6d5556..HEAD` → task-folder-only for the worker's
   own delta (the base-ancestry view lists full history; the worker's commits
   touch only the TASK-0136 folder).

## Evidence quality

- The validator is clean, dependency-free, and well-documented. It binds the
  accepted TASK-0112 contract (`verdigris.passive-tree-authority` 1.0.0),
  fails closed on all 9 pipeline codes (8 required + MALFORMED_ALLOCATION),
  emits deterministic `(rank, element)`-ordered JSON errors, and preserves the
  two point ledgers structurally distinct with counter-confusion detectors
  (CC-A collapse, CC-B cross-write, CC-C mis-derivation, CC-D alias).
- **Negative controls enforced:** the native +2/axis walk
  (`native_plus_two_axis_walk`) is a named negative control; raw snapshots are
  refused without `validation_provenance` and only rebuilt through validation
  (`native_raw_snapshot_save`). No node/effect/topology/cost/balance value is
  interpreted or authored (content-neutral per SPEC).
- Fixtures behave as specified: valid exits 0, counter-confusion exits nonzero
  with COUNTER_CONFUSION on all three cases. 21 tests pass.
- REPORT.md records literal transcripts + exit codes; STATUS flipped to
  REVIEW_REQUESTED at frozen head `aad6dadb`.

## Capsule

Task-folder-only implementation respected: no forbidden path, no ports, port
6500 untouched.

## Follow-up

None — the validator is a self-contained, dependency-free CLI ready for CI
wiring or successor consumption per the TASK-0112 contract.
