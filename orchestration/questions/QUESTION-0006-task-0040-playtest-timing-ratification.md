# QUESTION-0006 — TASK-0040 full-playtest timing ratification

Related task: TASK-0040 first-encounter readability

## Decision needed

May the owner ratify the documented full `npm run playtest` timing variance
for TASK-0040 and accept the task on the strength of the isolated affected
scenarios, or must the task remain unaccepted until the harness itself is
stabilized?

## Evidence

- Independent exact-tip validator runs: 17/31, 30/31, and 28/31.
- The 30/31 miss was `gear-outcomes` at 11.64s → 10.15s against a 1.15×
  threshold (11.67s), a 0.03s boundary miss; isolated rerun passed.
- Worker ran five fresh exact-tip `gear-outcomes` repetitions: all passed,
  with low-deep/high ratios 1.287, 1.247, 1.333, 1.333, and 1.229.
- Other full-run failures were build-divergence, session-arc, quest, zones,
  and dev-state/transition timeouts; affected isolated combat/encounter,
  movement, gear, and zone scenarios pass.
- TASK-0040 focused tests: 20/20; full unit: 120 files/767 tests; marker,
  production, scope, diff-check, and real WebSocket transcript all pass.

## Options

1. Ratify the variance for this task and accept the evidence-backed isolated
   gates; track the shared playtest scheduler/poll quantization separately.
2. Require a harness-wide timing stabilization before accepting TASK-0040.

## Recommendation

Option 1. The failures span unrelated scenarios and disappear in isolated
runs. A TASK-0040-specific dev combat cadence exception would be unspecced,
brittle, and would distort product behavior.

## Status

Not blocked: source correctness and all focused evidence are complete while
the owner decides the acceptance treatment for the shared timing harness.
