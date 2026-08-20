---
task: TASK-0066
title: Shared hard-fail capture helper (browser infra debt)
state: READY
packet: MECHANICAL
lane: any browser lane (mac-claude suggested if it wakes; else cursor)
priority: low-medium (infra debt; every UI task rewrites this today)
owned_paths:
  - tests/e2e/lib/**
  - orchestration/tasks/TASK-0066-capture-harness-consolidation/**
forbidden_paths:
  - existing task capture folders (history - do not rewrite old evidence)
  - src/**, server/**, playtest/**
---

# Outcome

One reusable module (tests/e2e/lib/capture-harness.mjs) extracted from
the 0055/0059 capture scripts: owns server start on a caller-supplied
port, Chronicles/guest login, viewport loop, bounding-box overlap
asserts, hard-fail JSON summary (CAPTURES OK/FAILED + non-zero exit),
PNG naming convention. Documented usage template in the module header
so future SPECs can say "use capture-harness" instead of restating the
pattern. Port must be a REQUIRED argument (no default; capsule
discipline).

# Acceptance

A demo script using the helper reproduces the 0059 assert set at
1366x768 green; unit suite untouched; architect runs the demo script.
