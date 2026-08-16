---
task: TASK-0006
verdict: ACCEPTED
reviewed_commits:
  - f542a04f83d0e78896edd19635f8353c207b6fe3
---

## What was reviewed

Full diff `bc73ce0..f542a04` (core.hpp event enum, core.cpp drop seam, 116
test lines), the three new test bodies, and an independent
`build.ps1 -RunTests` rerun in the worker worktree (denylist PASS, all
tests PASS).

## What is correct

- Resurfacing lives in the ordinary seeded reward stream: named
  `kRelicResurfaceOneIn = 4` constant, shared `rng_` roll consumed only
  when the pool is non-empty, oldest-candidate transfer, history append,
  `RelicResurfaced` event, `relic_resurfaced` legend. Single-ownership
  invariant (pool/ground/carried/stored) holds by construction — the item
  is moved, never copied.
- Tests cover the specified round trip (death → resurface → pickup →
  extract with `relic_extracted` legend and ordered history), the
  loss-again path (returns to pool exactly once, no ground duplicate,
  nothing stored), and byte-equal replay determinism including legends.
- The spec's "at least one Scion has died" gate is equivalent to
  pool-non-empty, which is what's checked — fine.

## Problems

None blocking.

1. (Observation, future work) A resurfaced relic that is carried but NOT
   equipped at the next death is lost forever — baseline behavior, since
   only the equipped item is registered as a relic candidate. Consistent
   with "unextracted value is lost," but when Brands & Bonds work begins,
   the owner may want notable *carried* relics to re-register too. Logged
   here so the question isn't lost; not a correction.

## Required corrections

None.

## Architectural effect

The death → loss → rediscovery loop specified in D-004 is now mechanically
closed in the native core. Integration approved.
