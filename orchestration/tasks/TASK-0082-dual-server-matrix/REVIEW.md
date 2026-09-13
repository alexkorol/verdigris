---
task: TASK-0082
title: Dual-server parity matrix runner
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: bf13efa70b98d4dc77d672e06e0951a5a9d169fe
reviewed_at: 2026-08-23T17:20:00Z
revision: 1
---

# Review — TASK-0082 (dual-server parity matrix runner)

## Verdict: ACCEPTED

Frozen head `bf13efa7` (worker branch `worker/verdigris/pc/ox-pc-bb`)
reviewed in detached worktree `review-task0082-bf13efa7`.

## Scope

Worker's own delta `f1180a29..bf13efa7` touches only its owned paths:
`playtest/tools/dual-server-matrix.mjs`,
`orchestration/tasks/TASK-0082-dual-server-matrix/**` (REPORT.md, STATUS.md,
captures/smoke.json). Zero forbidden-path changes (harness/run.mjs/scenarios/
server/native/src untouched). `git diff --check` clean.

Note: a `git diff 0bee7f1e..bf13efa7` view shows deletions in
`orchestration/RUN_STATUS.md`, but this is a **stale-base artifact**, not a
worker edit — the worker branched from `f1180a29` (the program tip at re-point
time), which predates the lead's `0bee7f1e` RUN_STATUS note. The worker's own
commits (`f1180a29..bf13efa7`) never touch RUN_STATUS.md.

## Acceptance gates

1. `node --check playtest/tools/dual-server-matrix.mjs` → exit 0.
2. `powershell -File native/build.ps1 -RunTests` → build + tests compiled and
   ran. A single run showed 5 session-test check failures, but a direct re-run
   of `verdigris_session_tests.exe` at the same frozen head passed ALL checks
   (exit 0), including the reconnect and `gate-b: slain elite surfaces the
   circulating heirloom` tests. The intermittent failure is the known-flaky
   gate-b hunt leg (RNG-dependent: "the successor fell to ordinary combat"),
   not caused by this task — the worker's delta has zero native changes.
3. `node playtest/tools/dual-server-matrix.mjs ... --js-port 6541 --native-port 6542 --scenarios quickstart,movement,zones --out .../smoke.json`
   → artifact written; `parity: true`; JS 3/3 PASS (quickstart/movement/zones),
   native 3/3 PASS; runner exit codes 0/0; `asymmetric: []`.
4. Artifact assertion prints `dual-server smoke: PASS` (exit 0).
5. `git diff --check` → clean, exit 0.

## Artifact verification

- `servers.js.url = ws://127.0.0.1:6541`, `servers.native.url =
  ws://127.0.0.1:6542` — both loopback, both inside the 6540-6559 capsule,
  neither 6500.
- Native listener binds `inet_addr("127.0.0.1")`
  (`native/src/networking.cpp`), JS binds `127.0.0.1` — loopback confirmed.
- `FORBIDDEN_PORT = 6500` with explicit rejection; duplicate-port check
  present; child processes tracked and killed only for spawned children
  (`killAll` on exit).
- Authentic negative (nonexistent scenario) → exit 2, no artifact written,
  nothing to restore (per REPORT.md).

## Tool quality

`playtest/tools/dual-server-matrix.mjs` is clean, self-terminating, serializes
JS then native against the UNCHANGED runner in `--attach` mode, records
revision/exe-hash/commands/urls/ports/PIDs/boot times/save paths per server,
writes the artifact atomically, and exits non-zero on any red or asymmetric
scenario.

## Follow-up

Layer-2 sweep layers (D-116) can call this tool with broader scenario lists.
