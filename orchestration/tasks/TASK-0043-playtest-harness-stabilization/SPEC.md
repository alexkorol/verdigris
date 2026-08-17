---
id: TASK-0043
title: Playtest harness stabilization — the parity bar must not flake
state: READY
track: tooling
priority: critical
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - playtest/**
  - tests/e2e/**
forbidden_paths:
  - server/**
  - src/**
  - native/**
  - package.json
acceptance_commands: []
---

## Goal

`npm run playtest` becomes trustworthy under machine load: the observed
flake classes (dev:state probe timeouts, zone-transition 8s timeouts,
gear-outcomes 1.15x boundary misses, scheduler cadence ~9Hz vs expected)
are eliminated or made load-adaptive WITHOUT loosening real regression
detection. This harness is the D-116 parity bar; its verdicts must mean
something.

## Scope

Diagnose each flake class with evidence (QUESTION-0006 has the data);
typical fixes: deadline-based waits instead of fixed 8s, dev:state probe
retry with backoff, threshold ratios computed against a same-run
baseline rather than absolute seconds, scenario setup serialization if
port/server contention is implicated. Harness-only changes.

## Acceptance criteria

TEN consecutive full `npm run playtest` runs, 31/31 each, executed while
a parallel CPU load runs (document the load method), transcripts
committed to the task folder. A deliberately introduced regression (e.g.
break a zone transition in a scratch copy) still FAILS — prove
sensitivity is preserved with one negative run.

## Review focus

No detection loosening disguised as stabilization; the ten-transcript
proof; negative-run authenticity.
