---
task: TASK-0013
verdict: ACCEPTED
reviewed_commits:
  - c2d62c3
---

## What was reviewed

Client-only diff at `c2d62c3`, independent `build.ps1 -RunTests
-RunClient` rerun (green, headless output unchanged), and the worker's
driven-pass captures (before-strike inspected: `telegraphs 1`, red warning
geometry, life 78 pre-strike; report documents life 78→67 and
`telegraphs 0` after the strike).

## What is correct

- Presentation-only tracking keyed by actor with event-time position/
  facing snapshots; expiry on strike, death, route transition, or elapsed
  windup — the full lifetime contract.
- Honest geometry stance: core range constants are private, so the client
  documents its 1100/1650 approximations instead of smuggling in core
  exports — exactly the stop-condition discipline the spec demanded, with
  the limitation recorded.
- The temporary route-unlock bootstrap used for verification was reverted
  before commit and disclosed.

## Problems

None blocking.

1. (Watch item, shared with 0009's) The client now approximates two more
   core constants by value. The future snapshot/query seam should expose
   skill costs AND telegraph-relevant ranges together.

## Required corrections

None.

## Architectural effect

Constitution §3.9 telegraph readability is now real end to end:
simulation truth → event → rendered warning. Integration approved.
