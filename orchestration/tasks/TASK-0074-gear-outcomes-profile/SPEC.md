---
task: TASK-0074
title: gear-outcomes timing profile (flake root-cause groundwork)
state: READY
packet: MECHANICAL
lane: mac-claude or cursor (browser)
priority: medium (WATCH item - named marginal scenario)
owned_paths:
  - orchestration/tasks/TASK-0074-gear-outcomes-profile/**
forbidden_paths:
  - playtest/** (read-only; PLAYTEST_TIMING_LOG already exists)
  - src/**, server/** (root cause -> note, separate fix task)
---

# Outcome

10 serialized full-suite runs with PLAYTEST_TIMING_LOG=1 on your port
capsule, then FINDINGS.md: gear-outcomes wall-time distribution
(min/median/p90/max), correlation with neighboring scenarios (does it
slow when following a specific one?), the dominant waitFor labels from
any DIAG output, and a root-cause hypothesis ranked list. If a failure
occurs mid-profiling, capture the full DIAG output - that IS the prize.

# Acceptance

timing JSONL (10 runs) + FINDINGS.md committed; suite still 32/32 in
all runs (or the failure fully DIAG-documented); no code changes.
Architect reads findings; a fix task is specced from them if warranted.
