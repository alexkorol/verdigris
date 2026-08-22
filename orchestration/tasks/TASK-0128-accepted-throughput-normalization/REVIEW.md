---
task: TASK-0128
verdict: REVISE
reviewed_head: bb67c5660dda2745469458f258dc24ecf115415d
reviewed_at: 2026-08-22 01:35 -07:00
---

# TASK-0128 review — REVISE

The collector design, null/UNKNOWN discipline, experimental-unit key, path
containment, and 16-test battery are useful and remain accepted foundations.
One release-blocking evidence defect remains:

1. The committed captures bind `repo_revision` to claim commit `0d1898bd`, but
   the pushed review head is `bb67c566`. Therefore `--check` against the
   authoritative worker head fails until write mode rewrites both captures.
   Architect verification reproduced this: write mode changed exactly the two
   capture files from `0d1898bd...` to `bb67c566...`; tests then passed 16/16.
2. Remove this self-reference trap. A committed capture cannot bind to the
   commit that contains its own bytes. Introduce an explicit, deterministic
   evidence/source revision (normally the implementation parent) and make
   `--check` verify that revision is an ancestor and that no relevant input
   evidence changed between it and the current head. Capture/report-only
   commits must not make their own evidence stale.
3. Commit the regenerated captures and any minimal collector/tests correction
   on the same worker branch, add a regression proving `--check` passes at the
   final post-capture commit and fails after a relevant evidence change, rerun
   all literal SPEC gates plus the tamper negative control, and push without
   amend or force-push.

Do not guess a runway rate. `hours:null` / `UNKNOWN` remains the correct current
answer until a complete comparable experimental unit exists.

## Revision review — REVISE (worker head `d247638e`)

The worker's one exact-session recovery produced and the supervisor narrowly
published clean worker commit `d247638e`. Independent branch verification
passed 19/19 tests, schema validation, `git diff --check`, and final-head
`--check`: the self-reference defect itself is corrected.

Integration is not accepted yet:

1. The current program line never contained worker implementation `bb67c566`,
   so the revision cannot be cherry-picked alone. A trial integration of the
   implementation plus revision was immediately reverted and never pushed.
2. On the integrated tree, running the committed 19-test suite rewrites six
   tracked fixture `throughput-observations.json` files (`skipped_folders`
   changes from two stale entries to `[]`). A green test run must leave the
   repository clean; committed golden outputs are stale under the revised
   collector.
3. Regenerate and commit every affected fixture output on the worker branch,
   prove `node --test orchestration/throughput/*.test.mjs` leaves `git status
   --short` empty, and retain the final-head/source-revision positive and both
   negative controls. Then a future reviewer may integrate the complete
   `bb67c566` + revision series and regenerate only the two live program
   captures at the integrated source revision.

The worker exhausted its single recovery after creating `d247638e`; do not
restart it automatically. Preserve the pushed branch for a later fresh-lane
revision. Runway remains honestly UNKNOWN.
