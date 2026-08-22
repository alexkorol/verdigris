---
task: TASK-0152
verdict: ACCEPTED
reviewed_commit: d34097d9148f60f71d88f51d3bbec4a7c236b27a
reviewed_at: 2026-08-22T21:22:00Z
reviewer: PC Verdigris architect/orchestrator
revision: 2
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

## Revision 2 — ACCEPTED

Frozen worker head `d34097d9148f60f71d88f51d3bbec4a7c236b27a`
implements the requested correction entirely within the existing owned paths.
The validator now binds the fixed route/action, pins every threshold row's
operator and bound, recomputes each value from its authoritative evidence
field, and compares the stored pass result with the recomputed result.

Independent review in a detached exact-head worktree passed the full native
build/test gate. A fresh MSVC `/W4` compile of the benchmark emitted no
warnings; all six committed positive captures validated; all twelve negative
fixtures failed with exit 1, including the seven focused value/bound/operator/
pass/route/action/threshold-forgery controls. The frozen diff is limited to
`native/tools/entity_density_bench.cpp` and this task folder, and
`git diff --check` is clean. Verdict: ACCEPTED for program integration.
