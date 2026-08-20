# Standing loop — canonical coordinator contract (v2, 2026-08-18)

Single source of truth for every coordinator's continuous-mode goal.
Substitute NAME (deepseek | kimi | kimi-work), CLONE path, and PORTS.
Briefs reference this file; goal texts must not drift from it.

## The loop

Repeat forever:

1. In CLONE: fetch + merge `origin/codex/native-reconstitution`; read
   `orchestration/RUN_STATUS.md` (routing), `ORCHESTRATION.md`,
   `ACCEPTANCE.md`.
2. Check your task folders for new `REVIEW.md` verdicts — a REVISE on
   your work outranks everything; fix and re-request first.
3. Otherwise claim the highest-priority READY task not already
   claimed, preferring your lane suggestion in RUN_STATUS.
4. Implement per SPEC: owned paths only, literal gate transcripts per
   ACCEPTANCE.md, hard-fail captures for UI, your PORTS only,
   loopback binds, never port 6500.
5. Flip STATUS to `REVIEW_REQUESTED`, push, loop to step 1 without
   waiting for the review.

## Claim semantics (exact, INC-011)

- A claim IS: a committed `STATUS.md` in the task's folder with
  `state: CLAIMED`, `coordinator: NAME`, worker branch, base commit.
  First committed claim wins; back off if one exists with another
  coordinator's name.
- NOTHING ELSE in a task folder is a claim. A stop-note, question, or
  stale file is not a claim — replace it when you claim.
- Your own notes NEVER go into a task folder's `STATUS.md` unless you
  are claiming it. Board-empty/status notes go to
  `orchestration/NOTES-NAME.md` only.

## Empty-board: peer verification first, then backoff

Before sleeping on an empty board, check for any task in
`REVIEW_REQUESTED` implemented by a DIFFERENT coordinator whose folder
has no `REVIEW-PEER-*.md` yet. If one exists and it is a browser task:
rerun its exact acceptance gates in YOUR clone (fresh merge with
current tip first), commit literal transcripts as
`REVIEW-PEER-NAME.md` in that task folder, push. Report facts only —
verdicts stay with the architect. Never edit the implementer's files
or STATUS. Native tasks: skip (architect-only review).

## Empty-board backoff (INC-011)

If no READY task exists after a fetch (and no peer verification is
pending): append one line to
`NOTES-NAME.md` (first time only), then run an actual sleep command
(`powershell -Command "Start-Sleep 900"`), re-check, and DOUBLE the
sleep up to 3600s while the board stays empty. Never spin
instant-fetch cycles.

## Stop conditions

Credits/quota nearly exhausted (note it in NOTES-NAME.md), or the
owner/architect says stop. An empty board is a BACKOFF, not a stop.

## Always forbidden

Merging to the program branch or master; editing other coordinators'
task files or evidence; peer processes/accounts/credentials; port
6500; weakening playtest assertions; claiming work you have not run.
