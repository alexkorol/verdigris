---
task: TASK-0162
title: Native passive-tree payload hardening
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 98068dfc
reviewed_at: 2026-08-23T23:45:00Z
revision: 1
---

# Review — TASK-0162 (native passive-tree payload hardening)

## Verdict: ACCEPTED

Frozen head `98068dfc` (worker branch `worker/verdigris/pc/ox-pc-bc`),
implementation head `c4346d98`, reviewed in detached worktree
`review-task0162-98068dfc`.

## Scope

Worker-only delta `1486f0e0..98068dfc` touches exactly the owned paths:
`native/client/remote_session.cpp`, `native/tests/session_tests.cpp`, and
`orchestration/tasks/TASK-0162-native-passive-tree-payload-hardening/**`
(REPORT.md, STATUS.md). No server/wire authority, protocol schema, UI, save,
gameplay, or content change. `git diff --check` clean.

## Acceptance gates (run literally at frozen head)

1. `powershell -File native/build.ps1 -RunTests` → BUILD_EXIT=0 (all suites +
   denylist PASS).
2. `native/build/verdigris_session_tests.exe` → "session tests passed",
   exit 0 (including gate-b journey).
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → exactly the two owned source files + task folder.

## Evidence quality

- **Fail-closed implementation verified in source:** `apply_passive_tree` now
  updates the mirror only when the envelope is an object with
  `schemaVersion == 2`, nonnegative integral `points.skill`/`earned` (rejecting
  NaN, negatives, fractional, and out-of-range via `sane_passive_tree_integer`),
  and array-typed `nodes`/`conduits` under a documented transport bound
  (65536). Rejections preserve the last valid snapshot untouched and emit one
  deterministic `ProtocolError` diagnostic ("passiveTree rejected: <reason>").
- **Transport bound correctly documented as not-a-balance-rule** (comment
  explicit: "a TRANSPORT BOUND, not a product rule"), staying safely inside the
  1 MiB reader frame ceiling. No tree design/cost/budget/balance authority.
- All three call sites (login, dev:state, skilltree:update) route through the
  hardened validation; absence remains legal and silent.
- 507 lines of new session tests (79 PASS checks): a test-only scripted
  loopback WebSocket server drives the real production parser with a valid
  login/update followed by missing fields, wrong types, fractional, negative,
  bare-Infinity, 1e400 inf, int-cast overflow, future schemaVersion, non-object
  tree, and oversized 65537-entry arrays; proves valid absent/zero/nonzero
  behavior unchanged, invalid data never mutates the last valid snapshot,
  diagnostics byte-stable, and healthy recovery.
- REPORT.md records literal transcripts + exit codes; STATUS flipped to
  REVIEW_REQUESTED.

## Capsule

Loopback test capsule (7160-7179) honored; no server/wire/protocol/UI/save/
gameplay/content change; port 6500 untouched; only owned paths changed.

## Follow-up

None — the fail-closed hardening is complete and test-locked. Future passive-
tree work (TASK-0136 validator consumption, progression panes) can rely on the
mirror being authoritative-only.
