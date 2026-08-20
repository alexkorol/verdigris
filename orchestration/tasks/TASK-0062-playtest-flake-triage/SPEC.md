---
task: TASK-0062
title: Playtest marginal-timeout flake triage + diagnostics
state: READY
packet: BOUNDED-DESIGN
lane: mac-claude suggested (browser/JS); any browser lane may claim
priority: medium (WATCH escalation — 2nd sighting 2026-08-20)
owned_paths:
  - playtest/run.mjs (diagnostics ONLY — see hard constraint)
  - playtest/lib/** (diagnostics ONLY)
  - orchestration/tasks/TASK-0062-playtest-flake-triage/**
forbidden_paths:
  - playtest scenario assertions, timeouts, retries (measuring stick —
    architect ruling required for ANY behavioral change)
  - src/**, server/** (unless a root cause is found — then STOP and
    file the finding for a separate fix task)
  - native/**
---

# Outcome

The suite tells us WHICH scenario flakes and WHY. Twice now a full run
has come back 31/32 with an immediate clean rerun (sightings: ~2026-08-17
loot scenario; 2026-08-20 02:26 scenario unidentified — the summary line
only reports counts).

## Deliverables

1. On any scenario failure, the runner prints a per-scenario line:
   name, wall time, attempt timeline of protocol waits (event waited
   for, timeout budget, elapsed), and the last 5 envelopes seen. Exit
   code and pass/fail semantics UNCHANGED.
2. A `PLAYTEST_TIMING_LOG=1` mode that appends per-scenario wall time +
   p99 lag to a JSONL file so repeat runs build a distribution.
3. Evidence: 3 consecutive full-suite runs (serialized, your port
   capsule) with the timing log; a short FINDINGS.md ranking the
   slowest/most marginal scenarios and, if the flake reproduces, the
   captured diagnosis.

## Acceptance

- Full playtest still 32/32 (three runs recorded).
- `npm run test:unit` green.
- Diff shows NO assertion/timeout/retry changes in scenarios.
- Architect reruns one full suite with diagnostics on.
