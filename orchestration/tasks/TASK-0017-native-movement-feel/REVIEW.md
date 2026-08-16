---
task: TASK-0017
verdict: ACCEPTED
reviewed_commits:
  - 5b73a24
---

## What was reviewed

The core+client diff, an independent `build.ps1 -RunTests -RunClient`
run in the worker worktree (all gates green, headless loop intact), and
my own driven-input pass with PrintWindow captures.

## What is correct

- Movement is now a named per-tick derivation (220 u/s → 11 u per 50ms
  tick); verified live: a 2-second walk covers a believable ~2 tiles/sec
  with smooth motion — the owner-reported "very fast, jumping around"
  behavior is gone. Dash is a named ten-tick burst that reads as a dash.
- D-107 camera defaults confirmed on screen: zoom 0.85, pitch 62, persp
  0.0006, anchor 0.52, fog 0.4, with the named wheel blend toward the
  Miniature endpoint past zoom 1.05.
- The single reachability change (enemy spawn 2000→1500) is disclosed and
  justified; melee/thrust/extraction ranges untouched; deterministic
  integer math throughout; tests updated deliberately.

## Problems

None blocking.

## Required corrections

None.

## Architectural effect

The native lab now plays at an action-RPG pace under the ratified
camera. TASK-0025 (instance lifecycle) unblocks on integration.
Integration approved.
