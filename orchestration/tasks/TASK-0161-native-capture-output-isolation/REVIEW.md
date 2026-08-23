# REVIEW — TASK-0161 native scenario capture-output isolation

- verdict: ACCEPTED
- reviewed frozen worker head: `9f004d2a6802782e2b3a79b895be2b0822f8dbf1`
- implementation commit: `c8cedf1730455d0fc62badc3c707dcbc96123254`
- reviewed base: `610a240e1e4bdfacfd77bec49e36be945a1ced13`
- reviewer: Cursor successor architect/orchestrator
- reviewed_at: 2026-08-22T17:18:00-07:00
- integrated_at: `76368466`

Independent detached review in
`Z:\Code\.worktrees\verdigris\review-task0161-9f004d2a` completed. The
interrupted predecessor Gate-B is discarded; this full rerun is the acceptance
evidence.

- local/remote worker heads equal `9f004d2a`; review tree clean before and after
  the gate;
- literal SPEC command
  `native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review`
  exited 0 (`GATE-EXIT=0`);
- post-gate `git status --short`, `git diff --check`, and `git diff --name-only`
  empty (no cleanup);
- `610a240e..HEAD` touches only owned paths (`main.cpp`, `build.ps1`, task
  folder). Program `main.cpp`/`build.ps1` are unchanged since that base, so the
  worker patch applies cleanly;
- outside-repo, `..`-escape, and uncreatable-under-file client seams exit 1 and
  create no files; build-layer `-CaptureRoot` outside the repo throws before
  build (`SCRIPT-REJECT-EXIT=1`);
- default `VERDIGRIS_CAPTURE_ROOT`-unset `--scenario move-and-camera` exits 0.

Integrate the frozen worker files. This is capture-output isolation only: no
renderer, gameplay, or historical-capture rewrite.
