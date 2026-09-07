---
task: TASK-0098
title: Native wire parser robustness and abuse-boundary audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 48a9d487bbb47a49981628217f3176f2a5851fc7
reviewed_at: 2026-08-23T18:20:00Z
revision: 1
---

# Review — TASK-0098 (native wire parser robustness and abuse-boundary audit)

## Verdict: ACCEPTED

Frozen head `48a9d487` (worker branch `worker/verdigris/pc/ox-pc-bc`),
content head `f3c90f09`, reviewed in detached worktree
`review-task0098-48a9d487`.

## Scope

Worker-only delta `c274dafe..48a9d487` touches only
`orchestration/tasks/TASK-0098-wire-parser-robustness-audit/**` (FINDINGS.md,
REPORT.md, STATUS.md, captures/parser-cases.json,
captures/acceptance-rg-transcript.txt). No source file modified; read-only
capsule honored (no traffic sent, no live fuzzing, no ports, port 6500
untouched). `git diff --check` clean.

## Acceptance gates (run literally at frozen head)

1. `rg -n "parse|payload|event|rate|auth|limit|invalid|unknown|close|error" native/src/networking.cpp native/include/verdigris/networking.hpp native/tests/networking_tests.cpp native/tests/session_tests.cpp`
   → 457 lines, exit 0.
2. `node -e "JSON.parse(...parser-cases.json...); console.log('parser cases: PASS')"`
   → prints `parser cases: PASS`, exit 0.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → owned additions only, exit 0.

## Escalated crash-class findings — VERIFIED in source

- **PC-014 / F-A (JSON nesting depth):** verified genuine.
  `JsonParser::value()` (networking.cpp:94-200) mutually recurses into
  `object()`/`array()` back to `value()` with **no depth budget**; parsing
  happens pre-auth on the reader thread under the 16384-byte frame cap. Deep
  nesting can plausibly exhaust a ~1 MiB default reader-thread stack and crash
  the server process. Finding and remediation sketch (hard depth budget /
  iterative descent + regression test) are correct.
- **PC-015 / F-B (road-node tier recursion):** verified genuine.
  `web_tier_width()` (networking.cpp:756-762) recurses once per tier decrement,
  so recursion depth equals the wire-supplied integer tier; `parse_node_id`
  accepts `tier>=1` within int range, and `world:zone:enter` reaches it after
  the core leg safely rejects unknown routes (core.cpp:540-541). Full static
  source-to-sink path documented. Crash-class, zero test coverage.

Both are correctly handled per SPEC: marked ESCALATION, conservative
high-if-confirmed severity with explicit preconditions and reachable paths, no
ready-to-run exploit payloads published, negative control PC-014 delivered and
**not marked safe**, dynamic confirmation intentionally deferred (read-only
capsule forbids live probing) with owner-approved-harness remediation.

## Evidence quality

- FINDINGS.md is excellent: maps 9 boundaries (envelope, framing/size, auth,
  rate, type/range, unknown events, malformed JSON, disconnect cleanup,
  deterministic errors), 30 parser cases (6 covered, 11 expected-pass gaps, 12
  red candidates, 1 negative control), plus medium integrity/abuse candidates
  (client-authoritative shop price, negative bank quantities, unclamped mint
  loops, absent rate gates, unbounded session registry) and low
  observability/parity gaps.
- Machine-readable twin `captures/parser-cases.json` parses; summary records
  the escalation list correctly.

## Capsule

Read-only audit respected throughout: no source patched, no live traffic, no
fuzzing, no ports bound, port 6500 untouched, only owned task-folder paths
changed.

## Follow-up / owner action

- PC-014 and PC-015 are genuine crash-class flaws on a loopback-bound dev
  server. Owner should commission an approved offline confirmation harness and
  the remediation sketches (JsonParser depth budget; clamp tier in
  parse_node_id / make web_tier_width iterative).
- Medium candidates (F-C..F-E) are integrity/abuse hardening for successor
  packets.
