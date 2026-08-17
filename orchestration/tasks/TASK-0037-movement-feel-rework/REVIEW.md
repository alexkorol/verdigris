---
task: TASK-0037
verdict: REVISE
reviewed_commits:
  - f49b318d
---

## What was reviewed

The diff scope, the diagnosis (animation timeline restarted on every
authoritative sample + input repeat timer drift — credible and
well-evidenced), and the gates (764 unit, playtest 31/31, alternate-port
smoke — the port-6500 owner-listener handling was correct).

## Problems

1. **The branch reverts TASK-0033.** The diff removes `MIDDAY_AMBIENT`,
   `sampleAmbientForClock`, and the cycle-toggle wiring from
   `lighting-renderer.js`/`perspective-renderer.js` — undoing the
   daytime default that shipped in PR #12. Root cause: stale base
   (branched before 0033 integrated; the merge resolved against it).
   Rendering files are also outside this task's owned_paths, which is
   how the clobber slipped in.

## Required corrections (revision 1)

1. Rebase/merge onto the current program tip so the diff contains ZERO
   rendering-file changes and 0033's daytime default survives intact
   (verify: `git diff <tip>..<branch> -- src/core/rendering/` is
   empty).
2. Rerun unit + playtest on the corrected base; alternate-port smoke
   again acceptable if 6500 is occupied.

## What is correct (keep)

The movement fix itself — continuous run timeline across same-direction
samples and absolute-deadline input repeats — is exactly the diagnosed
mechanism and stays as-is. Fast re-review promised: this is a rebase,
not a rework.

## Architectural effect

Adds a standing reviewer check: every branch diff is taken against the
CURRENT tip and inspected for reverts of recently shipped work
(stale-base clobber is now a known failure class).
