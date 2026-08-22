---
task: TASK-0080
verdict: REVISE
reviewed_head: 0ab4e7a5712c5b06bff46b580f200fd0946ec6c3
reviewed_at: 2026-08-21 22:24 -07:00
---

# TASK-0080 review — REVISE

The implementation is read-only, remains within the owned paths, and all five
literal SPEC gates pass at the pushed worker head. The 16-test suite covers
the listed fixture cases, and the queue-floor and malformed-state negative
controls have the required non-zero behavior.

One controller-critical correction is required before acceptance:

1. Discover and surface CLAIMED, IMPLEMENTED, REVIEW_REQUESTED, and REVISE
   tasks from the complete task-folder truth, not only from task ids that still
   appear in the Effective READY table. A correct coordinator removes a task
   from READY when it is routed or claimed; that transition must not make the
   claim disappear from the sentinel's claimed/review/revise output or from
   owned-path collision analysis.
2. Add regression fixtures where a task absent from the READY table has a
   CLAIMED or REVIEW_REQUESTED STATUS, and where such a global live claim
   collides with a READY task. The first must be surfaced without changing the
   READY count; the second must exit non-zero with the collision named.
3. Rerun all five literal SPEC gates and the negative floor control, preserve
   exact evidence, keep the change within this task's owned paths, and push an
   additive revision on the same worker branch. Do not amend or force-push.

This review deliberately preserves the existing parser, report, captures, and
passing behavior. The required change is limited to global lifecycle
enumeration, collision membership, and regression coverage.
