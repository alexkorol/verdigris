---
task: TASK-0001
verdict: ACCEPTED
reviewed_commits:
  - 7ed844d8d5a9a6fb2f5a2d2ee9428c10b1cf7fad
---

## What was reviewed

Full diff `0e02aa7..7ed844d` (core.hpp, core.cpp, core_tests.cpp), the four
new test bodies, and an independent rerun of
`powershell -File native/build.ps1 -RunTests` in the worker worktree
(build clean, denylist PASS, core tests PASS).

## What is correct

- `record_legend` is fully deterministic: monotonic ordinals, tick-stamped,
  no clock or unseeded randomness; `operator==` enables the byte-equal
  replay assertion, and the replay test passes.
- Eviction preserves founding entries first and falls back safely when all
  entries are founding; the cap is a named constant as specified.
- Entry kinds map to real constitution moments (creation, first clear,
  unlocks, elite kill, death with killer+route, relic candidacy/extraction,
  campaign completion) — no noise kinds.
- Elite identity is a flag plus a level-delta rule on the shared schema —
  actor symmetry (D-003) intact.
- `handle_death` gained killer attribution via a defaulted parameter, so
  all existing call sites and behaviors are preserved; existing 11 behaviors
  still green.

## Problems

None blocking. Two observations, no action required:

1. `clear_route_and_unlock_children` now sets `campaign_complete` only once
   (previously reassigned every clear). Behavior-improving and covered by
   tests; noting it as an intentional semantic tightening.
2. The `relic_extracted` path is dormant at this baseline, as the report
   honestly states. Keep the hook; a future relic re-entry task will
   exercise it.

## Required corrections

None.

## Optional follow-ups

- When serialization arrives, `LegendEntry` should get a stable on-disk
  ordering guarantee (the in-memory vector already provides one).

## Architectural effect

None; implements D-004 groundwork. Integration approved — merge
`codex/TASK-0001-native-legends-records` into the program branch.
