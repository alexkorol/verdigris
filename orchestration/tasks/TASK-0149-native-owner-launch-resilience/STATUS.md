---
task: TASK-0149
title: Native owner-launch resilience
state: REVIEW_REQUESTED
coordinator: ox-alpha
worker: ox-pc-j
started-at: 2026-08-22T02:53:00-07:00
review-requested-at: 2026-08-22T03:45:00-07:00
---

# Review request

Implementation complete at this branch head; all SPEC acceptance gates run
with literal transcripts in `REPORT.md`:

- `native/build.ps1 -RunTests`: denylist/core/networking/camera2d/session all
  PASS, exit 0 (one transient load flake in timing-sensitive `reconnect:`
  checks documented; did not reproduce; sources untouched by this task).
- `play-native.ps1 -LifecycleSelfTest`: normal-close (real VerdigrisNativeClient
  window, WM_CLOSE, client self-exit code 0) and forced-exit (client killed)
  both leave no orphan server/client (PID-scoped assertion), exit 0.
- Documented owner command `play-native.ps1 -Rebuild` end-to-end with
  owner-style window close: rebuild PASS, chosen port/log self-reported,
  real windowed remote client, client exit code 0, launcher exit code 0,
  zero leftover verdigris processes machine-wide.
- Negative controls (`-Port 6500`, `-Port 7000`, `-LifecycleSelfTest` with
  `-Local`/`-Port`): fail fast pre-build, exit 1.
- `git diff --check`: pass (exit 0).

Changed file is exactly `native/tools/play-native.ps1` plus this task folder;
no forbidden path touched; port 6500 untouched; capsule 6520-6539 preserved.

# Claim

First committed STATUS write wins. No prior `STATUS.md` or `RELEASE.md`
existed in this folder at fetch time (folder contained only `SPEC.md`,
state READY).

## Identity

| Field | Value |
|---|---|
| Machine | `DESKTOP-TVU7OR7` |
| Worker lane | `ox-pc-j` |
| Lane port capsule | 6800-6819, loopback only |
| Preserved product capsule | 6520-6539 (play-native.ps1 server range; never 6500) |
| Provider | `openrouter` |
| Model | `stealth/ox-alpha` |
| Harness | OpenCode CLI 1.18.21 (version per RUN_STATUS endpoint identity watch; binary not on this shell PATH) |
| Branch | `codex/TASK-0149-native-owner-launch-resilience-ox-pc-j` |
| Routed head | `30e98e024d4a22a744be4bee63dfcf607f63010a` (exact HEAD verified) |
| Immutable SPEC base | `060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2` (verified ancestor of HEAD) |
| Worktree / clone path | `Z:\Code\.worktrees\verdigris\ox-pc-j` |
| Start time | 2026-08-22 02:53 PDT (-07:00) |

## Preflight evidence

- `git status --short`: clean.
- `git status -sb`: `## codex/TASK-0149-native-owner-launch-resilience-ox-pc-j` (no upstream yet; will push `-u origin`).
- `git fetch --prune origin`: done.
- `git rev-parse HEAD` == routed head exactly.
- `git merge-base --is-ancestor 060c1151... HEAD`: true.
- Task folder pre-claim contents: only `SPEC.md` (no competing claim, no RELEASE).

## Plan

Implement exactly the SPEC on `native/tools/play-native.ps1` plus this task
folder only. Fail-fast one-command Windows owner launch, self-explaining
port/log choice, deterministic lifecycle checks proving cleanup after normal
close and forced client exit, preserving default remote mode and the
6520-6539 capsule. All literal acceptance gates run before REVIEW_REQUESTED.
