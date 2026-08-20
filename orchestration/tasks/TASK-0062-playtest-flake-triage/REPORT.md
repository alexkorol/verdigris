---
task: TASK-0062
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0062-playtest-flake-triage-cursor
base_commit: 7f3ba270ef3f9d561188942aeb738fb8b0647097
architect_review_required: true
---

# TASK-0062 REPORT — Playtest flake diagnostics (no assertion changes)

## Executive summary

The runner now names a failing scenario and why. On failure it prints
wall time, the `waitFor` timeline (label, authored timeout budget,
elapsed, ok), and the last five envelopes. `PLAYTEST_TIMING_LOG=1`
appends per-scenario wall + p99 lag to JSONL.

Three serialized full suites on port **6580** were **32/32**. The
31/32 flake did not reproduce. Slowest/most variable: `gear-outcomes`
(32–53s). See FINDINGS.md.

## Changed files

- `playtest/lib/diagnostics.mjs` — waitFor wrap (budgets untouched),
  DIAG formatter, JSONL append.
- `playtest/run.mjs` — install diagnostics; print DIAG on fail; optional
  timing log. Exit code / pass-fail unchanged.
- `orchestration/tasks/TASK-0062-playtest-flake-triage/{STATUS,REPORT,FINDINGS,timing.jsonl}`

No `playtest/scenarios/**` edits. No timeout/retry/assert changes.

## Test commands and outcomes

`npm run test:unit`:

```
 Test Files  134 passed (134)
      Tests  841 passed (841)
```

`PLAYTEST_PORT=6580 PLAYTEST_TIMING_LOG=1 npm run playtest` (three runs):

```
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":43.876351,"maxEventLoopLagMs":146.145279}

32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":32.194559,"maxEventLoopLagMs":117.112831}

32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":32.276479,"maxEventLoopLagMs":113.901567}
```

Authentic negative (stub HeadlessPlayer `waitFor` throws): runner-shaped
DIAG line includes scenario name, wait label `loot drop`, timeout budget
8000, and last 5 envelopes. Exit semantics of the real runner unchanged.

## Deviations

- `playtest/lib/` did not exist; created for the owned diagnostics module
  instead of patching `harness.mjs` (not in owned_paths).
- Flake did not reproduce; no live last-envelope capture from a 31/32.
