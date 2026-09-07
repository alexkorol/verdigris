---
task: TASK-0152
verdict: ACCEPTED
reviewed_commit: d34097d9148f60f71d88f51d3bbec4a7c236b27a
reviewed_at: 2026-08-23T04:55:00Z
reviewer: independent validator (deepseek-v4-flash)
revision: 3
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

## Revision 3 — independent re-validation (ACCEPTED)

Re-validated by a different model (deepseek-v4-flash) at the same frozen head
`d34097d9148f60f71d88f51d3bbec4a7c236b27a` in detached worktree
`Z:\Code\.worktrees\verdigris\review-task0152-d34097d9`. Every acceptance
claim was re-run independently, not read from the report:

1. **Native gate** — `powershell -NoProfile -ExecutionPolicy Bypass -File
   native\build.ps1 -RunTests` → `GATE_EXIT=0`: legacy denylist PASS, core
   PASS, networking PASS, camera2d PASS, session tests passed, presentation
   events tests PASS.
2. **Fresh bench build** with build.ps1's exact `cl` flags
   (`/nologo /std:c++20 /EHsc /W4` MSVC 2019 v16.11.42 x64) →
   `COMPILE_EXIT=0`, `LINK_EXIT=0`, zero warnings.
3. **Validator matrix** on the committed captures: 6/6 positive captures
   `--validate` EXIT=0; 12/12 negative fixtures EXIT=1, including the seven
   tamper controls (check-value, check-bound, check-op, check-pass,
   threshold-fail, route, action).
4. **Double-process determinism probe** — two fresh processes
   `--n 1000 --run 1 --seed 42424242`: both EXIT=0, both
   `determinism.state_checksum = fnv1a64:9f2964a4df5ac069` (identical to each
   other and to the committed n=1000 capture), counts equal
   (monsters_start 1000, events 19, final_tick 1001), `reproducible=true`,
   `counts_match=true`, `checksum_match=true`; the fresh evidence validates
   with EXIT=0.
5. **Usage contract** — `--scenario bogus`, `--n 0`, and no args all EXIT=2.
6. **Scope + whitespace** — base→head diff touches only
   `native/tools/entity_density_bench.cpp` and this task folder (SCOPE_OK);
   `git diff --check` clean (exit 0). The worktree stayed clean; the bench
   was built into the gitignored `native/build/`.
7. **Integration** — the worker head `d34097d9` carries byte-identical
   TASK-0152 content to the integrated program commit `76837dec` on
   `origin/codex/native-reconstitution`.

Verdict: **ACCEPTED** — independent confirmation of Revision 2. The stale
STATUS `state: REVIEW_REQUESTED` is reconciled to INTEGRATED (see STATUS.md
note); no further work is required.
