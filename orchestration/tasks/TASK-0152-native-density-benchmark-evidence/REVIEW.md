---
task: TASK-0152
verdict: REVISE
reviewed_commit: 86f72c1cb04f21062eae16e299f3e05e8e88a70d
reviewed_at: 2026-08-22T20:51:00Z
reviewer: PC Verdigris architect/orchestrator
revision: 1
---

# Review — REVISE

The benchmark producer itself is reproducible and the frozen-head native gate is
green. Independent verification ran the complete native test suite plus
`-RunDensityBench`, repeated an identical seeded 1,000-entity run in two fresh
processes with matching counts/checksum, accepted all six committed positive
captures, rejected all five committed negative fixtures, and confirmed a bogus
scenario exits 2. Scope and whitespace checks are clean.

## Required correction

1. Make `--validate` independently re-enforce
   `verdigris-density-threshold/1` from the evidence fields. It currently only
   requires seven recognized ids and stored `pass: true` booleans; it does not
   verify each id's exact operator/bound/value or recompute the expected result
   from `scenario`, `determinism`, and `timings`. Consequently fabricated
   passing checks can validate. Add focused negative fixtures proving that a
   changed check value, bound, operator, or stored pass result cannot turn
   threshold-failing/inconsistent evidence green. Also bind the fixed scenario
   route/action values (`route:tin:1:0`, `Melee`) during validation, as promised
   by the schema documentation.

Keep the revision within the existing owned paths. Rerun the native test gate,
positive/negative validator matrix, deterministic double-process probe, scope
check, and `git diff --check`; then update REPORT/STATUS and push a new frozen
`REVIEW_REQUESTED` head.

