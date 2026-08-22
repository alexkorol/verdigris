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
