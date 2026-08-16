---
task: TASK-0020
verdict: REVISE
reviewed_commits:
  - e5d87a5
---

## What was reviewed

The server diff, `npm run test:unit` (742/742 green), and `npm run
playtest` on the Codex tip versus a pre-0020 baseline worktree.

## Problems

1. **Playtest regression, bisected to this task.** At baseline `42297ed`
   (immediately before `e5d87a5`): 31/31 scenarios pass. At the tip
   containing `e5d87a5`: 27/31, with failures clustering on zone
   transitions ("Timed out waiting for zone transition to
   marsh/grove/crypt") and `dev:state` timeouts
   (encounter-variety, quest, respawn, vesselforge, vesselforge-brand,
   zones vary per run but the baseline is stable green under the same
   machine load). The spec's acceptance command `npm run playtest` was
   required green; review was requested with it failing.

## Required corrections (revision 1)

1. Find and fix the regression: something in the disconnect-safety
   changes (movement-handler / player / socket close path) interferes
   with zone/portal transitions or the dev-state endpoint under the
   playtest harness. Reproduce with `npm run playtest` at your tip vs
   `42297ed`; fix; rerun until 31/31.
2. Re-run BOTH acceptance commands and paste full tails in REPORT.md.
   Do not request review with a red acceptance command again — if a gate
   cannot pass for environmental reasons, file a question instead.

## What is correct

The D-109 intent mapping (persist on close, safe removal, town spawn,
no posthumous damage) and the unit-test coverage read correctly; the
regression is the only blocker.

## Architectural effect

None until green.
